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
    size_t reset_at[8]; // how many events had been reported when each reset came
    uint32_t activations; // Ready events delivered
} RunLog;

static RunLog run_log;

static void note_reset(void) {
    if(run_log.resets < 8) run_log.reset_at[run_log.resets] = run_log.count;
    run_log.resets++;
    fake_tag_power_cycle();
}

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
            if(cmd == NfcCommandReset) note_reset();
        }

        ev.event_data = &ready;
        run_log.activations++;
        const NfcCommand cmd = iso15693_poller_nfc_callback(ev, inst);
        if(cmd == NfcCommandStop) return;
        if(cmd == NfcCommandReset) note_reset();
    }
    printf("  (driver guard tripped -- the poller never returned Stop)\n");
    current_failed = true;
}

// A poller instance set up the way start_internal leaves one, for `mode`.
static BitBuffer* step_tx;
static BitBuffer* step_rx;

// Only the buffers iso15693_poller_alloc owns. NOT address_uid: setting that here would hide a
// write_step that never takes the card's address, which is the thing these tests are for.
static Iso15693Poller make_poller(Iso15693PollerMode mode, bool gen1) {
    if(step_tx == NULL) {
        step_tx = bit_buffer_alloc(ISO15693_POLLER_BUF_SIZE);
        step_rx = bit_buffer_alloc(ISO15693_POLLER_BUF_SIZE);
    }
    Iso15693Poller inst;
    memset(&inst, 0, sizeof(inst));
    inst.frame_tx = step_tx;
    inst.frame_rx = step_rx;
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

// gen1 writes the UID with ordinary WRITE BLOCKs into 56/57/62/63, and the verify sits behind a field
// reset. NOT because the card latches -- measured on three chips, it does not; the reset is there to
// re-activate cleanly and read a card whose identity has just moved out from under the session. The
// reset count is asserted rather than inferred, because the fake now moves the UID immediately and so
// cannot fail a verify that skipped it.
static void test_gen1_uid_takes_and_the_verify_sits_behind_a_reset(void) {
    begin("a gen1 UID write takes, and its verify runs behind a field reset");
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
    CHECK(!inst.uid_unexpected); // ...and did not move either, so there is no new UID to report
    end();
}

// The gen1 sequence re-addresses between 56 and 57, and if 57 is lost the card is left with half a UID,
// neither its own nor the target. That card IS gen1, and the UID it answers to now is the only way to
// find it again, so the gen1 verify reports it the way the gen2 verify reports one.
static void test_a_half_written_gen1_uid_is_reported_as_unexpected(void) {
    begin("a gen1 run left with half a UID reports the UID the card now answers to");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = true;
    fake_tag.is_gen2_magic = false;
    fake_tag.uid_register_drop_mask = 0x2; // 56 lands, 57 is lost

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, true);
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventFail);
    CHECK(inst.uid_unexpected);
    CHECK(memcmp(inst.uid_readback, fake_tag.uid, ISO15693_3_UID_SIZE) == 0);
    CHECK(memcmp(fake_tag.uid, TARGET_UID, ISO15693_3_UID_SIZE) != 0); // half of it, not all
    end();
}

// The other half: 56's frame lost, so the card keeps answering to its own address, 57 lands there, and
// the card is left with the target's head under its own tail -- the second of the two UIDs a half can
// leave.
static void test_a_gen1_run_that_wrote_only_57_is_reported_as_unexpected(void) {
    begin("a gen1 run that wrote only block 57 reports the UID the card now answers to");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = true;
    fake_tag.is_gen2_magic = false;
    fake_tag.uid_register_drop_mask = 0x1; // 56 is lost, 57 lands

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, true);
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventFail);
    CHECK(inst.uid_unexpected);
    CHECK(memcmp(inst.uid_readback, fake_tag.uid, ISO15693_3_UID_SIZE) == 0);
    CHECK(memcmp(&fake_tag.uid[0], &TARGET_UID[0], 4) == 0); // the target's head...
    CHECK(memcmp(&fake_tag.uid[4], &TARGET_UID[4], 4) != 0); // ...and not its tail
    end();
}

