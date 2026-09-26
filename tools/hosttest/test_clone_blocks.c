// Host-side tests for the ISO15693 clone loop's decision logic.
//
// Same technique as test_wipe_sweep.c: the shipped poller is compiled verbatim and the SDK calls resolve
// to the fake tag. See README.md for how the fake's semantics were verified against firmware source.
//
// The interesting cases here are the classification ones -- whether a block that would not write counts
// as lost data, as space past the card's capacity, or as neither. clone_capacity_confirmed is the
// strongest factual claim this feature makes about the user's hardware ("Card too small"), so the tests
// concentrate on what is allowed to set it.

#include "fake_tag.h"

#include "../../magic/protocols/iso15693/iso15693_poller.c" // NOLINT -- deliberate, see above

#include <stdio.h>

static int tests_run;
static int tests_failed;
static const char* current_test;
static bool current_failed;

#define CHECK_EQ(actual, expected)                                                                 \
    do {                                                                                           \
        long long a_ = (long long)(actual), e_ = (long long)(expected);                            \
        if(a_ != e_) {                                                                             \
            printf(                                                                                \
                "  FAIL %s:%d  %s == %lld, expected %lld\n", __FILE__, __LINE__, #actual, a_, e_); \
            current_failed = true;                                                                 \
        }                                                                                          \
    } while(0)

#define CHECK(cond)                                                  \
    do {                                                             \
        if(!(cond)) {                                                \
            printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            current_failed = true;                                   \
        }                                                            \
    } while(0)

static void begin(const char* name) {
    current_test = name;
    current_failed = false;
    tests_run++;
}

static void end(void) {
    if(current_failed) {
        tests_failed++;
        printf("FAILED: %s\n", current_test);
        printf("  --- captured log ---\n%s  --------------------\n", fake_log_text());
    } else {
        printf("  ok  %s\n", current_test);
    }
}

static Iso15693_3Data source;

static bool bitmap_bit(const Iso15693Poller* inst, uint16_t block) {
    return (inst->clone_failed_bitmap[block / 8] & (1u << (block % 8))) != 0;
}

static uint16_t bitmap_count(const Iso15693Poller* inst) {
    uint16_t n = 0;
    for(uint16_t b = 0; b < ISO15693_POLLER_BLOCK_BITMAP_SIZE * 8; b++) {
        if(bitmap_bit(inst, b)) n++;
    }
    return n;
}

// The poller allocates these once for its whole life and addresses every write to the card it found
// at activation. These drivers call the pass functions directly, below write_step, so they stand in
// for both: one buffer pair for the run, and the address Iso15693WriteStateStart would have taken.
static BitBuffer* driver_tx;
static BitBuffer* driver_rx;

static void driver_init(Iso15693Poller* inst) {
    if(driver_tx == NULL) {
        driver_tx = bit_buffer_alloc(ISO15693_POLLER_BUF_SIZE);
        driver_rx = bit_buffer_alloc(ISO15693_POLLER_BUF_SIZE);
    }
    memset(inst, 0, sizeof(*inst));
    inst->frame_tx = driver_tx;
    inst->frame_rx = driver_rx;
    memcpy(inst->address_uid, fake_tag.uid, ISO15693_3_UID_SIZE);
}

static bool run_clone(Iso15693Poller* inst, bool skip_backdoor) {
    driver_init(inst);
    inst->clone_source = &source;
    return iso15693_poller_write_source_blocks(inst, NULL, skip_backdoor);
}

// ---- the cases ------------------------------------------------------------------------------------

// A 28-block source onto a 64-block card. Everything fits and everything writes.
static void test_clean_clone_fits(void) {
    begin("28-block source onto a 64-block card writes cleanly");
    fake_tag_init(64, 64, 4);
    fake_data_init(&source, 28, 4);
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present);
    CHECK_EQ(inst.clone_blocks_total, 28);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK_EQ(inst.clone_over_capacity, 0);
    CHECK(!inst.clone_capacity_confirmed);
    CHECK_EQ(bitmap_count(&inst), 0);
    end();
}

