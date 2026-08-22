// Host-side tests for the ISO15693 terminal-outcome contract.
//
// iso15693_poller_success_or_partial is a pure function of the poller's result fields: no radio, no
// clock. It decides which of Success / Partial / Fail the scene receives, so it is the single place
// where "what happened" becomes "what the user is told", and every review round so far has landed on
// some part of it.
//
// The tests are written as the contract rather than as the implementation: each one names the outcome a
// user should see and why. A change that makes one fail is a change to what the app promises.

#include "fake_tag.h"

#include "../../magic/protocols/iso15693/iso15693_poller.c" // NOLINT -- see README.md

#include <stdio.h>

static int tests_run;
static int tests_failed;
static const char* current_test;
static bool current_failed;

static const char* event_name(Iso15693PollerEvent e) {
    switch(e) {
    case Iso15693PollerEventSuccess:
        return "Success";
    case Iso15693PollerEventPartial:
        return "Partial";
    case Iso15693PollerEventFail:
        return "Fail";
    case Iso15693PollerEventCardLost:
        return "CardLost";
    case Iso15693PollerEventCardDetected:
        return "CardDetected";
    case Iso15693PollerEventWriteProgress:
        return "WriteProgress";
    case Iso15693PollerEventNotGen2:
        return "NotGen2";
    default:
        return "?";
    }
}

#define CHECK_OUTCOME(inst, expected)                                             \
    do {                                                                          \
        const Iso15693PollerEvent got_ = iso15693_poller_success_or_partial(inst); \
        if(got_ != (expected)) {                                                  \
            printf("  FAIL %s:%d  got %s, expected %s\n",                         \
                   __FILE__, __LINE__, event_name(got_), event_name(expected));   \
            current_failed = true;                                                \
        }                                                                         \
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
    } else {
        printf("  ok  %s\n", current_test);
    }
}

// A finished clone with nothing wrong: 28 of 28 blocks written, UID took, identity fields reproduced.
static Iso15693Poller clean_clone(void) {
    Iso15693Poller inst;
    memset(&inst, 0, sizeof(inst));
    inst.mode = Iso15693PollerModeClone;
    inst.clone_blocks_total = 28;
    inst.uid_verified = true;
    return inst;
}

// A finished wipe with nothing wrong: 64 blocks proven and cleared, UID re-read and unchanged.
static Iso15693Poller clean_wipe(void) {
    Iso15693Poller inst;
    memset(&inst, 0, sizeof(inst));
    inst.mode = Iso15693PollerModeWipe;
    inst.clone_blocks_total = 64;
    inst.wipe_advertised = 64;
    inst.uid_verified = true;
    return inst;
}

// ---- the contract ---------------------------------------------------------------------------------

static void test_clean_clone_is_success(void) {
    begin("a clone with nothing wrong is Success");
    Iso15693Poller inst = clean_clone();
    CHECK_OUTCOME(&inst, Iso15693PollerEventSuccess);
    end();
}

static void test_clean_wipe_is_success(void) {
    begin("a wipe with nothing wrong is Success");
    Iso15693Poller inst = clean_wipe();
    CHECK_OUTCOME(&inst, Iso15693PollerEventSuccess);
    end();
}

// Any block that would not take its write is real data lost or a promise unkept, so the result is
// qualified rather than clean.
static void test_any_failed_block_is_partial(void) {
    begin("a single failed block makes either operation Partial");
    Iso15693Poller clone = clean_clone();
    clone.clone_failed_count = 1;
    CHECK_OUTCOME(&clone, Iso15693PollerEventPartial);

    Iso15693Poller wipe = clean_wipe();
    wipe.clone_failed_count = 1;
    CHECK_OUTCOME(&wipe, Iso15693PollerEventPartial);
    end();
}

// An over-capacity note is NOT a defect: the source held nothing in those blocks, so nothing was lost.
// It stays a Success and the scene adds the note. (scene_write.c routes it to the result screen so the
// count is visible, which is a different thing from downgrading the outcome.)
static void test_over_capacity_alone_is_still_success(void) {
    begin("an empty over-capacity tail stays Success");
    Iso15693Poller inst = clean_clone();
    inst.clone_over_capacity = 6;
    inst.clone_capacity_confirmed = true;
    CHECK_OUTCOME(&inst, Iso15693PollerEventSuccess);
    end();
}

