// Host-side tests for the ADDRESSED ISO15693 write path.
//
// The shipped poller is compiled VERBATIM -- this file includes the .c so it can reach the file-statics,
// and the SDK calls resolve to the fake tag. No production code is modified or wrapped.
//
// Why this path has its own file: the app builds the WRITE BLOCK frame and parses its response itself,
// because the SDK hardcodes unaddressed flags and keeps its response parser internal to lib/nfc. That
// makes the wire format and the error decode ours to get right, and neither is visible in the counters
// the sweep and clone tests assert on -- a frame addressed to the wrong card and a card that refuses
// everything look identical from there.

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

// The poller allocates these once for its whole life and addresses every write to the card it found at
// activation. These drivers call the pass functions directly, below write_step, so they stand in for
// both: one buffer pair for the run, and the address Iso15693WriteStateStart would have taken.
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

// ---- the wire format ------------------------------------------------------------------------------

// Byte for byte against the frame measured on hardware. `slix-1k-50mm` answered UID
// E0 04 01 50 20 26 08 63, and the frame it accepted a write from was
// 22 21 63 08 26 20 50 01 04 E0 08 11 22 33 44 -- so the UID is REVERSED on the wire and the 0xE0 that
// every screen prints first goes out last. The wrong order does not fail loudly: it addresses a card
// that is not in the field, so every write meets silence and the card reads as one that refuses
// everything. That is what makes this worth pinning as bytes rather than as behaviour.
static void test_frame_layout(void) {
    begin("the write frame is addressed and carries the UID least significant byte first");
    const uint8_t uid[ISO15693_3_UID_SIZE] = {0xE0, 0x04, 0x01, 0x50, 0x20, 0x26, 0x08, 0x63};
    const uint8_t data[4] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t expected[] = {
        0x22, // SUBCARRIER_1 | DATA_RATE_HI | T4_ADDRESSED
        0x21, // WRITE BLOCK
        0x63, 0x08, 0x26, 0x20, 0x50, 0x01, 0x04, 0xE0, // the UID, reversed
        0x08, // block
        0x11, 0x22, 0x33, 0x44};

    BitBuffer* tx = bit_buffer_alloc(ISO15693_POLLER_BUF_SIZE);
    iso15693_poller_build_write_frame(tx, uid, 0x08, data, sizeof(data));

    CHECK_EQ(bit_buffer_get_size_bytes(tx), sizeof(expected));
    for(size_t i = 0; i < sizeof(expected) && i < bit_buffer_get_size_bytes(tx); i++) {
        CHECK_EQ(bit_buffer_get_byte(tx, i), expected[i]);
    }
    bit_buffer_free(tx);
    end();
}

// ---- the response decode --------------------------------------------------------------------------

static Iso15693_3Error parse_bytes(const uint8_t* bytes, size_t len) {
    BitBuffer* rx = bit_buffer_alloc(ISO15693_POLLER_BUF_SIZE);
    for(size_t i = 0; i < len; i++) {
        bit_buffer_append_byte(rx, bytes[i]);
    }
    const Iso15693_3Error error = iso15693_poller_parse_write_response(rx);
    bit_buffer_free(rx);
    return error;
}

// The whole reason this function exists: a tag that refuses IN BAND answers with a well-formed,
// CRC-valid error frame, so the radio layer reports a successful exchange and the refusal is invisible
// without a look at the bytes. Every case here is one the SDK's own parser distinguishes; matching it
// is what keeps the two halves of the app saying the same thing about the same tag.
static void test_response_decode(void) {
    begin("a write that took, and every shape of refusal, decode as the SDK decodes them");
    const uint8_t ok[] = {0x00};
    const uint8_t unavailable[] = {0x01, 0x10};
    const uint8_t locked[] = {0x01, 0x12};
    const uint8_t unsupported[] = {0x01, 0x01};
    const uint8_t vendor[] = {0x01, 0xA5};
    const uint8_t truncated[] = {0x01};
    const uint8_t overlong[] = {0x00, 0x00};

    CHECK_EQ(parse_bytes(ok, sizeof(ok)), Iso15693_3ErrorNone);
    CHECK_EQ(parse_bytes(unavailable, sizeof(unavailable)), Iso15693_3ErrorInternal);
    CHECK_EQ(parse_bytes(locked, sizeof(locked)), Iso15693_3ErrorInternal);
    CHECK_EQ(parse_bytes(unsupported, sizeof(unsupported)), Iso15693_3ErrorNotSupported);
    CHECK_EQ(parse_bytes(vendor, sizeof(vendor)), Iso15693_3ErrorCustom);
    // The error flag is set and there is no code to read, which is not a write that took.
    CHECK_EQ(parse_bytes(truncated, sizeof(truncated)), Iso15693_3ErrorUnexpectedResponse);
    // A WRITE BLOCK answer is the flags byte and nothing else; anything longer is not this response.
    CHECK_EQ(parse_bytes(overlong, sizeof(overlong)), Iso15693_3ErrorUnexpectedResponse);
    CHECK_EQ(parse_bytes(NULL, 0), Iso15693_3ErrorBufferEmpty);
    end();
}