// The hardware-verified oversize case: a 70-block source with DATA in its tail onto 64-block silicon.
// Six blocks of real data are lost, so it is a genuine failure -- and the failures are a persistent,
// contiguous, read-refusing run at the top, which is what earns "Card too small".
// Bench 2026-08-11: "70/64 clone -> Partial, 6 blocks named, Card too small".
static void test_oversize_source_with_data_tail(void) {
    begin("70/64 with a non-empty tail is Partial and earns the capacity claim");
    fake_tag_init(64, 64, 4);
    fake_data_init(&source, 70, 4); // every block non-zero
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present);
    CHECK_EQ(inst.clone_blocks_total, 70);
    CHECK_EQ(inst.clone_failed_count, 6); // 64..69 held data and could not be written
    CHECK_EQ(inst.clone_over_capacity, 0);
    CHECK(inst.clone_capacity_confirmed);
    CHECK_EQ(bitmap_count(&inst), 6);
    for(uint16_t b = 64; b <= 69; b++) {
        CHECK(bitmap_bit(&inst, b));
    }
    end();
}

// Same geometry, but the source's tail is EMPTY. Nothing was lost, so the clone is a Success carrying a
// note rather than a Partial -- the over_capacity bucket.
static void test_oversize_source_with_empty_tail(void) {
    begin("70/64 with an empty tail loses nothing and reports over-capacity");
    fake_tag_init(64, 64, 4);
    fake_data_init(&source, 70, 4);
    fake_data_fill(&source, 64, 69, 0x00); // the source held nothing up there
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present);
    CHECK_EQ(inst.clone_blocks_total, 70);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK_EQ(inst.clone_over_capacity, 6);
    CHECK(inst.clone_capacity_confirmed);
    end();
}

// THE discriminating case, and the reason the read-probe runs on every persistent failure. Same empty
// tail, but the target's blocks 64..69 ANSWER READS while refusing writes. A block that answers a read
// exists, so this is not the card's capacity edge whatever shape the failures make -- calling it one
// would fabricate a claim about the user's hardware. Shipped on reasoning alone until now.
static void test_failure_that_answers_a_read_is_not_capacity(void) {
    begin("a failed block that answers a read is not a capacity edge");
    fake_tag_init(64, 64, 4);
    fake_tag_set_range(64, 69, FakeBlockLocked); // refuse writes, still answer reads
    fake_data_init(&source, 70, 4);
    fake_data_fill(&source, 64, 69, 0x00);
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present);
    CHECK(!inst.clone_capacity_confirmed); // no "Card too small"
    CHECK_EQ(inst.clone_over_capacity, 0); // not excused as past capacity
    CHECK_EQ(inst.clone_failed_count, 6); // folded into plain failures instead
    CHECK_EQ(bitmap_count(&inst), 6);
    end();
}

// A failure with a successful write ABOVE it is not a tail at all, so it cannot be the capacity edge --
// tracked as "did anything write above a failure" rather than by comparing indices, because the gen1
// path skips blocks 56/57/62/63 and an index comparison reads that gap as a scattered failure.
static void test_interior_failure_is_not_capacity(void) {
    begin("an interior failure with writes above it is not a capacity edge");
    fake_tag_init(64, 64, 4);
    fake_tag_set_range(10, 10, FakeBlockLocked);
    fake_data_init(&source, 28, 4);
    fake_data_fill(&source, 10, 10, 0x00); // empty, so only position could excuse it
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present);
    CHECK(!inst.clone_capacity_confirmed);
    CHECK_EQ(inst.clone_over_capacity, 0);
    CHECK_EQ(inst.clone_failed_count, 1);
    CHECK(bitmap_bit(&inst, 10));
    end();
}

// The same rule, but on the silicon that actually exercises the guard. The test above uses a LOCKED
// block, which still ANSWERS a read -- so any_failure_answered decides it and wrote_above_failure never
// does any work. Dropping that conjunct therefore left the whole suite green while the clone reported a
// card with a mid-memory dropout as "Clone finished / Holds 27 of 28", a fabricated verdict about the
// user's hardware. An ABSENT block refuses the write and fails the read, which is how a real dropout
// presents and the only shape where position is the sole remaining excuse.
static void test_interior_absent_block_is_not_capacity(void) {
    begin("an interior block that answers NOTHING is still not a capacity edge");
    fake_tag_init(64, 64, 4);
    fake_tag_set_range(10, 10, FakeBlockAbsent);
    fake_data_init(&source, 28, 4);
    fake_data_fill(&source, 10, 10, 0x00); // empty, so only position could excuse it
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present);
    CHECK(!inst.clone_capacity_confirmed); // blocks 11..27 wrote ABOVE it, so it is no tail
    CHECK_EQ(inst.clone_over_capacity, 0);
    CHECK_EQ(inst.clone_failed_count, 1); // counted as a real failure, not excused
    CHECK(bitmap_bit(&inst, 10));
    end();
}

