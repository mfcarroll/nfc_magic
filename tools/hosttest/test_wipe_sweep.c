// Host-side tests for the ISO15693 wipe sweep's decision logic.
//
// The shipped poller is compiled VERBATIM -- this file includes the .c so it can reach the file-statics,
// and the SDK calls resolve to the fake tag. No production code is modified or wrapped, so a behaviour
// change in the sweep is a test failure here.
//
// The fake tag's semantics are not guesses: each one is checked against the firmware's own
// iso15693_3 implementation, cited in README.md. What the host cannot reach is the radio layer below
// that -- what real silicon puts on the wire -- which is a hardware question, not a source question.

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

// Run the sweep against whatever the fake tag currently is.
static uint16_t run_sweep(Iso15693Poller* inst, bool* card_lost) {
    memset(inst, 0, sizeof(*inst));
    *card_lost = false;
    return iso15693_poller_wipe_blocks(inst, NULL, card_lost);
}

// ---- the cases ------------------------------------------------------------------------------------

// A clean card. Everything clears, the report matches the claim, nothing is flagged.
static void test_clean_64(void) {
    begin("clean 64/64 clears everything");
    fake_tag_init(64, 64, 4);
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 64);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK_EQ(inst.clone_blocks_total, 64);
    CHECK_EQ(inst.wipe_advertised, 64);
    CHECK(!inst.pass_truncated);
    CHECK(!card_lost);
    CHECK_EQ(bitmap_count(&inst), 0);
    end();
}

// The card the sweep exists for: cloned from a 28-block source onto 64-block silicon, so it advertises
// 28 while holding 64. Bounded by the claim it would clear 28 and call it Success, leaving 36 blocks of
// the previous card's data. Measured on hardware 2026-08-04.
static void test_advertises_28_holds_64(void) {
    begin("advertised 28 / physical 64 sweeps past the claim");
    fake_tag_init(28, 64, 4);
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 64);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK_EQ(inst.clone_blocks_total, 64); // what it PROVED, not what it claimed
    CHECK_EQ(inst.wipe_advertised, 28);
    CHECK(!inst.pass_truncated);
    CHECK(!card_lost);
    end();
}

// A card claiming more than it holds: 66 advertised, 64 real. The two phantom blocks are above the
// card's real top and were never its to clear, so they must be DROPPED rather than reported as
// "not cleared: 2".
static void test_advertises_66_holds_64(void) {
    begin("advertised 66 / physical 64 drops the phantom tail");
    fake_tag_init(66, 64, 4);
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 64);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK_EQ(inst.clone_blocks_total, 64);
    CHECK_EQ(bitmap_count(&inst), 0);
    CHECK(!card_lost);
    end();
}

// His Round 4 blocking finding, positive case. 64-block card, blocks 20..63 stop answering DURING the
// sweep -- so the activation cache holds their real contents. Without the discriminator this reports a
// clean "Wipe complete / Cleared 20 blocks" over 44 blocks that still hold the previous card's data.
static void test_dead_stretch_inside_claim_is_counted(void) {
    begin("dead stretch inside the claim counts, not drops (Round 4 blocking)");
    fake_tag_init(64, 64, 4);
    fake_tag_cache_all_advertised(); // they answered when the card was presented
    fake_tag_set_range(20, 63, FakeBlockAbsent); // and died during the sweep
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 20);
    CHECK_EQ(inst.clone_failed_count, 44); // proven present, proven not cleared
    CHECK_EQ(inst.clone_blocks_total, 64);
    CHECK(!card_lost);
    for(uint16_t b = 20; b <= 63; b++) {
        CHECK(bitmap_bit(&inst, b));
    }
    end();
}