// ---- addressing, against the tag --------------------------------------------------------------

// The control for everything below: prove the fake tag actually FILTERS on the address, so that a pass
// which clears its blocks is evidence the address was right rather than evidence nobody was checking.
// Measured on four of the five chips -- a UID one byte wrong gets no answer at all.
//
// Reads are unaddressed here, as they are in the app, so every block still answers one. That is why the
// signature of a wrong address is not "the card vanished" but "every block refused a write and still
// holds its data" -- 28 failures, nothing wiped.
//
// A 28-block card, deliberately: a 64-block one reaches block 56, and the re-address there takes the
// right UID off the card and repairs the address for everything above it. Real, and no use as a
// control -- it would leave this test asserting a partial clear that a tag ignoring the flag entirely
// could also produce.
static void test_wrong_address_is_answered_by_nothing(void) {
    begin("a write addressed to the wrong card clears nothing");
    fake_tag_init(28, 28, 4);

    Iso15693Poller inst;
    driver_init(&inst);
    inst.address_uid[0] ^= 0x01; // one byte wrong
    bool card_lost = false;
    const uint16_t wiped = iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);

    CHECK_EQ(wiped, 0);
    CHECK_EQ(inst.clone_failed_count, 28);
    CHECK(!card_lost); // the card is right there -- it is answering reads and inventories
    end();
}

// The measured hazard, and the reason the wipe cannot take its address once and keep it: blocks 56 and
// 57 ARE the gen1 UID registers, so the sweep's own zero-write moves the card's identity -- immediately,
// with no power-cycle, measured on NXP ICODE SLIX and ST LRi2K.
//
// Without the re-address the sweep would clear 0..56, lose the card's ear at 57, and report a card
// shorter than the one in the field -- on exactly the armed-gen1 path this whole check exists for.
static void test_the_sweep_readdresses_when_the_uid_moves_under_it(void) {
    begin("a sweep that moves the UID at 56/57 keeps writing the blocks above them");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = true; // so zeroing 56/57 rewrites the UID, as an armed card does

    Iso15693Poller inst;
    driver_init(&inst);
    bool card_lost = false;
    const uint16_t wiped = iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);

    // The hazard fired: the card is answering to a different UID than the one the sweep started with.
    const uint8_t moved[ISO15693_3_UID_SIZE] = {0};
    CHECK(memcmp(fake_tag.uid, moved, ISO15693_3_UID_SIZE) == 0);
    CHECK(memcmp(inst.address_uid, moved, ISO15693_3_UID_SIZE) == 0);
    // And the sweep finished anyway.
    CHECK_EQ(wiped, 64);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK(!card_lost);
    end();
}

// 62 and 63 are backdoor registers too, but they carry no UID, so a write there costs no inventory.
// Stated as a test because "re-address after the backdoor blocks" is the easy misreading of the rule,
// and it would put two more frames into every sweep for nothing.
static void test_only_the_uid_registers_cost_an_inventory(void) {
    begin("only blocks 56 and 57 trigger a re-address");
    CHECK(iso15693_poller_is_uid_block(56));
    CHECK(iso15693_poller_is_uid_block(57));
    CHECK(!iso15693_poller_is_uid_block(62));
    CHECK(!iso15693_poller_is_uid_block(63));
    CHECK(!iso15693_poller_is_uid_block(0));

    // Counted as a DIFFERENCE rather than as an absolute. Both sweeps take one inventory of their own,
    // the card-present check at the absent run past the claim, and pinning that number here would make
    // this test fail whenever the sweep changes for reasons that have nothing to do with addressing.
    fake_tag_init(28, 28, 4); // stops short of 56, so no UID register is written
    Iso15693Poller inst;
    driver_init(&inst);
    bool card_lost = false;
    iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);
    const uint32_t without_uid_blocks = fake_tag.inventories;

    fake_tag_init(64, 64, 4); // reaches 56, 57, 62 and 63 -- and resets the counters
    driver_init(&inst);
    iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);
    const uint32_t with_uid_blocks = fake_tag.inventories;

    CHECK_EQ(with_uid_blocks - without_uid_blocks, 2); // 56 and 57, not 62 and 63
    end();
}

int main(void) {
    printf("iso15693 addressed write\n");
    test_frame_layout();
    test_response_decode();
    test_wrong_address_is_answered_by_nothing();
    test_the_sweep_readdresses_when_the_uid_moves_under_it();
    test_only_the_uid_registers_cost_an_inventory();
    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}