// A card lifted mid-clone makes every remaining block fail, which is EXACTLY the shape of the card's
// capacity ending there. It must report the removal rather than classify anything.
static void test_card_lifted_mid_clone(void) {
    begin("card lifted mid-clone reports removal, not capacity");
    fake_tag_init(64, 64, 4);
    fake_data_init(&source, 64, 4);
    fake_tag.ops_until_lifted = 10;
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(!present); // caller reports CardLost and discards the counters
    end();
}

// The gen1 path leaves blocks 56/57/62/63 alone -- they carry the UID/unlock/commit, not source data --
// and excludes them from the reported total, so "Cloned X/Y" counts only what gen1 can carry.
static void test_gen1_skips_the_backdoor_blocks(void) {
    begin("gen1 excludes the four backdoor blocks from the total");
    fake_tag_init(64, 64, 4);
    fake_data_init(&source, 64, 4);
    // The source must NOT share the tag's fill byte. With both at FAKE_MARKER, "block 56 is non-zero"
    // was true whether the block had been skipped or overwritten with source data, so this test passed
    // against a mutant whose is_backdoor_block always returned false -- i.e. against a gen1 clone
    // writing straight over the backdoor registers, which is the one thing it exists to catch.
    fake_data_fill(&source, 0, 63, 0x5A);
    Iso15693Poller inst;
    const bool present = run_clone(&inst, true);

    CHECK(present);
    CHECK_EQ(inst.clone_blocks_total, 60); // 64 less 56, 57, 62, 63
    CHECK(inst.clone_gen1_blocks_skipped); // and the result screens may say so
    CHECK_EQ(inst.clone_failed_count, 0);
    // The four were never written, so the target still holds its OWN byte, not the source's.
    CHECK_EQ(fake_tag.content[56][0], FAKE_MARKER);
    CHECK_EQ(fake_tag.content[57][0], FAKE_MARKER);
    CHECK_EQ(fake_tag.content[62][0], FAKE_MARKER);
    CHECK_EQ(fake_tag.content[63][0], FAKE_MARKER);
    // ...and a block either side DID take the source's byte, so the skip is a skip and not a clone
    // that wrote nothing at all.
    CHECK_EQ(fake_tag.content[55][0], 0x5A);
    CHECK_EQ(fake_tag.content[58][0], 0x5A);
    end();
}

// A source SMALLER than the gen1 backdoor addresses. Blocks 56/57/62/63 are not in it at all, so nothing
// may be deducted from the total -- deducting unconditionally would report "Cloned 28/28" as "28/24" and
// understate a clone that lost nothing. Real case: magic SLIX cards ship with 32 blocks, where those four
// addresses are outside the data space entirely and a gen1 clone therefore loses no fidelity at all.
static void test_gen1_small_source_deducts_nothing(void) {
    begin("gen1 on a source below block 56 deducts nothing from the total");
    fake_tag_init(32, 32, 4);
    fake_data_init(&source, 32, 4);
    Iso15693Poller inst;
    const bool present = run_clone(&inst, true);

    CHECK(present);
    CHECK_EQ(inst.clone_blocks_total, 32); // all 32, no phantom deduction
    // ...and nothing was skipped, so no screen may tell the user those blocks are missing from the
    // card. The deduction and the caveat come off the same count for exactly this reason.
    CHECK(!inst.clone_gen1_blocks_skipped);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK_EQ(inst.clone_over_capacity, 0);
    end();
}

// The boundary: a source that reaches 56 and 57 but not 62 and 63. Each backdoor block is deducted only
// if the source actually contains it, so this must lose exactly two.
static void test_gen1_partial_backdoor_overlap(void) {
    begin("gen1 deducts only the backdoor blocks the source actually has");
    fake_tag_init(58, 58, 4);
    fake_data_init(&source, 58, 4);
    fake_data_fill(&source, 0, 57, 0x5A); // distinct from the tag's own byte -- see the test above
    Iso15693Poller inst;
    const bool present = run_clone(&inst, true);

    CHECK(present);
    CHECK_EQ(inst.clone_blocks_total, 56); // 58 less blocks 56 and 57; 62 and 63 are out of range
    CHECK_EQ(inst.clone_failed_count, 0);
    // The two in range were skipped, so the target keeps its own byte rather than the source's.
    CHECK_EQ(fake_tag.content[56][0], FAKE_MARKER);
    CHECK_EQ(fake_tag.content[57][0], FAKE_MARKER);
    CHECK_EQ(fake_tag.content[55][0], 0x5A); // and the block below them did take
    end();
}