// gen1 stores the UID and commit words in data blocks 56/57/62/63, so a clone that fell back to gen1
// cannot be byte-identical to its source there.
static void test_gen1_clone_is_partial(void) {
    begin("a clone that fell back to gen1 is Partial");
    Iso15693Poller inst = clean_clone();
    inst.clone_used_gen1 = true;
    CHECK_OUTCOME(&inst, Iso15693PollerEventPartial);
    end();
}

// ...but a bare Write-UID has no source data to disturb, so gen1 there is a clean Success. This is the
// asymmetry the contract comment calls out, and it is easy to break by hoisting the gen1 test out of
// the clone-only guard.
static void test_gen1_write_uid_is_success(void) {
    begin("gen1 on a bare Write-UID is Success, not Partial");
    Iso15693Poller inst;
    memset(&inst, 0, sizeof(inst));
    inst.mode = Iso15693PollerModeWriteUid;
    inst.clone_used_gen1 = true;
    inst.uid_verified = true;
    CHECK_OUTCOME(&inst, Iso15693PollerEventSuccess);
    end();
}

// AFI / DSFID are identity extras, not the core payload, so a rejected one qualifies the clone without
// failing it -- and only for a clone, since nothing else writes them.
static void test_identity_failure_is_partial_for_a_clone_only(void) {
    begin("a rejected AFI or DSFID is Partial, and clone-only");
    Iso15693Poller afi = clean_clone();
    afi.clone_afi_failed = true;
    CHECK_OUTCOME(&afi, Iso15693PollerEventPartial);

    Iso15693Poller dsfid = clean_clone();
    dsfid.clone_dsfid_failed = true;
    CHECK_OUTCOME(&dsfid, Iso15693PollerEventPartial);

    // The same flags on a wipe mean nothing -- a wipe writes no identity fields.
    Iso15693Poller wipe = clean_wipe();
    wipe.clone_afi_failed = true;
    wipe.clone_dsfid_failed = true;
    CHECK_OUTCOME(&wipe, Iso15693PollerEventSuccess);
    end();
}

// A wipe that cleared its blocks but moved the card's UID is not clean whatever the counts say: the
// card's identity changed under an operation that never sends a UID command.
static void test_uid_changed_is_partial(void) {
    begin("a wipe that moved the UID is Partial whatever the counts say");
    Iso15693Poller inst = clean_wipe();
    inst.uid_changed = true;
    CHECK_OUTCOME(&inst, Iso15693PollerEventPartial);
    end();
}

// A sweep the clock cut left blocks the card claims unattempted, so the operation's own job is unfinished
// -- which is what Partial means. Previously shipped on reasoning alone.
static void test_truncated_sweep_is_partial(void) {
    begin("a sweep the clock cut is Partial");
    Iso15693Poller inst = clean_wipe();
    inst.pass_truncated = true;
    CHECK_OUTCOME(&inst, Iso15693PollerEventPartial);
    end();
}

// His round-6 blocking finding. The all-rejected guard sits AHEAD of the Partial test and judges on
// counts alone -- and on a cut clone the back-fill has already recorded every block above the cut as a
// failure, so failed_count reaches blocks_total whether the card refused those blocks or nothing was
// ever sent to them. Judged there, the run reported "no data block took" about a source the radio never
// addressed: no cut note, no Details, no Retry, on the outcome with the MOST blocks above the cut.
static void test_cut_clone_that_accepted_nothing_is_partial_not_fail(void) {
    begin("a cut clone that accepted nothing is Partial, not Fail");
    Iso15693Poller inst = clean_clone();
    inst.clone_blocks_total = 256;
    inst.clone_failed_count = 256; // every block accounted a failure: some refused, most never sent
    inst.pass_truncated = true;
    inst.pass_cut_block = 40;
    CHECK_OUTCOME(&inst, Iso15693PollerEventPartial);
    end();
}

// The guard's own purpose has to survive the exclusion: an UNCUT clone that accepted nothing is still a
// Fail, because there a full count really does mean the card refused every block.
static void test_uncut_clone_that_accepted_nothing_is_still_fail(void) {
    begin("an uncut clone that accepted nothing is still Fail");
    Iso15693Poller inst = clean_clone();
    inst.clone_blocks_total = 28;
    inst.clone_failed_count = 28;
    inst.pass_truncated = false;
    CHECK_OUTCOME(&inst, Iso15693PollerEventFail);
    end();
}

