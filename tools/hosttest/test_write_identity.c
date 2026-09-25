// Host-side tests for iso15693_poller_write_identity -- the AFI / DSFID write-then-read-back-and-verify
// retry loop a clone runs so the copy advertises the same chip identity as the source.
//
// The shipped poller is compiled VERBATIM (this file includes the .c) and the SDK calls resolve to the
// fake tag, same as the other poller tests.
//
// This was the last named gap in the harness. Its OUTCOME was already covered -- test_outcome.c pins
// that a rejected identity field makes a clone Partial and is clone-only -- but the mechanics were not,
// and the mechanics are where the interesting claim lives:
//
//   iso15693_3_poller_send_frame returns Iso15693_3ErrorNone whether or not the tag applied the write.
//   A tag refusing in band answers with a well-formed, CRC-valid error frame, and the SDK has no
//   response parser for WRITE AFI / WRITE DSFID (unlike write_block, which pairs send_frame with
//   iso15693_3_write_block_response_parse). So the send tells you nothing at all, in either direction,
//   and GET SYSTEM INFO is the only thing that can.
//
// A fake that reported refusals as errors could not test any of that, so it doesn't: refuses_afi /
// refuses_dsfid make the tag swallow the write and still answer None, exactly as silicon would.

#include "fake_tag.h"

#include "../../magic/protocols/iso15693/iso15693_poller.c" // NOLINT -- deliberate, see above

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