// An empty source has nothing to clone. The caller refuses before this point, but the loop must not
// misbehave if reached.
static void test_empty_source(void) {
    begin("an empty source writes nothing");
    fake_tag_init(64, 64, 4);
    fake_data_init(&source, 0, 4);
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present);
    CHECK_EQ(inst.clone_blocks_total, 0);
    CHECK_EQ(inst.clone_failed_count, 0);
    end();
}

// The source's block size comes straight out of a loaded .nfc, so it is unbounded by anything the app
// controls, and it is passed as the read length into a 32-byte stack buffer. It must be clamped.
static void test_absurd_source_block_size_is_clamped(void) {
    begin("an absurd source block size is clamped, not trusted");
    fake_tag_init(64, 64, 32);
    fake_data_init(&source, 8, 32);
    source.system_info.block_size = 200; // hand-edited .nfc
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present); // did not run off the end of the probe buffer
    CHECK_EQ(inst.clone_blocks_total, 8);
    end();
}

// The source count is clamped to the block-number space, so a corrupt .nfc claiming more than 256 blocks
// cannot make clone_blocks_total overstate what was attempted.
static void test_source_count_clamped_to_bitmap(void) {
    begin("a source claiming more than 256 blocks is clamped");
    fake_tag_init(256, 256, 4);
    fake_data_init(&source, 256, 4);
    source.system_info.block_count = 400; // beyond the block-number space
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present);
    CHECK_EQ(inst.clone_blocks_total, 256);
    end();
}

// The clock backstop, with the card STILL PRESENT. Blocks the clock never reached were not attempted, so
// they are recorded as failures rather than counted as written -- otherwise a clone stopped at block 10
// of 256 would claim all 256 landed.
//
// This case is on the reasoned-only list: no card writes slowly enough to spend 10s over 256 blocks.
static void test_clock_cut_clone_with_card_present(void) {
    begin("a clone cut by the clock records the blocks it never reached");
    fake_tag_init(256, 256, 4);
    fake_data_init(&source, 256, 4);
    fake_tag.tick_cost_per_op = 50; // ~200 accepted blocks inside the 10s budget
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present); // the card never left
    CHECK_EQ(inst.clone_blocks_total, 256);
    CHECK(inst.clone_failed_count > 0); // the unreached blocks are accounted for
    CHECK(strstr(fake_log_text(), "time limit reached") != NULL);

    // Every block is either written or recorded as failed -- nothing is silently counted as written.
    CHECK_EQ(inst.clone_failed_count, bitmap_count(&inst));

    // The card is fine; it was the clock that stopped us. Claiming the card is too small would be a
    // fabricated statement about the user's hardware -- the exact failure the read-probe exists to
    // prevent, arrived at from a different direction.
    CHECK(!inst.clone_capacity_confirmed);

    // ...and that fact has to LEAVE the function. Held in a local, the truncation stopped the poller
    // fabricating "Card too small" but let every screen downstream fabricate the same thing per block:
    // "Not written: 246", 246 indices listed under "Blocks not written", and Finish rather than Retry.
    // The card refused none of them. These two fields are what the report reads to tell a block the
    // card refused from one nothing was sent to.
    CHECK(inst.pass_truncated);
    CHECK(inst.pass_cut_block > 0);
    CHECK(inst.pass_cut_block < 256); // it really was cut, not run to the end

    // The division the screens depend on: below the cut, blocks that were attempted; at and above it,
    // blocks that were not. Everything the back-fill recorded sits above the cut.
    uint16_t refused_below_cut = 0;
    for(uint16_t b = 0; b < inst.pass_cut_block; b++) {
        if(inst.clone_failed_bitmap[b / 8] & (1u << (b % 8))) refused_below_cut++;
    }
    CHECK_EQ(refused_below_cut, 0); // this card accepted every block it was actually asked for
    CHECK_EQ(inst.clone_failed_count, (uint16_t)(256 - inst.pass_cut_block));
    end();
}

