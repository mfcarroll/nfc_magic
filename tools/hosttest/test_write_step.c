// Host-side tests for the ISO15693 write state machine.
//
// Unlike the other three files, these do not call one function -- they drive the real poller callback the
// way the SDK does: build an NfcGenericEvent, call iso15693_poller_nfc_callback, and honour the NfcCommand
// it returns. NfcCommandReset power-cycles the fake tag, NfcCommandStop ends the run. So the field resets,
// the activation-error budgets and the gen2-then-gen1 sequencing are all exercised rather than assumed.
//
// The events the poller reports are recorded, so a test can assert the whole sequence and not just the
// terminal one -- CardDetected firing exactly once is part of the contract.

#include "fake_tag.h"

#include "../../magic/protocols/iso15693/iso15693_poller.c" // NOLINT -- see README.md

#include <stdio.h>

static int tests_run;
static int tests_failed;
static const char* current_test;
static bool current_failed;

#define CHECK(cond)                                                  \
    do {                                                             \
        if(!(cond)) {                                                \
            printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            current_failed = true;                                   \
        }                                                            \
    } while(0)

#define CHECK_EQ(actual, expected)                                                                 \
    do {                                                                                           \
        long long a_ = (long long)(actual), e_ = (long long)(expected);                            \
        if(a_ != e_) {                                                                             \
            printf(                                                                                \
                "  FAIL %s:%d  %s == %lld, expected %lld\n", __FILE__, __LINE__, #actual, a_, e_); \
            current_failed = true;                                                                 \
        }                                                                                          \
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

// ---- the driver: what the SDK's poller loop does -------------------------------------------------

#define MAX_EVENTS (64U)

typedef struct {
    Iso15693PollerEvent seen[MAX_EVENTS];
    size_t count;
    uint32_t resets; // NfcCommandReset returned -> field power-cycles
    uint32_t activations; // Ready events delivered
} RunLog;

static RunLog run_log;

static void record_event(Iso15693PollerEvent event, void* context) {
    (void)context;
    if(run_log.count < MAX_EVENTS) run_log.seen[run_log.count++] = event;
}

static bool saw_event(Iso15693PollerEvent e) {
    for(size_t i = 0; i < run_log.count; i++) {
        if(run_log.seen[i] == e) return true;
    }
    return false;
}

static size_t count_event(Iso15693PollerEvent e) {
    size_t n = 0;
    for(size_t i = 0; i < run_log.count; i++) {
        if(run_log.seen[i] == e) n++;
    }
    return n;
}

// The terminal event is the last one reported; everything before it is progress or CardDetected.
static Iso15693PollerEvent terminal_event(void) {
    return run_log.count ? run_log.seen[run_log.count - 1] : Iso15693PollerEventWriteProgress;
}

// Run the poller to completion. `activation_failures` Error events are delivered before each Ready, so a
// test can starve the activation budget the way an absent card does.
static void run_poller(Iso15693Poller* inst, uint32_t activation_failures_per_activation) {
    memset(&run_log, 0, sizeof(run_log));
    inst->callback = record_event;
    inst->context = NULL;

    Iso15693_3PollerEvent ready = {.type = Iso15693_3PollerEventTypeReady};
    Iso15693_3PollerEvent error = {.type = Iso15693_3PollerEventTypeError};

    NfcGenericEvent ev = {.protocol = NfcProtocolIso15693_3, .instance = NULL, .event_data = NULL};

    for(uint32_t guard = 0; guard < 2000; guard++) {
        // Failed activations first, exactly as iso15693_3_poller_run delivers them.
        for(uint32_t i = 0; i < activation_failures_per_activation; i++) {
            ev.event_data = &error;
            const NfcCommand cmd = iso15693_poller_nfc_callback(ev, inst);
            if(cmd == NfcCommandStop) return;
            if(cmd == NfcCommandReset) {
                run_log.resets++;
                fake_tag_power_cycle();
            }
        }

        ev.event_data = &ready;
        run_log.activations++;
        const NfcCommand cmd = iso15693_poller_nfc_callback(ev, inst);
        if(cmd == NfcCommandStop) return;
        if(cmd == NfcCommandReset) {
            run_log.resets++;
            fake_tag_power_cycle();
        }
    }
    printf("  (driver guard tripped -- the poller never returned Stop)\n");
    current_failed = true;
}

// A poller instance set up the way start_internal leaves one, for `mode`.
static Iso15693Poller make_poller(Iso15693PollerMode mode, bool gen1) {
    Iso15693Poller inst;
    memset(&inst, 0, sizeof(inst));
    inst.mode = mode;
    inst.write_state = Iso15693WriteStateStart;
    inst.attempt_gen1 = gen1;
    inst.gen1_attempted = gen1;
    inst.progress_step = UINT8_MAX;
    return inst;
}

static const uint8_t TARGET_UID[ISO15693_3_UID_SIZE] =
    {0xE0, 0x04, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

// ---- Write UID -----------------------------------------------------------------------------------

// The happy path on a gen2 magic card: send the backdoor UID, power-cycle, read it back, done.
static void test_write_uid_gen2_success(void) {
    begin("Write UID on a gen2 magic card is Success after one field reset");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, false);
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventSuccess);
    CHECK_EQ(count_event(Iso15693PollerEventCardDetected), 1); // exactly once, not per activation
    CHECK_EQ(run_log.resets, 1); // one power-cycle before the verify
    CHECK(memcmp(fake_tag.uid, TARGET_UID, ISO15693_3_UID_SIZE) == 0);
    end();
}

// A tag that ignores the gen2 backdoor must be reported as NotGen2 -- and nothing may have been written,
// because the scene offers the destructive gen1 opt-in off the back of this event.
static void test_non_magic_tag_reports_not_gen2(void) {
    begin("a tag that ignores gen2 reports NotGen2 with the UID untouched");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = false;
    uint8_t before[ISO15693_3_UID_SIZE];
    memcpy(before, fake_tag.uid, ISO15693_3_UID_SIZE);

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, false);
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventNotGen2);
    CHECK(memcmp(fake_tag.uid, before, ISO15693_3_UID_SIZE) == 0);
    CHECK(!inst.uid_unexpected);
    end();
}