// The residual he named and we did not close: if the degradation PREDATES activation, the cache is zeros
// there too, so nothing proves those blocks existed and they drop with the phantoms. Different symptom
// (degraded before the wipe rather than during it). Asserted so the limitation is recorded as behaviour
// rather than prose, and so closing it later shows up here as a change.
static void test_dead_stretch_before_activation_is_dropped(void) {
    begin("dead stretch predating activation drops (known residual)");
    fake_tag_init(64, 64, 4);
    fake_tag_set_range(20, 63, FakeBlockAbsent);
    fake_tag_cache_from_activation(); // cache stops at block 20, rest zeroed
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 20);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK_EQ(inst.clone_blocks_total, 20);
    CHECK(!card_lost);
    end();
}

// A write-protected block that still answers reads and still holds data. The wipe's promise was not kept
// there, so it counts and is named -- and the block exists, so it does not shorten the card.
static void test_locked_block_holding_data(void) {
    begin("locked block still holding data is a real failure");
    fake_tag_init(64, 64, 4);
    fake_tag_set_range(30, 30, FakeBlockLocked);
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 63);
    CHECK_EQ(inst.clone_failed_count, 1);
    CHECK_EQ(inst.clone_blocks_total, 64);
    CHECK(bitmap_bit(&inst, 30));
    CHECK_EQ(bitmap_count(&inst), 1);
    end();
}

// A locked block that is ALREADY zero did not lose anything, so it is not a failure -- classified by
// content, not by the write's return value.
static void test_locked_but_already_empty_is_not_a_failure(void) {
    begin("locked but already-empty block is not a failure");
    fake_tag_init(64, 64, 4);
    fake_tag_fill(30, 30, 0x00);
    fake_tag_set_range(30, 30, FakeBlockLocked);
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 63);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK_EQ(inst.clone_blocks_total, 64);
    CHECK_EQ(bitmap_count(&inst), 0);
    end();
}

// A dropout that recovers: blocks 10..14 answer nothing, 15 upward is fine. A later block answering
// proves 10..14 were interior, so they are faults to report rather than the top of the card.
static void test_interior_dropout_that_recovers(void) {
    begin("interior dropout that recovers counts as faults");
    fake_tag_init(64, 64, 4);
    fake_tag_cache_all_advertised();
    fake_tag_set_range(10, 14, FakeBlockAbsent);
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 59);
    CHECK_EQ(inst.clone_failed_count, 5);
    CHECK_EQ(inst.clone_blocks_total, 64);
    for(uint16_t b = 10; b <= 14; b++) {
        CHECK(bitmap_bit(&inst, b));
    }
    end();
}

// No usable geometry: report nothing wiped rather than sweeping a card that says it has no blocks.
static void test_advertised_zero(void) {
    begin("advertised 0 reports nothing wiped");
    fake_tag_init(0, 0, 4);
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 0);
    CHECK_EQ(inst.clone_blocks_total, 0);
    CHECK(!card_lost);
    end();
}

// The other half of the geometry guard, and the one that can lie. A card that claims blocks but
// reports a block size of 0 also sweeps nothing -- and must not leave its CLAIM sitting in
// blocks_total, because this path returns 0 wiped, which reaches a terminal event (NothingWiped).
// blocks_total's contract is that terminal events only ever carry the measured figure and only
// WriteProgress sees the advertised one, so the live denominator has to be set BELOW the guard.
static void test_zero_block_size_reports_no_claim(void) {
    begin("a card claiming blocks with block size 0 reports 0, not its claim");
    fake_tag_init(64, 64, 0);
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 0);
    CHECK_EQ(inst.clone_blocks_total, 0); // NOT 64 -- the claim must not reach a terminal event
    CHECK_EQ(inst.wipe_advertised, 64); // the claim itself is still recorded, where it belongs
    CHECK(!card_lost);
    end();
}