// The same bound on a run that is NOT cut must leave both fields alone -- otherwise every ordinary
// partial would offer Retry and hide its block list behind a truncation note.
static void test_uncut_clone_sets_no_truncation(void) {
    begin("a clone that finishes leaves the truncation fields clear");
    fake_tag_init(64, 64, 4);
    fake_data_init(&source, 64, 4);
    fake_tag_set_range(
        30, 30, FakeBlockLocked); // an ordinary refusal, nothing to do with the clock
    Iso15693Poller inst;
    const bool present = run_clone(&inst, false);

    CHECK(present);
    CHECK(!inst.pass_truncated);
    CHECK_EQ(inst.pass_cut_block, 0);
    CHECK_EQ(inst.clone_failed_count, 1); // block 30, genuinely refused
    end();
}

// ---- the survey above the source -----------------------------------------------------------------

// A clone writes the source's blocks and leaves the rest alone. On a card bigger than the source that
// means the previous card's data stays readable above it -- and on a gen2 clone the CFG frame then
// reports the source's smaller count over the top, so an ordinary dump shows a clean copy. Measured on
// a 64-block card cloned from a 28-block source: it reported 28 and blocks 28..63 still read back.
//
// The count cannot be the test, before or after, because on a magic card a count is a claim. So the
// survey READS upward, and these pin what it concludes.
static void test_the_survey_finds_data_left_above_the_source(void) {
    begin("readable blocks above the source that still hold data are reported");
    fake_tag_init(64, 64, 4); // honest about its size, and every block filled
    fake_data_init(&source, 8, 4);
    Iso15693Poller inst;
    run_clone(&inst, false);

    CHECK(inst.clone_residue_found);
    CHECK_EQ(inst.clone_residue_first, 8); // the first block the clone did not write
    CHECK_EQ(inst.clone_residue_last, 63); // ...through the card's real top
    // ...but this card's count is truthful, so there is nothing to say about its SIZE. The two
    // findings are independent and this is the case that proves it.
    CHECK(!inst.clone_holds_more);
    end();
}

// The gen2 shape, and the one the warning exists for: the CFG frame reports the source's smaller count
// over a card that still physically holds the rest, so an ordinary dump shows a clean copy and the
// data above it is invisible. Measured on a 64-block card reporting 28 after the clone.
static void test_a_card_reporting_less_than_it_holds_is_reported(void) {
    begin("a card that answers above the count it reports is reported as larger than it claims");
    fake_tag_init(28, 64, 4); // claims 28, holds 64, all filled
    fake_data_init(&source, 8, 4);
    Iso15693Poller inst;
    run_clone(&inst, false);

    CHECK(inst.clone_holds_more);
    CHECK_EQ(inst.clone_survey_top, 63);
    CHECK(inst.clone_residue_found); // and in this case the space has data in it too
    end();
}

// Existing is not residue. A clone onto a WIPED larger card leaves a clean tail, and warning about
// data there would fire on every such run. The SIZE finding still stands, though -- the card is bigger
// than it claims whether or not anything is left in it, which is the phantom tail the wipe sweeps for.
static void test_an_empty_tail_is_size_without_residue(void) {
    begin("an empty tail above the source is reported as size, not as data");
    fake_tag_init(28, 64, 4); // claims 28, holds 64
    fake_tag_fill(0, 63, 0x00); // ...and was wiped first, so the tail is clean
    fake_data_init(&source, 8, 4);
    Iso15693Poller inst;
    run_clone(&inst, false);

    CHECK(!inst.clone_residue_found);
    CHECK(inst.clone_holds_more);
    CHECK_EQ(inst.clone_survey_top, 63);
    end();
}

// And a card that really is the source's size says nothing at all. Without this the other two are
// consistent with a survey that reports on every clone.
static void test_a_card_the_size_of_its_source_is_quiet(void) {
    begin("a card no bigger than its source reports nothing");
    fake_tag_init(8, 8, 4);
    fake_data_init(&source, 8, 4);
    Iso15693Poller inst;
    run_clone(&inst, false);

    CHECK(!inst.clone_residue_found);
    CHECK(!inst.clone_holds_more);
    CHECK(!inst.clone_memory_differs);
    end();
}