// The same flag on a clone. It used to be wipe-only, which is how a cut clone reached the report with
// nothing marking it as cut -- so its unattempted blocks arrived indistinguishable from refused ones.
// It qualifies for the same reason a sweep does: the operation's own job is left undone.
static void test_truncated_clone_is_partial(void) {
    begin("a clone the clock cut is Partial");
    Iso15693Poller inst = clean_clone();
    inst.pass_truncated = true;
    CHECK_OUTCOME(&inst, Iso15693PollerEventPartial);
    end();
}

// The UID check failing to REACH an answer is deliberately not a downgrade. It is best-effort, the wipe
// itself finished, and making it Partial would flag every wipe where the user lifts the card as it
// completes. This test exists because uid_verified's absence from the Partial list reads like an
// oversight, and a future reader "fixing" it would regress every normal wipe.
static void test_unverified_uid_alone_is_not_a_downgrade(void) {
    begin("an unreached UID check alone does NOT downgrade a wipe");
    Iso15693Poller inst = clean_wipe();
    inst.uid_verified = false;
    CHECK_OUTCOME(&inst, Iso15693PollerEventSuccess);
    end();
}

// A clone whose UID took but whose every data block was rejected has written no data at all. Calling
// that Partial would put a Finish button under "Cloned 0/28 blocks", and the card would carry the
// source's UID with none of its data -- correct to a UID-only reader, broken to anything reading memory.
static void test_clone_with_every_block_rejected_is_fail(void) {
    begin("a clone with every block rejected is Fail, not Partial");
    Iso15693Poller inst = clean_clone();
    inst.clone_failed_count = 28; // == blocks_total
    CHECK_OUTCOME(&inst, Iso15693PollerEventFail);
    end();
}

// The boundary: one block through is a genuine partial clone, so the guard must be >= and not >.
static void test_one_block_written_is_partial_not_fail(void) {
    begin("one block written out of 28 is Partial, not Fail");
    Iso15693Poller inst = clean_clone();
    inst.clone_failed_count = 27;
    CHECK_OUTCOME(&inst, Iso15693PollerEventPartial);

    inst.clone_failed_count = 28;
    CHECK_OUTCOME(&inst, Iso15693PollerEventFail); // the very next value flips it
    end();
}

// The all-rejected guard is clone-only and must not catch a wipe. A wipe reaches this function only when
// at least one block accepted, and its failed and accepted sets are disjoint -- so failed_count can equal
// blocks_total there while blocks really were cleared.
static void test_all_rejected_guard_does_not_catch_a_wipe(void) {
    begin("the all-rejected guard is clone-only and spares a wipe");
    Iso15693Poller inst = clean_wipe();
    inst.clone_blocks_total = 64;
    inst.clone_failed_count = 64;
    CHECK_OUTCOME(&inst, Iso15693PollerEventPartial); // Partial, never Fail
    end();
}

// An empty source leaves blocks_total 0, and the guard is written to skip that case so the "empty source"
// reason survives to the scene instead of being overwritten by the all-rejected one.
static void test_empty_source_skips_the_all_rejected_guard(void) {
    begin("blocks_total 0 does not trip the all-rejected guard");
    Iso15693Poller inst = clean_clone();
    inst.clone_blocks_total = 0;
    inst.clone_failed_count = 0;
    CHECK_OUTCOME(&inst, Iso15693PollerEventSuccess);
    end();
}

int main(void) {
    printf("iso15693 terminal outcome\n");
    test_clean_clone_is_success();
    test_clean_wipe_is_success();
    test_any_failed_block_is_partial();
    test_over_capacity_alone_is_still_success();
    test_gen1_clone_is_partial();
    test_gen1_write_uid_is_success();
    test_identity_failure_is_partial_for_a_clone_only();
    test_uid_changed_is_partial();
    test_truncated_sweep_is_partial();
    test_truncated_clone_is_partial();
    test_cut_clone_that_accepted_nothing_is_partial_not_fail();
    test_uncut_clone_that_accepted_nothing_is_still_fail();
    test_unverified_uid_alone_is_not_a_downgrade();
    test_clone_with_every_block_rejected_is_fail();
    test_one_block_written_is_partial_not_fail();
    test_all_rejected_guard_does_not_catch_a_wipe();
    test_empty_source_skips_the_all_rejected_guard();

    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