// HOW FAR PAST ITS CLAIM THE SWEEP GETS, which decides whether a wipe can destroy an armed gen1
// card's UID. Block 56 is modelled as real gen1 silicon presents it: it takes a write (it is a
// backdoor register) and answers no read, so it looks absent until written to. 48 and 49 are the
// boundary -- measured, not derived; the first attempt at this arithmetic said 57.
static void test_sweep_stops_short_of_56_at_48(void) {
    begin("a card claiming 48 never reaches block 56");
    fake_tag_init(48, 48, 4);
    fake_tag_set_range(56, 57, FakeBlockWritable);
    fake_tag_fill(56, 57, FAKE_MARKER);
    Iso15693Poller inst;
    bool card_lost;
    run_sweep(&inst, &card_lost);

    CHECK_EQ(fake_tag.content[56][0], FAKE_MARKER);
    CHECK(!card_lost);
    end();
}

// One more block of claim and the run reaches 56 -- seven blocks above everything the card admits
// to holding. 57 goes with it rather than one claim later, because the write to 56 lands and resets
// the run.
static void test_sweep_reaches_56_and_57_at_49(void) {
    begin("a card claiming 49 reaches both 56 and 57, above its own claim");
    fake_tag_init(49, 49, 4);
    fake_tag_set_range(56, 57, FakeBlockWritable);
    fake_tag_fill(56, 57, FAKE_MARKER);
    Iso15693Poller inst;
    bool card_lost;
    run_sweep(&inst, &card_lost);

    CHECK_EQ(fake_tag.content[56][0], 0x00); // on an armed gen1 card this is the UID going
    CHECK_EQ(fake_tag.content[57][0], 0x00);
    CHECK(!card_lost);
    end();
}

// The block-number space is 256 wide and the bitmap holds exactly that many bits. A card that is
// writable all the way to the ceiling must stop there rather than run past the bitmap.
static void test_full_256_hits_the_ceiling(void) {
    begin("256 writable blocks stop at the ceiling");
    fake_tag_init(256, 256, 4);
    Iso15693Poller inst;
    bool card_lost;
    const uint16_t wiped = run_sweep(&inst, &card_lost);

    CHECK_EQ(wiped, 256);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK_EQ(inst.clone_blocks_total, 256);
    CHECK(!card_lost);
    end();
}

// The wall-clock backstop. A card that refuses every write while still answering reads never accumulates
// an absent run, so nothing but the clock stops it. Unreachable on real hardware; here it is one field.
static void test_clock_cuts_the_sweep(void) {
    begin("a card that refuses writes but answers reads is cut by the clock");
    fake_tag_init(256, 256, 4);
    fake_tag_set_range(0, 255, FakeBlockLocked);
    fake_tag.tick_cost_per_op = 40; // ~10s lands well before block 256
    Iso15693Poller inst;
    bool card_lost;
    run_sweep(&inst, &card_lost);

    CHECK(inst.pass_truncated);
    CHECK(!card_lost);
    CHECK(inst.clone_blocks_total < 256); // stopped short of the claim
    CHECK(strstr(fake_log_text(), "time limit reached") != NULL);
    end();
}

// The cut index is its own figure, and these two cases are why it has to be. Both were rendering a
// false claim about which blocks the sweep tried, from the same substitution: blocks_total standing in
// for the cut. blocks_total is the highest block that ANSWERED, so it tracks the cut only when the
// sweep happens to end on a proven-present block, which is exactly what a truncation prevents.

// Below the claim: a trailing run the tail-drop discards was attempted -- three writes and a read each
// -- but leaves blocks_total behind it. The screen credited the sweep with less than it tried.
static void test_cut_index_exceeds_total_after_a_tail_drop(void) {
    begin("a dropped trailing run leaves the cut above blocks_total");
    fake_tag_init(64, 64, 4);
    fake_tag_set_range(
        50, 63, FakeBlockAbsent); // answers neither, and the floor keeps sweeping it
    fake_tag_cache_from_activation(); // nothing proves 50+ existed, so the tail-drop discards them
    // 50 accepted blocks cost 1 op each; each absent one costs 3 writes + a read. 160 ticks/op puts
    // the 10s cut a few blocks into the dead stretch -- past 50, so the drop and the cut disagree.
    fake_tag.tick_cost_per_op = 160;
    Iso15693Poller inst;
    bool card_lost;
    run_sweep(&inst, &card_lost);

    CHECK(inst.pass_truncated);
    CHECK(!card_lost);
    // The whole point: these are different numbers, and the larger one is what "stopped at" means.
    // Blocks 50..53 were attempted -- three writes and a read each -- then dropped by the tail rule,
    // so the total stops at 50 while the sweep reached 54.
    CHECK_EQ(inst.pass_cut_block, 54);
    CHECK_EQ(inst.clone_blocks_total, 50);
    end();
}