// With a second tag in the field the 1-slot inventory can answer for it (#251), so a UID that is
// neither the card's nor the target may be that tag's. Only the two a half-written sequence leaves are
// this card's. Anything else keeps the gen1-failed report, whose screen names the four blocks the
// user's consent has just cost -- printing the stranger's UID as this card's would replace it.
static void test_a_stranger_at_the_gen1_verify_keeps_the_spent_blocks_report(void) {
    begin("a stranger's UID at the gen1 verify is not reported as this card's");
    fake_tag_init(64, 64, 4); // an ordinary tag, so the sequence lands in its memory
    fake_tag.is_gen1_magic = false;
    fake_tag.is_gen2_magic = false;
    fake_tag.bystander_answers_inventory = true;
    memset(fake_tag.bystander_uid, 0x77, sizeof(fake_tag.bystander_uid));

    Iso15693Poller inst = make_poller(Iso15693PollerModeWriteUid, true);
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventFail);
    CHECK(inst.gen1_attempted); // the report that names 56/57/62/63...
    CHECK(!inst.uid_unexpected); // ...not one printing the stranger's UID as this card's
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

// ---- the re-read after a gen2-path pass that wrote 56/57 ------------------------------------------

// On gen2 silicon 56/57 are memory and the UID cannot move, so the re-read finds the target and the
// result stands -- one field reset more than a clone that stops below 56, and nothing else.
static void test_a_clone_that_wrote_56_rereads_the_uid_behind_a_reset(void) {
    begin("a gen2-path clone that wrote 56/57 re-reads the UID behind a second reset");
    static Iso15693_3Data src;

    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;
    fake_data_init(&src, 64, 4);
    Iso15693Poller wide = make_poller(Iso15693PollerModeClone, false);
    wide.clone_source = &src;
    memcpy(wide.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&wide, 0);
    CHECK_EQ(terminal_event(), Iso15693PollerEventSuccess);
    CHECK_EQ(run_log.resets, 2); // before the gen2 verify, and before the re-read
    // ...and 100% waits for the re-read: the last progress frame comes after the second reset.
    size_t last_progress = 0;
    for(size_t i = 0; i < run_log.count; i++) {
        if(run_log.seen[i] == Iso15693PollerEventWriteProgress) last_progress = i;
    }
    CHECK(last_progress >= run_log.reset_at[1]);

    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;
    fake_data_init(&src, 28, 4);
    Iso15693Poller narrow = make_poller(Iso15693PollerModeClone, false);
    narrow.clone_source = &src;
    memcpy(narrow.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&narrow, 0);
    CHECK_EQ(terminal_event(), Iso15693PollerEventSuccess);
    CHECK_EQ(run_log.resets, 1); // never reached 56, so nothing to re-read
    end();
}

// A gen1 card already wearing the file's UID takes the gen2 path, converts at 56, and repairs its UID
// with frames that each go out once. Lose one and the card answers to bytes out of the file, under a
// screen that would otherwise report the clone. The re-read is what catches it.
static void test_a_repair_that_lost_a_frame_reports_the_uid_the_card_answers_to(void) {
    begin("a converted clone whose repair lost a frame reports the UID the card answers to");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = true;
    fake_tag.is_gen2_magic = false;
    fake_tag_set_uid_now(TARGET_UID); // already the file's UID, so the gen2 verify passes
    fake_tag.uid_register_drop_mask = 0x2; // the pass's 56 lands; the repair's is lost
    static Iso15693_3Data src;
    fake_data_init(&src, 64, 4);
    fake_data_fill(&src, 0, 63, 0x5A);

    Iso15693Poller inst = make_poller(Iso15693PollerModeClone, false);
    inst.clone_source = &src;
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK(inst.uid_moved_by_write); // it did convert
    CHECK_EQ(terminal_event(), Iso15693PollerEventFail);
    CHECK(inst.uid_unexpected);
    CHECK(memcmp(inst.uid_readback, fake_tag.uid, ISO15693_3_UID_SIZE) == 0);
    CHECK(memcmp(fake_tag.uid, TARGET_UID, ISO15693_3_UID_SIZE) != 0);
    end();
}

// And a card gone by the re-read has confirmed nothing, so there is no result to report.
static void test_a_card_gone_before_the_reread_is_card_lost(void) {
    begin("a clone whose card is gone at the re-read reports CardLost");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen2_magic = true;
    fake_tag.lifted_at_power_cycle = 2; // the reset before the re-read
    static Iso15693_3Data src;
    fake_data_init(&src, 64, 4);

    Iso15693Poller inst = make_poller(Iso15693PollerModeClone, false);
    inst.clone_source = &src;
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventCardLost);
    // ...but the result says the re-read was due, which is the card-lost screen's cue for its note.
    Iso15693PollerResult result;
    iso15693_poller_get_result(&inst, &result);
    CHECK(result.uid_recheck);
    end();
}