// "Larger than it claims" needs a claim. If GET SYSTEM INFO does not answer, the card's count is
// unknown rather than zero -- and comparing against zero would make every readable block above the
// source look like an over-claim, so one dropped frame would print "reports 0 blocks" about the
// user's card. The survey still runs, and still finds the residue, because a READ answering is a fact
// about the card either way; only the size finding depends on the claim.
// The other way to have no claim, and the one that needs no failure to reach: the card answers GET
// SYSTEM INFO and simply does not state a block count. Kept separate from the timeout case because a
// guard written only against a failed frame would pass that test and still read an unstated count as
// zero.
static void test_a_card_that_states_no_count_is_not_an_over_claim(void) {
    begin("a card that states no block count is not reported as larger than it claims");
    fake_tag_init(28, 64, 4);
    fake_data_init(&source, 8, 4);
    fake_tag.hides_memory = true; // answers, but claims no size
    Iso15693Poller inst;
    run_clone(&inst, false);

    CHECK(!inst.clone_holds_more);
    CHECK(!inst.clone_card_blocks_known);
    CHECK(inst.clone_residue_found);
    end();
}

static void test_an_unknown_card_count_is_not_an_over_claim(void) {
    begin("a card whose count never answered is not reported as larger than it claims");
    fake_tag_init(28, 64, 4); // claims 28, holds 64, all filled -- the over-claim shape
    fake_data_init(&source, 8, 4);
    fake_tag.sysinfo_fails = true; // ...but the claim never arrives
    Iso15693Poller inst;
    run_clone(&inst, false);

    CHECK(!inst.clone_holds_more); // no claim, so nothing to be larger than
    CHECK(!inst.clone_card_blocks_known);
    CHECK(inst.clone_residue_found); // the reads still answered, so this still stands
    CHECK_EQ(inst.clone_residue_last, 63);
    end();
}

// The geometry finding is a comparison of CLAIMS, which is the right subject: it answers "will the copy
// present as the source", not "what is the silicon". gen2 programs its answer through the CFG register
// so the two agree there; gen1 has no such register and the card goes on announcing its own size.
static void test_a_card_that_keeps_reporting_its_own_size_says_so(void) {
    begin("a card still reporting a geometry the source did not is reported");
    fake_tag_init(40, 40, 4); // the card claims 40...
    fake_data_init(&source, 28, 4); // ...cloned from a source claiming 28
    Iso15693Poller inst;
    run_clone(&inst, false);

    CHECK(inst.clone_memory_differs);
    CHECK_EQ(inst.clone_card_blocks, 40);
    CHECK(inst.clone_memory_differs);
    end();
}

// The survey stops a run past the card's top rather than walking to the block-number ceiling. No
// result changes either way -- absent blocks contribute nothing -- so only the cost tells them apart,
// and the cost is what matters: on a small card the difference is tens of reads against hundreds, each
// paying a radio timeout, against a pass budget that also bounds a clone.
//
// Counted as a DIFFERENCE between two cards whose tops differ, so it pins the survey tracking the
// card rather than any particular absolute.
static void test_the_survey_stops_at_the_card_rather_than_the_ceiling(void) {
    begin("the survey stops just past the card's top, not at the block-number ceiling");
    fake_tag_init(28, 64, 4);
    fake_data_init(&source, 8, 4);
    Iso15693Poller inst;
    run_clone(&inst, false);
    const uint32_t reads_64 = fake_tag.reads_attempted;

    fake_tag_init(28, 32, 4); // same claim, half the card
    fake_data_init(&source, 8, 4);
    run_clone(&inst, false);
    const uint32_t reads_32 = fake_tag.reads_attempted;

    CHECK_EQ(reads_64 - reads_32, 32); // exactly the 32 blocks the bigger card has and this one lacks
    end();
}

// ---- a gen1 card reached down the gen2 path --------------------------------------------------