// Asking to write the UID the card already has proves nothing -- any tag passes the read-back having
// ignored every frame. The run must stop BEFORE writing and say so.
static void test_write_uid_matching_current_is_unverifiable(void) {
    begin("writing the card's own UID back is refused as unverifiable");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, false);
    memcpy(inst.target_uid, fake_tag.uid, ISO15693_3_UID_SIZE); // the card's own UID

    const uint32_t frames_before = fake_tag.ops;
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventFail);
    CHECK(inst.uid_unverifiable);
    CHECK_EQ(run_log.resets, 0); // stopped before any write, so no power-cycle
    CHECK_EQ(fake_tag.ops, frames_before); // and no frames went out at all
    end();
}

// The one outcome that PROVES the card is magic: the UID moved, but to neither the original nor the
// target. It must not be reported as "not a magic tag", and the UID it now answers to is the only way
// the user finds the card again.
static void test_uid_moved_somewhere_unexpected(void) {
    begin("a UID that moves to neither original nor target is recorded, not called non-magic");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, false);
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);

    // The card takes a magic write but lands somewhere else entirely.
    const uint8_t elsewhere[ISO15693_3_UID_SIZE] = {
        0xE0, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99};
    fake_tag.is_gen2_magic = false; // ignore the backdoor UID...
    fake_tag_set_uid_now(elsewhere); // ...but the UID is not what it was, nor the target

    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventFail);
    CHECK(inst.uid_unexpected);
    CHECK(memcmp(inst.uid_readback, elsewhere, ISO15693_3_UID_SIZE) == 0);
    end();
}