// Above the claim: the card this bound was designed for. It answers a read at every address, so no
// absent run ever closes the sweep and it walks well past the advertised count before the clock fires.
// The cut then sits ABOVE the claim -- which is what made "Stopped at %u of %u" render "200 of 64",
// parsing as a fraction, and so as falling short of 64 when it had gone beyond it.
static void test_cut_index_can_exceed_the_advertised_count(void) {
    begin("a read-everywhere card is cut above the count it advertises");
    fake_tag_init(64, 256, 4);
    fake_tag_set_range(0, 255, FakeBlockLocked); // refuses every write, answers every read
    // 4 ops per refused block, so 25 ticks/op spends the 10s budget around block 100 -- comfortably
    // past the 64 it claims, which is the condition under test.
    fake_tag.tick_cost_per_op = 25;
    Iso15693Poller inst;
    bool card_lost;
    run_sweep(&inst, &card_lost);

    CHECK(inst.pass_truncated);
    CHECK(!card_lost);
    CHECK(inst.pass_cut_block > inst.wipe_advertised); // the "200 of 64" shape
    end();
}

// A card lifted mid-sweep looks exactly like memory ending there. It must report CardLost rather than
// pass off the blocks it never reached as absent.
static void test_card_lifted_mid_sweep(void) {
    begin("card lifted mid-sweep reports CardLost");
    fake_tag_init(64, 64, 4);
    fake_tag.ops_until_lifted = 12; // a handful of blocks in
    Iso15693Poller inst;
    bool card_lost;
    run_sweep(&inst, &card_lost);

    CHECK(card_lost);
    end();
}

// The summary line is a parseable assertion target, and it must be emitted on EVERY exit -- the timing
// figure exists so the sweep's cost is measurable rather than estimated.
//
// It also pins a documented cost: a clean 64-block card reports 72 blocks ATTEMPTED, because the sweep
// probes ISO15693_POLLER_WIPE_ABSENT_RUN blocks past the card's real top before concluding it ended.
// That tolerance is paid on every wipe and is the reason the run length is not raised further.
static void test_summary_line_is_emitted(void) {
    begin("the sweep logs its summary, and pays ABSENT_RUN probes past the top");
    fake_tag_init(64, 64, 4);
    Iso15693Poller inst;
    bool card_lost;
    run_sweep(&inst, &card_lost);

    CHECK_EQ(64 + ISO15693_POLLER_WIPE_ABSENT_RUN, 72);
    CHECK(strstr(fake_log_text(), "72 blocks attempted, 64 cleared") != NULL);
    CHECK(strstr(fake_log_text(), "advertised 64") != NULL);
    CHECK(strstr(fake_log_text(), "card ends at block 63") != NULL);
    end();
}

// iso15693_poller_cut_pass_if_expired is now the ONE place the clock cut is decided and recorded, for both
// the clone's write loop and this sweep. The two properties every caller leans on are pinned here
// rather than inferred from a whole run, because a full sweep can only show them together.
static void test_pass_cut_records_it_on_the_instance(void) {
    begin("the pass-expiry helper records the cut, and leaves the fields alone inside the budget");
    Iso15693Poller inst;
    memset(&inst, 0, sizeof(inst));
    const uint32_t start = 1000;
    const uint32_t budget = furi_ms_to_ticks(ISO15693_POLLER_PASS_MAX_MS);

    // Exactly at the budget is not yet over it, and an uncut run must not look truncated: these two
    // fields are what every screen reads to tell a refused block from one nothing was sent to.
    fake_tick = start + budget;
    CHECK(!iso15693_poller_cut_pass_if_expired(&inst, start, budget, 40));
    CHECK(!inst.pass_truncated);
    CHECK_EQ(inst.pass_cut_block, 0);

    // One tick past, and the block handed in is the one recorded -- the callers pass the block they
    // have NOT yet attempted, so this is the exclusive end of the attempted range.
    fake_tick = start + budget + 1;
    CHECK(iso15693_poller_cut_pass_if_expired(&inst, start, budget, 40));
    CHECK(inst.pass_truncated);
    CHECK_EQ(inst.pass_cut_block, 40);
    end();
}