// The gen2 verify passes whenever the card already WEARS the target UID, because then it proves only
// that the UID matches -- not that anything magic happened. On gen1 silicon that sends the data pass
// straight into 56/57, which are the UID registers, and the card's identity becomes bytes lifted out
// of the file. Measured: a 64-block source onto a gen1 SLIX already carrying its UID left the card
// answering to 39 A5 39 5A 38 A5 38 5A -- the file's blocks 56 and 57, fed through the UID mapping.
//
// The write that moves the UID is also what proves what the card is, so the run converts itself into
// a gen1 clone from that point: it stops feeding the file into registers, and afterwards puts the
// target UID back.
static void test_a_gen1_card_on_the_gen2_path_converts_and_repairs(void) {
    begin("a clone that finds 56/57 are registers finishes as a gen1 clone and restores the UID");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = true;
    fake_data_init(&source, 64, 4);
    fake_data_fill(&source, 0, 63, 0x5A);

    static const uint8_t target[ISO15693_3_UID_SIZE] =
        {0xE0, 0x04, 0x01, 0x10, 0xB1, 0xB2, 0xB3, 0xB4};

    Iso15693Poller inst;
    driver_init(&inst);
    inst.clone_source = &source;
    memcpy(inst.target_uid, target, ISO15693_3_UID_SIZE);
    // skip_backdoor false: the gen2 arm's choice, made before the card could contradict it.
    iso15693_poller_write_source_blocks(&inst, NULL, false);

    CHECK(inst.uid_moved_by_write); // the card said what it is
    CHECK(inst.clone_used_gen1); // ...and the run is reported as the gen1 clone it became
    CHECK(inst.clone_gen1_blocks_skipped);
    CHECK_EQ(inst.clone_blocks_total, 60); // 64 less the four registers
    CHECK_EQ(inst.clone_failed_count, 0);

    // The fixture holds a gen1 UID write until the next power-cycle, which is stricter than the
    // hardware on purpose (see gen1_uid_pending). The repair's effect is visible on the far side.
    fake_tag_power_cycle();
    CHECK(memcmp(fake_tag.uid, target, ISO15693_3_UID_SIZE) == 0);
    end();
}

// The same card WITHOUT a matching UID takes the gen1 path in the first place, skips the registers up
// front, and never converts. Without this, the conversion is consistent with firing on every gen1 run.
static void test_the_ordinary_gen1_path_does_not_convert(void) {
    begin("a clone that skipped the registers from the start never converts or repairs");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = true;
    fake_data_init(&source, 64, 4);

    Iso15693Poller inst;
    driver_init(&inst);
    inst.clone_source = &source;
    iso15693_poller_write_source_blocks(&inst, NULL, true); // the gen1 arm's choice

    CHECK(!inst.uid_moved_by_write);
    end();
}

// And a gen2 card is untouched by any of it: 56/57 are ordinary memory there, the UID does not move,
// and the file's data at those addresses is written like any other block. This is the case the
// alternative fix -- skipping the registers whenever the UID already matched -- would have broken,
// silently dropping four blocks of the file while reporting a clean success.
static void test_a_gen2_card_writes_those_blocks_like_any_other(void) {
    begin("on a gen2 card 56/57 are data, and the file's blocks there are written");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true; // NOT gen1: those addresses are memory
    fake_data_init(&source, 64, 4);
    fake_data_fill(&source, 0, 63, 0x5A);

    Iso15693Poller inst;
    driver_init(&inst);
    inst.clone_source = &source;
    iso15693_poller_write_source_blocks(&inst, NULL, false);

    CHECK(!inst.uid_moved_by_write);
    CHECK_EQ(inst.clone_blocks_total, 64); // nothing deducted
    CHECK_EQ(fake_tag.content[56][0], 0x5A); // and the file's data really is in them
    CHECK_EQ(fake_tag.content[57][0], 0x5A);
    end();
}