// ---- the gen1 opt-in -----------------------------------------------------------------------------

// gen1 writes the UID with ordinary WRITE BLOCKs into 56/57/62/63 and the card latches it only on the
// next power-up. So the verify has to sit behind a reset -- read inline it would see the old UID and
// report failure on exactly the card gen1 works on.
static void test_gen1_uid_latches_on_the_power_cycle(void) {
    begin("gen1 latches its UID on the field reset, and the verify sees it");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = true;
    fake_tag.is_gen2_magic = false;

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, true);
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventSuccess);
    CHECK_EQ(run_log.resets, 1);
    CHECK(memcmp(fake_tag.uid, TARGET_UID, ISO15693_3_UID_SIZE) == 0);
    CHECK(inst.gen1_attempted); // reported whatever the outcome, since the frames went out
    end();
}

// A tag that is not gen1 either: the four backdoor registers were still overwritten by ordinary writes,
// so the result must carry gen1_attempted rather than only "not a magic tag".
static void test_gen1_failure_still_reports_the_spent_attempt(void) {
    begin("a failed gen1 attempt still reports that the four blocks were written");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = false;
    fake_tag.is_gen2_magic = false;

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, true);
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventFail);
    CHECK(inst.gen1_attempted);
    CHECK(!inst.clone_used_gen1); // the UID did not take
    end();
}

// ---- the wipe's UID re-check ---------------------------------------------------------------------

// A clean wipe: blocks cleared, then the UID re-read behind a power-cycle and found unchanged.
static void test_wipe_verifies_the_uid_unchanged(void) {
    begin("a wipe re-reads the UID behind a reset and reports Success");
    fake_tag_init(64, 64, 4);

    Iso15693Poller inst = make_poller(Iso15693PollerModeWipe, false);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventSuccess);
    CHECK_EQ(run_log.resets, 1); // the wipe's single power-cycle before VerifyWipe
    CHECK(inst.uid_verified);
    CHECK(!inst.uid_changed);
    end();
}

// The case the check exists for: an ALREADY-ARMED gen1 card, where zeroing blocks 56/57 is itself a gen1
// UID write. The wipe cannot prevent it, but it must not promise the UID is untouched -- it reports the
// change and prints what the card now answers to.
static void test_wipe_on_an_armed_gen1_card_reports_the_uid_change(void) {
    begin("a wipe that moves the UID on an armed gen1 card reports it");
    fake_tag_init(64, 64, 4);
    // Zeroing 56/57 on an armed card writes an all-zero UID, latched at the next power-up.
    const uint8_t zeros[ISO15693_3_UID_SIZE] = {0};
    fake_tag_arm_gen1_uid(zeros);

    Iso15693Poller inst = make_poller(Iso15693PollerModeWipe, false);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventPartial); // not a clean Success
    CHECK(inst.uid_verified);
    CHECK(inst.uid_changed);
    CHECK(memcmp(inst.uid_readback, zeros, ISO15693_3_UID_SIZE) == 0);
    end();
}

// A card that never comes back from the power-cycle. The wipe already happened and the UID check is
// best-effort, so its result must still be reported rather than thrown away -- the user has most likely
// just picked the card up. This is the uid_verified-false path, previously untested.
static void test_wipe_card_gone_after_reset_still_reports(void) {
    begin("a card that never returns from the reset still gets its wipe reported");
    fake_tag_init(64, 64, 4);

    Iso15693Poller inst = make_poller(Iso15693PollerModeWipe, false);
    // Enough failed activations to exhaust ISO15693_POLLER_WIPE_VERIFY_ACTIVATIONS after the reset.
    run_poller(&inst, ISO15693_POLLER_WIPE_VERIFY_ACTIVATIONS + 1);

    CHECK_EQ(terminal_event(), Iso15693PollerEventSuccess); // the wipe itself finished
    CHECK(!inst.uid_verified); // but the check never reached an answer
    CHECK(!inst.uid_changed); // absence of proof is NOT a reported change
    CHECK(strstr(fake_log_text(), "did not return after the field reset") != NULL);
    end();
}