// Why the helper compares elapsed-against-budget rather than holding an absolute deadline. An
// absolute deadline passes every other test in this suite and fails only here.
static void test_pass_cut_survives_a_tick_wraparound(void) {
    begin("the pass-expiry helper survives a tick wraparound");
    Iso15693Poller inst;
    memset(&inst, 0, sizeof(inst));
    const uint32_t start = UINT32_MAX - 100;
    const uint32_t budget = furi_ms_to_ticks(ISO15693_POLLER_PASS_MAX_MS);

    fake_tick = start + 50; // still below the ceiling
    CHECK(!iso15693_poller_cut_pass_if_expired(&inst, start, budget, 7));
    CHECK(!inst.pass_truncated);

    fake_tick = (uint32_t)(start + 200); // past the wrap
    CHECK(fake_tick < start); // the wrap really happened
    CHECK(!iso15693_poller_cut_pass_if_expired(&inst, start, budget, 7)); // 200 ticks, not 4 billion
    CHECK(!inst.pass_truncated);

    fake_tick = (uint32_t)(start + budget + 1); // genuinely over, wrap notwithstanding
    CHECK(iso15693_poller_cut_pass_if_expired(&inst, start, budget, 7));
    CHECK(inst.pass_truncated);
    CHECK_EQ(inst.pass_cut_block, 7);
    end();
}

// The clamp both the clone and the wipe run their block size through. Neither controls its input --
// one is a loaded .nfc, the other the card's own GET SYSTEM INFO -- and both feed a fixed buffer.
static void test_block_size_clamps_to_the_buffer(void) {
    begin("an over-large block size clamps to the fixed buffer, and valid sizes pass through");
    CHECK_EQ(iso15693_poller_clamp_block_size(4), 4);
    CHECK_EQ(iso15693_poller_clamp_block_size(32), 32); // exactly the buffer is not over it
    CHECK_EQ(iso15693_poller_clamp_block_size(33), 32);
    CHECK_EQ(iso15693_poller_clamp_block_size(255), 32); // the whole of a hand-edited uint8_t
    CHECK_EQ(iso15693_poller_clamp_block_size(0), 0); // callers reject 0 themselves; do not mask it
    end();
}

int main(void) {
    printf("iso15693 wipe sweep\n");
    test_clean_64();
    test_advertises_28_holds_64();
    test_advertises_66_holds_64();
    test_dead_stretch_inside_claim_is_counted();
    test_dead_stretch_before_activation_is_dropped();
    test_locked_block_holding_data();
    test_locked_but_already_empty_is_not_a_failure();
    test_interior_dropout_that_recovers();
    test_advertised_zero();
    test_zero_block_size_reports_no_claim();
    test_sweep_stops_short_of_56_at_48();
    test_sweep_reaches_56_and_57_at_49();
    test_full_256_hits_the_ceiling();
    test_clock_cuts_the_sweep();
    test_cut_index_exceeds_total_after_a_tail_drop();
    test_cut_index_can_exceed_the_advertised_count();
    test_card_lifted_mid_sweep();
    test_summary_line_is_emitted();
    test_pass_cut_records_it_on_the_instance();
    test_pass_cut_survives_a_tick_wraparound();
    test_block_size_clamps_to_the_buffer();

    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