#define CHECK_EQ(actual, expected)                                                          \
    do {                                                                                    \
        long long a_ = (long long)(actual), e_ = (long long)(expected);                     \
        if(a_ != e_) {                                                                       \
            printf(                                                                          \
                "  FAIL %s:%d  %s == %lld, expected %lld\n", __FILE__, __LINE__, #actual, a_, e_); \
            current_failed = true;                                                           \
        }                                                                                    \
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

// ---- harness ----------------------------------------------------------------------------------------

static Iso15693_3Data source;

// A clone source that reports the identity fields the caller asks for. The source's flags are what make
// write_identity attempt anything at all -- it reproduces only what the source claimed to have.
static void source_with_identity(bool want_dsfid, uint8_t dsfid, bool want_afi, uint8_t afi) {
    fake_data_init(&source, 8, 4);
    source.system_info.flags = ISO15693_3_SYSINFO_FLAG_MEMORY;
    if(want_dsfid) {
        source.system_info.flags |= ISO15693_3_SYSINFO_FLAG_DSFID;
        source.system_info.dsfid = dsfid;
    }
    if(want_afi) {
        source.system_info.flags |= ISO15693_3_SYSINFO_FLAG_AFI;
        source.system_info.afi = afi;
    }
}

// The poller owns its frame buffers for its whole life and addresses every write to the card it found
// at activation. This driver calls write_identity directly, below write_step, so it stands in for both.
static BitBuffer* driver_tx;
static BitBuffer* driver_rx;

static Iso15693Poller run_identity(void) {
    if(driver_tx == NULL) {
        driver_tx = bit_buffer_alloc(ISO15693_POLLER_BUF_SIZE);
        driver_rx = bit_buffer_alloc(ISO15693_POLLER_BUF_SIZE);
    }
    Iso15693Poller inst;
    memset(&inst, 0, sizeof(inst));
    inst.frame_tx = driver_tx;
    inst.frame_rx = driver_rx;
    memcpy(inst.address_uid, fake_tag.uid, ISO15693_3_UID_SIZE);
    inst.clone_source = &source;
    iso15693_poller_write_identity(&inst, NULL);
    return inst;
}

// ---- the cases --------------------------------------------------------------------------------------

// A source reporting neither field has nothing to reproduce, so the whole function is a no-op. Worth
// pinning because it is why the state-machine tests never exercise this path.
static void test_source_with_no_identity_sends_nothing(void) {
    begin("a source reporting neither field sends no frames");
    fake_tag_init(8, 8, 4);
    source_with_identity(false, 0, false, 0);
    const uint32_t ops_before = fake_tag.ops;

    Iso15693Poller inst = run_identity();

    CHECK_EQ(fake_tag.ops, ops_before); // not one frame went out
    CHECK_EQ(fake_tag.identity_writes_seen, 0);
    CHECK(!inst.clone_dsfid_failed);
    CHECK(!inst.clone_afi_failed);
    end();
}

// WRITE AFI and WRITE DSFID are STANDARD commands, so an unaddressed one lands on a bystander of any
// size -- and a changed AFI can drop that tag out of a selective inventory, which is a card that then
// looks absent to the reader that uses it. That gave these the strongest claim to an address of
// anything left unaddressed. Pinned as bytes because the wrong UID order does not fail loudly: it
// addresses a card that is not there, so the field simply never takes.
static void test_the_identity_frame_is_addressed(void) {
    begin("an identity write is addressed and carries the UID least significant byte first");
    fake_tag_init(8, 8, 4);
    Iso15693Poller inst = run_identity(); // for the buffers and the address it sets up
    iso15693_poller_send_identity_frame(&inst, NULL, ISO15693_MAGIC_CMD_WRITE_AFI, 0x27);

    const uint8_t expected[] = {
        0x22, // SUBCARRIER_1 | DATA_RATE_HI | T4_ADDRESSED
        0x27, // WRITE AFI
        0xE7, 0xE6, 0xE5, 0xE4, 0xE3, 0xE2, 0xE1, 0xE0, // fake_tag_init's UID, reversed
        0x27}; // the value
    CHECK_EQ(bit_buffer_get_size_bytes(inst.frame_tx), sizeof(expected));
    for(size_t i = 0; i < sizeof(expected) && i < bit_buffer_get_size_bytes(inst.frame_tx); i++) {
        CHECK_EQ(bit_buffer_get_byte(inst.frame_tx, i), expected[i]);
    }
    end();
}

// The control for that: prove the tag FILTERS on the address, so a field that lands is evidence the
// address was right rather than evidence nobody was checking.
static void test_a_wrongly_addressed_identity_write_lands_nowhere(void) {
    begin("an identity write addressed to the wrong card sets nothing");
    fake_tag_init(8, 8, 4);
    source_with_identity(true, 0x5A, true, 0xC3);

    Iso15693Poller inst;
    memset(&inst, 0, sizeof(inst));
    inst.frame_tx = driver_tx;
    inst.frame_rx = driver_rx;
    memcpy(inst.address_uid, fake_tag.uid, ISO15693_3_UID_SIZE);
    inst.address_uid[0] ^= 0x01; // one byte wrong
    inst.clone_source = &source;
    iso15693_poller_write_identity(&inst, NULL);

    CHECK(inst.clone_dsfid_failed);
    CHECK(inst.clone_afi_failed);
    CHECK_EQ(fake_tag.dsfid, 0);
    CHECK_EQ(fake_tag.afi, 0);
    end();
}

// The OPTION flag is a property of the card, not of the command, so a card that objects to a data
// block objects to these too. It is learned here rather than inherited, because write_identity runs
// BEFORE the first data block -- so on a TI clone this pass is where the 0x03 first arrives.
static void test_a_card_that_wants_the_option_flag_gets_it_here_too(void) {
    begin("a card wanting the OPTION flag gets it on the identity writes, learned from its own answer");
    fake_tag_init(8, 8, 4);
    fake_tag.requires_option = true;
    fake_tag.writes_are_unacknowledged = true; // the other half of the same card
    source_with_identity(true, 0x5A, true, 0xC3);

    Iso15693Poller inst = run_identity();

    CHECK(inst.write_option); // set by the card's own 0x03, with no data block written yet
    CHECK(!inst.clone_dsfid_failed);
    CHECK(!inst.clone_afi_failed);
    CHECK_EQ(fake_tag.dsfid, 0x5A);
    CHECK_EQ(fake_tag.afi, 0xC3);
    end();
}

// The happy path: the tag takes both writes and reports them back.
static void test_both_fields_written_and_verified(void) {
    begin("a tag that takes both writes reports neither as failed");
    fake_tag_init(8, 8, 4);
    source_with_identity(true, 0x5A, true, 0xC3);

    Iso15693Poller inst = run_identity();

    CHECK(!inst.clone_dsfid_failed);
    CHECK(!inst.clone_afi_failed);
    CHECK_EQ(fake_tag.dsfid, 0x5A);
    CHECK_EQ(fake_tag.afi, 0xC3);
    CHECK_EQ(fake_tag.identity_writes_seen, 2); // one attempt each, no retries needed
    end();
}

// THE CASE THE READ-BACK EXISTS FOR. The tag refuses in band, so every send returns None and nothing
// about the send betrays it. Only GET SYSTEM INFO can, and it must -- otherwise the clone reports a
// faithful copy of an identity it never wrote.
static void test_in_band_refusal_is_caught_by_the_readback(void) {
    begin("an in-band refusal is invisible to the send and caught by the read-back");
    fake_tag_init(8, 8, 4);
    fake_tag.refuses_dsfid = true;
    fake_tag.refuses_afi = true;
    source_with_identity(true, 0x11, true, 0x22);

    Iso15693Poller inst = run_identity();

    CHECK(inst.clone_dsfid_failed);
    CHECK(inst.clone_afi_failed);
    // Every attempt was spent, since nothing ever verified.
    CHECK_EQ(fake_tag.identity_writes_seen, 2 * ISO15693_POLLER_WRITE_ATTEMPTS);
    CHECK(strstr(fake_log_text(), "DSFID did not read back as written") != NULL);
    CHECK(strstr(fake_log_text(), "AFI did not read back as written") != NULL);
    end();
}

// The converse, named in the poller's own comment: a tag that APPLIES the write without answering would
// look like a failure if the send's return were trusted. Here the fake accepts the write, so the
// read-back passes -- the point being that the verdict comes from the read, not the send.
static void test_verdict_comes_from_the_read_not_the_send(void) {
    begin("a tag that applies the write is a pass, however the send looked");
    fake_tag_init(8, 8, 4);
    source_with_identity(true, 0x77, false, 0);

    Iso15693Poller inst = run_identity();

    CHECK(!inst.clone_dsfid_failed);
    CHECK_EQ(fake_tag.dsfid, 0x77);
    // AFI was never wanted, so it is not attempted and not held against the clone.
    CHECK(!inst.clone_afi_failed);
    CHECK_EQ(fake_tag.identity_writes_seen, 1);
    end();
}

// A transient refusal is exactly what the retry loop is for. Two failures then success must NOT be
// reported as a rejection -- the same reasoning as the block loop's retries.
static void test_transient_refusal_is_ridden_out(void) {
    begin("a transient refusal is retried, not reported");
    fake_tag_init(8, 8, 4);
    fake_tag.identity_writes_refused = 1; // the first write is swallowed, later ones land
    source_with_identity(true, 0x3C, false, 0);

    Iso15693Poller inst = run_identity();

    CHECK(!inst.clone_dsfid_failed);
    CHECK_EQ(fake_tag.dsfid, 0x3C);
    CHECK(fake_tag.identity_writes_seen > 1); // it really did take more than one go
    end();
}

// Holding the right value is not enough: the target must ADVERTISE the field too, or the copy no longer
// reports the identity the source reported. This is the clause that makes the verify strict, and it is
// the one a reader is most likely to think redundant.
static void test_value_without_the_flag_is_a_failure(void) {
    begin("the right value without the advertised flag still fails");
    fake_tag_init(8, 8, 4);
    source_with_identity(true, 0x42, false, 0);
    // The tag already holds exactly the value the source wants...
    fake_tag.dsfid = 0x42;
    // ...but refuses the write, so it never starts advertising the field.
    fake_tag.refuses_dsfid = true;
    fake_tag.advertises_dsfid = false;

    Iso15693Poller inst = run_identity();

    CHECK(inst.clone_dsfid_failed);
    end();
}

// If the verify cannot reach an answer at all, the fields cannot be claimed as written. Failing closed
// is the only safe direction: the alternative is reporting a faithful clone on no evidence.
static void test_unreachable_verify_fails_closed(void) {
    begin("a verify that never gets an answer fails closed");
    fake_tag_init(8, 8, 4);
    fake_tag.sysinfo_fails = true;
    source_with_identity(true, 0x01, true, 0x02);

    Iso15693Poller inst = run_identity();

    CHECK(inst.clone_dsfid_failed);
    CHECK(inst.clone_afi_failed);
    end();
}

// Only the fields the source claimed are attempted. A tag that would refuse AFI is irrelevant to a
// source that never reported one, and must not be held against the clone.
static void test_only_the_requested_field_is_attempted(void) {
    begin("a field the source never reported is neither written nor failed");
    fake_tag_init(8, 8, 4);
    fake_tag.refuses_afi = true; // would fail, if it were ever asked
    source_with_identity(true, 0x09, false, 0);

    Iso15693Poller inst = run_identity();

    CHECK(!inst.clone_dsfid_failed);
    CHECK(!inst.clone_afi_failed); // never attempted, so never a failure
    CHECK(!fake_tag.advertises_afi);
    end();
}

// One field failing must not drag the other down with it. They are reported separately because the
// screens name them separately.
static void test_one_field_failing_does_not_taint_the_other(void) {
    begin("one field failing leaves the other's verdict alone");
    fake_tag_init(8, 8, 4);
    fake_tag.refuses_afi = true;
    source_with_identity(true, 0x5B, true, 0x6C);

    Iso15693Poller inst = run_identity();

    CHECK(!inst.clone_dsfid_failed); // took the write
    CHECK(inst.clone_afi_failed); // refused it
    CHECK_EQ(fake_tag.dsfid, 0x5B);
    end();
}

// The retry loop must stop as soon as both fields verify, rather than burning its full budget. The cost
// matters: this runs inside the same pass the wall-clock bound governs.
static void test_loop_stops_once_both_verify(void) {
    begin("the retry loop stops as soon as both fields verify");
    fake_tag_init(8, 8, 4);
    source_with_identity(true, 0x10, true, 0x20);

    run_identity();

    // Two writes total, not two per attempt: the loop's condition is "not yet both ok".
    CHECK_EQ(fake_tag.identity_writes_seen, 2);
    CHECK(ISO15693_POLLER_WRITE_ATTEMPTS > 1); // otherwise this test proves nothing
    end();
}

int main(void) {
    printf("iso15693 identity write (AFI / DSFID)\n");
    test_source_with_no_identity_sends_nothing();
    test_both_fields_written_and_verified();
    test_in_band_refusal_is_caught_by_the_readback();
    test_verdict_comes_from_the_read_not_the_send();
    test_transient_refusal_is_ridden_out();
    test_value_without_the_flag_is_a_failure();
    test_unreachable_verify_fails_closed();
    test_only_the_requested_field_is_attempted();
    test_one_field_failing_does_not_taint_the_other();
    test_loop_stops_once_both_verify();
    test_the_identity_frame_is_addressed();
    test_a_wrongly_addressed_identity_write_lands_nowhere();
    test_a_card_that_wants_the_option_flag_gets_it_here_too();
    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}