// The same question is left open when the card goes mid-pass, once the pass has sent 56/57 a frame:
// the re-read was due and never ran. That includes a card lost well below 56. The pass goes on sending
// every block its frame and learns the card has gone only at its end, so it cannot tell which of those
// frames the card heard. A pass that ends below 56 leaves no question: nothing it sent could have moved
// the UID.
static void test_a_clone_lost_mid_pass_reports_an_unread_uid_once_56_was_sent(void) {
    begin("a clone lost mid-pass reports its UID unread once its pass sent 56/57 a frame");
    static Iso15693_3Data src;
    const uint16_t source_blocks[2] = {48, 200}; // ends below 56, and runs past it
    for(int run = 0; run < 2; run++) {
        fake_tag_init(200, 200, 4);
        fake_tag.is_gen2_magic = true;
        fake_tag.ops_until_lifted = 40; // about block 34 of either pass
        fake_data_init(&src, source_blocks[run], 4);
        fake_data_fill(&src, 0, (uint16_t)(source_blocks[run] - 1), 0x5A);
        Iso15693Poller inst = make_poller(Iso15693PollerModeClone, false);
        inst.clone_source = &src;
        memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
        run_poller(&inst, 0);

        CHECK_EQ(terminal_event(), Iso15693PollerEventCardLost);
        // The lift was inside the pass: its first block landed, and block 40 never did.
        CHECK_EQ(fake_tag.content[0][0], 0x5A);
        CHECK_EQ(fake_tag.content[40][0], FAKE_MARKER);
        Iso15693PollerResult result;
        iso15693_poller_get_result(&inst, &result);
        CHECK_EQ(result.uid_recheck, run == 1);
    }
    end();
}

// The popup's last frame is the one that says the work is done, so it waits for the work: here that is
// the re-read behind the second reset. A clone that converted reads its own total there, and the blocks
// it attempted come to the same figure, since the write that converted it went to a register the total
// leaves out.
static void test_the_last_progress_frame_waits_for_the_reread(void) {
    begin("a clone's last progress frame follows the re-read, and reads its total");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = true;
    fake_tag.is_gen2_magic = false;
    fake_tag_set_uid_now(TARGET_UID); // gen1 silicon wearing the file's UID: the pass converts
    static Iso15693_3Data src;
    fake_data_init(&src, 64, 4);
    fake_data_fill(&src, 0, 63, 0x5A);

    Iso15693Poller inst = make_poller(Iso15693PollerModeClone, false);
    inst.clone_source = &src;
    memcpy(inst.target_uid, TARGET_UID, ISO15693_3_UID_SIZE);
    run_poller(&inst, 0);

    CHECK_EQ(terminal_event(), Iso15693PollerEventPartial); // its file held data at 56/57/62/63
    CHECK_EQ(run_log.resets, 2);
    size_t last_progress = 0;
    for(size_t i = 0; i < run_log.count; i++) {
        if(run_log.seen[i] == Iso15693PollerEventWriteProgress) last_progress = i;
    }
    CHECK(last_progress >= run_log.reset_at[1]);
    CHECK_EQ(inst.clone_blocks_total, 60);
    CHECK_EQ(inst.clone_blocks_done, inst.clone_blocks_total);
    end();
}

int main(void) {
    printf("iso15693 write state machine\n");
    test_write_uid_gen2_success();
    test_non_magic_tag_reports_not_gen2();
    test_write_uid_matching_current_is_unverifiable();
    test_uid_moved_somewhere_unexpected();
    test_gen1_uid_takes_and_the_verify_sits_behind_a_reset();
    test_gen1_failure_still_reports_the_spent_attempt();
    test_a_half_written_gen1_uid_is_reported_as_unexpected();
    test_a_gen1_run_that_wrote_only_57_is_reported_as_unexpected();
    test_a_stranger_at_the_gen1_verify_keeps_the_spent_blocks_report();
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
    test_a_clone_that_wrote_56_rereads_the_uid_behind_a_reset();
    test_a_repair_that_lost_a_frame_reports_the_uid_the_card_answers_to();
    test_a_card_gone_before_the_reread_is_card_lost();
    test_a_clone_lost_mid_pass_reports_an_unread_uid_once_56_was_sent();
    test_the_last_progress_frame_waits_for_the_reread();

    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