// The shorter budget is the point: the post-wipe wait must not sit through the full no-card timeout, or
// the popup freezes for seconds on the common case of the user lifting the card as the wipe ends.
static void test_wipe_verify_budget_is_the_short_one(void) {
    begin("the post-wipe verify uses the short activation budget, not the full one");
    CHECK(ISO15693_POLLER_WIPE_VERIFY_ACTIVATIONS < ISO15693_POLLER_MAX_ACTIVATION_ERRORS);

    fake_tag_init(64, 64, 4);
    Iso15693Poller inst = make_poller(Iso15693PollerModeWipe, false);
    // One MORE than the short budget but fewer than the long one: must give up, not keep waiting.
    run_poller(&inst, ISO15693_POLLER_WIPE_VERIFY_ACTIVATIONS + 1);

    CHECK_EQ(terminal_event(), Iso15693PollerEventSuccess);
    CHECK(!inst.uid_verified);
    end();
}

// No card at all, in a mode that has not written anything: report CardLost once the full budget is spent.
static void test_no_card_reports_card_lost(void) {
    begin("no card at all reports CardLost on the full budget");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, false);
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    // Never deliver a Ready: only errors, past the full budget.
    memset(&run_log, 0, sizeof(run_log));
    inst.callback = record_event;
    Iso15693_3PollerEvent error = {.type = Iso15693_3PollerEventTypeError};
    NfcGenericEvent ev = {
        .protocol = NfcProtocolIso15693_3, .instance = NULL, .event_data = &error};
    NfcCommand cmd = NfcCommandContinue;
    for(uint32_t i = 0; i < ISO15693_POLLER_MAX_ACTIVATION_ERRORS + 2 && cmd != NfcCommandStop;
        i++) {
        cmd = iso15693_poller_nfc_callback(ev, &inst);
    }

    CHECK_EQ(cmd, NfcCommandStop);
    CHECK_EQ(terminal_event(), Iso15693PollerEventCardLost);
    end();
}

// An event from another protocol must be ignored outright rather than misread as this one's.
static void test_foreign_protocol_event_is_ignored(void) {
    begin("an event from another protocol is ignored");
    fake_tag_init(64, 64, 4);
    Iso15693Poller inst = make_poller(Iso15693PollerModeWipe, false);
    memset(&run_log, 0, sizeof(run_log));
    inst.callback = record_event;

    NfcGenericEvent ev = {
        .protocol = NfcProtocolIso14443_3a, .instance = NULL, .event_data = NULL};
    const NfcCommand cmd = iso15693_poller_nfc_callback(ev, &inst);

    CHECK_EQ(cmd, NfcCommandContinue);
    CHECK_EQ(run_log.count, 0); // nothing reported
    CHECK_EQ(inst.write_state, Iso15693WriteStateStart); // state untouched
    end();
}

// ---- clone through the state machine -------------------------------------------------------------

// End to end: gen2 UID takes, then the data blocks are written -- and NOT before, so a tag that refuses
// the UID is never clobbered by a doomed clone.
static void test_clone_writes_data_only_after_the_uid_takes(void) {
    begin("a clone writes its data blocks only after the gen2 UID verifies");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;

    static Iso15693_3Data src;
    fake_data_init(&src, 28, 4);

    Iso15693Poller inst = make_poller(Iso15693PollerModeClone, false);
    inst.clone_source = &src;
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventSuccess);
    CHECK_EQ(inst.clone_blocks_total, 28);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK(fake_tag.writes_accepted >= 28); // the data pass ran
    CHECK(saw_event(Iso15693PollerEventWriteProgress)); // and reported progress
    end();
}