// A write taken by a UID register says nothing about how much MEMORY the card has, so it must not
// suppress the capacity finding. On a card smaller than its source the failures run to the top and
// then block 56 answers -- because it is a register -- and counting that as "a block wrote above the
// failures" costs the report the one thing it could still say: the card is too small.
//
// The two runs must agree. A gen1 clone and a clone that CONVERTED to one are the same result, and a
// reader should not be able to tell which path produced it.
static void test_a_register_write_is_not_evidence_about_capacity(void) {
    begin("a converted clone reaches the same capacity verdict as the gen1 path");
    static const uint8_t target[ISO15693_3_UID_SIZE] =
        {0xE0, 0x04, 0x01, 0x10, 0xB1, 0xB2, 0xB3, 0xB4};

    // The gen1 path: registers skipped from the start, failures run to the card's top.
    fake_tag_init(28, 28, 4);
    fake_tag.is_gen1_magic = true;
    fake_tag_set_range(56, 57, FakeBlockWritable); // the registers answer, as gen1 silicon does
    fake_data_init(&source, 64, 4);
    Iso15693Poller gen1;
    driver_init(&gen1);
    gen1.clone_source = &source;
    memcpy(gen1.target_uid, target, ISO15693_3_UID_SIZE);
    iso15693_poller_write_source_blocks(&gen1, NULL, true);

    // The same card reached down the gen2 path, which converts at block 56.
    fake_tag_init(28, 28, 4);
    fake_tag.is_gen1_magic = true;
    fake_tag_set_range(56, 57, FakeBlockWritable);
    fake_data_init(&source, 64, 4);
    Iso15693Poller converted;
    driver_init(&converted);
    converted.clone_source = &source;
    memcpy(converted.target_uid, target, ISO15693_3_UID_SIZE);
    iso15693_poller_write_source_blocks(&converted, NULL, false);

    CHECK(converted.uid_moved_by_write); // it really did take the other route
    CHECK_EQ(converted.clone_capacity_confirmed, gen1.clone_capacity_confirmed);
    CHECK(gen1.clone_capacity_confirmed); // ...and the verdict they agree on is the right one
    CHECK_EQ(converted.clone_blocks_total, gen1.clone_blocks_total);
    end();
}

// The two halves of the geometry comparison move independently, and the screen names only the one
// that moved. A 28-block file onto a 28-block card whose IC reference differs is a real mismatch --
// but saying "the card reports 28 blocks, not the file's" about it describes a number that matches.
static void test_geometry_names_the_half_that_actually_differs(void) {
    begin("a size that agrees is not reported as a mismatch");
    // Same size, different chip: the IC reference is the only thing wrong.
    fake_tag_init(28, 28, 4);
    fake_tag.advertises_ic_ref = true;
    fake_tag.ic_ref = 0x01;
    fake_data_init(&source, 28, 4);
    source.system_info.flags |= ISO15693_3_SYSINFO_FLAG_IC_REF;
    source.system_info.ic_ref = 0x03;
    Iso15693Poller inst;
    run_clone(&inst, false);

    CHECK(inst.clone_ic_ref_differs);
    CHECK(!inst.clone_memory_differs); // 28 == 28, so the screen must not name the size

    // And a size that really does differ is reported as one.
    fake_tag_init(40, 40, 4);
    fake_tag.advertises_ic_ref = true;
    fake_tag.ic_ref = 0x03;
    fake_data_init(&source, 28, 4);
    source.system_info.flags |= ISO15693_3_SYSINFO_FLAG_IC_REF;
    source.system_info.ic_ref = 0x03; // identical chip, so only the size is left
    run_clone(&inst, false);

    CHECK(inst.clone_memory_differs);
    CHECK(!inst.clone_ic_ref_differs); // identical chip, so only the size is named
    end();
}

int main(void) {
    printf("iso15693 clone loop\n");
    test_clean_clone_fits();
    test_oversize_source_with_data_tail();
    test_oversize_source_with_empty_tail();
    test_failure_that_answers_a_read_is_not_capacity();
    test_interior_failure_is_not_capacity();
    test_interior_absent_block_is_not_capacity();
    test_card_lifted_mid_clone();
    test_gen1_skips_the_backdoor_blocks();
    test_gen1_small_source_deducts_nothing();
    test_the_survey_finds_data_left_above_the_source();
    test_a_card_reporting_less_than_it_holds_is_reported();
    test_an_empty_tail_is_size_without_residue();
    test_a_card_the_size_of_its_source_is_quiet();
    test_an_unknown_card_count_is_not_an_over_claim();
    test_a_card_that_states_no_count_is_not_an_over_claim();
    test_a_card_that_keeps_reporting_its_own_size_says_so();
    test_the_survey_stops_at_the_card_rather_than_the_ceiling();
    test_a_gen1_card_on_the_gen2_path_converts_and_repairs();
    test_the_ordinary_gen1_path_does_not_convert();
    test_a_gen2_card_writes_those_blocks_like_any_other();
    test_a_register_write_is_not_evidence_about_capacity();
    test_geometry_names_the_half_that_actually_differs();
    test_gen1_partial_backdoor_overlap();
    test_uncut_clone_sets_no_truncation();
    test_empty_source();
    test_absurd_source_block_size_is_clamped();
    test_source_count_clamped_to_bitmap();
    test_clock_cut_clone_with_card_present();

    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