// The seam iso15693_poller_finish_write exists to protect. Both UID verifies end in that one call and
// differ ONLY in skip_backdoor -- gen2 writes the backdoor blocks because its UID lives elsewhere, gen1
// must skip them because its UID lives IN them. Nothing tested that flag when the tail was extracted:
// flipping it at either call site left all 107 cases green. So assert it at both, via the one field it
// moves -- clone_blocks_total, which write_source_blocks deducts the four skipped registers from.
static void test_the_two_verify_arms_disagree_about_the_backdoor_blocks(void) {
    begin("a gen2 clone counts the backdoor blocks and a gen1 clone deducts them");
    static Iso15693_3Data src;

    // gen2: UID lives in its own register space, so all 64 source blocks are in scope.
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;
    fake_data_init(&src, 64, 4);
    Iso15693Poller gen2 = make_poller(Iso15693PollerModeClone, false);
    gen2.clone_source = &src;
    memcpy(gen2.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&gen2, 0);
    CHECK_EQ(gen2.clone_blocks_total, 64);

    // gen1: the UID occupies 56/57/62/63, so the data pass must skip them and not count them.
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = false;
    fake_tag.is_gen1_magic = true;
    fake_data_init(&src, 64, 4);
    Iso15693Poller gen1 = make_poller(Iso15693PollerModeClone, true);
    gen1.clone_source = &src;
    memcpy(gen1.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&gen1, 0);
    CHECK_EQ(gen1.clone_blocks_total, 60); // 64 - the four backdoor registers
    end();
}

// The protective ordering, stated as a test: a tag that refuses the gen2 UID must have NO data written.
static void test_clone_on_non_magic_writes_nothing(void) {
    begin("a clone onto a tag that refuses the gen2 UID writes no data at all");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = false;

    static Iso15693_3Data src;
    fake_data_init(&src, 28, 4);

    Iso15693Poller inst = make_poller(Iso15693PollerModeClone, false);
    inst.clone_source = &src;
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventNotGen2);
    CHECK_EQ(fake_tag.writes_accepted, 0); // nothing was clobbered
    end();
}

// A clone with no usable geometry is refused before the card is touched, so the app cannot stamp a UID
// and then report a misleading "clone complete".
static void test_empty_source_clone_is_refused_before_writing(void) {
    begin("an empty clone source is refused before any frame goes out");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;

    static Iso15693_3Data src;
    fake_data_init(&src, 0, 4);

    Iso15693Poller inst = make_poller(Iso15693PollerModeClone, false);
    inst.clone_source = &src;
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    const uint32_t ops_before = fake_tag.ops;
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventFail);
    CHECK_EQ(inst.clone_blocks_total, 0); // the scene reads this as "empty source"
    CHECK_EQ(fake_tag.ops, ops_before); // nothing transmitted
    CHECK_EQ(run_log.resets, 0);
    end();
}

int main(void) {
    printf("iso15693 write state machine\n");
    test_write_uid_gen2_success();
    test_non_magic_tag_reports_not_gen2();
    test_write_uid_matching_current_is_unverifiable();
    test_uid_moved_somewhere_unexpected();
    test_gen1_uid_latches_on_the_power_cycle();
    test_gen1_failure_still_reports_the_spent_attempt();
    test_wipe_verifies_the_uid_unchanged();
    test_wipe_on_an_armed_gen1_card_reports_the_uid_change();
    test_wipe_card_gone_after_reset_still_reports();
    test_wipe_verify_budget_is_the_short_one();
    test_no_card_reports_card_lost();
    test_foreign_protocol_event_is_ignored();
    test_clone_writes_data_only_after_the_uid_takes();
    test_the_two_verify_arms_disagree_about_the_backdoor_blocks();
    test_clone_on_non_magic_writes_nothing();
    test_empty_source_clone_is_refused_before_writing();

    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
