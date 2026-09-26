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
    iso15693_poller_build_write_frame(
        tx, ISO15693_POLLER_WRITE_FLAGS, uid, 0x08, data, sizeof(data));

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

// ---- the OPTION flag ------------------------------------------------------------------------------

// TI Tag-it HF-I Plus answers an addressed write with the OPTION bit clear as error 0x03 -- the tag
// naming the bit -- and accepts the identical frame with it set. The app cannot know that in advance
// and deliberately does not guess from the UID, so the first write of the run is the question and the
// refusal is the answer.
//
// Counted as a DIFFERENCE against the same card without the requirement. A sweep of a 64-block card
// already spends three refused attempts on each of the eight phantom blocks above its top, and pinning
// that total here would make this test fail whenever the sweep's tail handling changes.
//
// Three runs, because two would not be enough. The plain card is the baseline. The card that wants the
// flag must cost exactly ONE more refusal than it -- recovering from block 1 onward would leave block 0
// unwiped and still report a clean sweep. And the same card with the flag already set must cost the
// baseline again, which is what shows the fake is enforcing the flag rather than ignoring it.
static uint32_t failed_write_attempts(void) {
    return fake_tag.writes_attempted - fake_tag.writes_accepted;
}

static void test_a_card_that_wants_the_option_flag_is_written_anyway(void) {
    begin("a card that refuses without the OPTION flag costs one refusal and is written");
    Iso15693Poller inst;
    bool card_lost = false;

    fake_tag_init(64, 64, 4);
    driver_init(&inst);
    CHECK_EQ(iso15693_poller_wipe_blocks(&inst, NULL, &card_lost), 64);
    const uint32_t baseline = failed_write_attempts();

    fake_tag_init(64, 64, 4);
    fake_tag.requires_option = true;
    driver_init(&inst);
    CHECK_EQ(iso15693_poller_wipe_blocks(&inst, NULL, &card_lost), 64);
    CHECK(inst.write_option); // the tag's own answer turned it on
    CHECK_EQ(failed_write_attempts(), baseline + 1);
    CHECK_EQ(inst.clone_failed_count, 0);

    // The retry that recovers that block was going to happen anyway, so the flag costs no frames the
    // write budget had not already allowed for.
    fake_tag_init(64, 64, 4);
    fake_tag.requires_option = true;
    driver_init(&inst);
    inst.write_option = true;
    CHECK_EQ(iso15693_poller_wipe_blocks(&inst, NULL, &card_lost), 64);
    CHECK_EQ(failed_write_attempts(), baseline);
    end();
}

// A card that does NOT want the flag must never be given it. The four chips this was measured against
// were measured at 0x22, and sending them a frame they were not measured with would be a change with
// no evidence behind it.
static void test_a_card_that_never_complains_never_gets_the_flag(void) {
    begin("a card that takes the plain frame is never sent the OPTION flag");
    fake_tag_init(64, 64, 4);

    Iso15693Poller inst;
    driver_init(&inst);
    bool card_lost = false;
    iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);

    CHECK(!inst.write_option);
    end();
}

// The refusal that sets the flag is a specific one. A block that is merely unavailable or locked says
// so with its own code, and reading either as "wants the option" would turn the flag on for a card
// that never asked -- on the strength of a block that does not exist.
static void test_only_the_option_refusal_sets_the_flag(void) {
    begin("only error 0x03 turns the flag on, not any other refusal");
    const uint8_t wants_option[] = {0x01, ISO15693_3_RESP_ERROR_OPTION};
    const uint8_t unavailable[] = {0x01, ISO15693_3_RESP_ERROR_BLOCK_UNAVAILABLE};
    const uint8_t locked[] = {0x01, ISO15693_3_RESP_ERROR_BLOCK_LOCKED};
    const uint8_t took[] = {0x00};

    BitBuffer* rx = bit_buffer_alloc(ISO15693_POLLER_BUF_SIZE);
    struct {
        const uint8_t* bytes;
        size_t len;
        bool expected;
    } cases[] = {
        {wants_option, sizeof(wants_option), true},
        {unavailable, sizeof(unavailable), false},
        {locked, sizeof(locked), false},
        {took, sizeof(took), false},
    };
    for(size_t c = 0; c < COUNT_OF(cases); c++) {
        bit_buffer_reset(rx);
        for(size_t i = 0; i < cases[c].len; i++) {
            bit_buffer_append_byte(rx, cases[c].bytes[i]);
        }
        CHECK_EQ(iso15693_poller_response_wants_option(rx), cases[c].expected);
    }
    bit_buffer_free(rx);
    end();
}

// ---- a card that never acknowledges a write ---------------------------------------------------

// The whole TI Tag-it failure, end to end. With OPTION set the card owes its answer only after a
// standalone EOF, which this SDK cannot send, so the block is programmed and nothing is said. Before
// the rescue this reported "Wipe failed / No blocks could be cleared" over a card that had in fact
// been entirely zeroed -- measured on `white-coin`, whose block 8 went from AA BB CC DD to zeros
// across a wipe the app called a total failure.
//
// Asserted on the CARD as well as on the count, because the count alone cannot tell "cleared and
// reported" from "reported without clearing", and this bug was the second of those in reverse.
static void test_a_card_that_never_acknowledges_is_still_wiped(void) {
    begin("a card that writes without acknowledging is wiped, and says so");
    fake_tag_init(64, 64, 4);
    fake_tag.requires_option = true;
    fake_tag.writes_are_unacknowledged = true;
    fake_tag_fill(0, 63, FAKE_MARKER);

    Iso15693Poller inst;
    driver_init(&inst);
    bool card_lost = false;
    const uint16_t wiped = iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);

    CHECK_EQ(wiped, 64);
    CHECK_EQ(inst.clone_failed_count, 0);
    CHECK(!card_lost);
    for(uint16_t b = 0; b < 64; b++) {
        CHECK(iso15693_poller_block_is_empty(fake_tag.content[b], 4));
    }
    end();
}

// The rescue is scoped to cards that ASKED for the OPTION flag, and this is that scoping. A card that
// simply stops answering looks identical at the radio layer, and reading its memory back would let a
// removed card's leftover contents pass as writes that landed. The retries are the right answer
// there, and a failure is the right verdict.
static void test_silence_alone_is_still_a_failure(void) {
    begin("silence from a card that never asked for the flag is still a failure");
    fake_tag_init(64, 64, 4);
    fake_tag.writes_are_unacknowledged = true; // but NOT requires_option
    fake_tag_fill(0, 63, FAKE_MARKER);

    Iso15693Poller inst;
    driver_init(&inst);
    bool card_lost = false;
    const uint16_t wiped = iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);

    CHECK(!inst.write_option);
    CHECK_EQ(wiped, 0);
    end();
}

// An in-band refusal is never second-guessed, whatever else is set. The dangerous shape is a refused
// block that ALREADY HOLDS what is being written: a locked block that is already empty, under a wipe.
// Read it back and it matches, so a rescue keyed on the content alone would call the refusal a
// success -- and a card that is write-protected and already blank would report "Wiped 64/64" instead
// of the truth, which is that it accepted nothing and cannot be wiped. The card answered; its answer
// stands, and the read is never asked.
static void test_an_answered_refusal_is_not_overruled_by_a_read(void) {
    begin("an in-band refusal stands even when the block already holds what was written");
    fake_tag_init(64, 64, 4);
    fake_tag.requires_option = true;
    fake_tag.writes_are_unacknowledged = true;
    fake_tag_fill(0, 63, FAKE_MARKER);
    fake_tag_set_range(30, 30, FakeBlockLocked); // answers a read, refuses the write in band
    fake_tag_fill(30, 30, 0x00); // ...and already holds exactly what the wipe is about to write

    Iso15693Poller inst;
    driver_init(&inst);
    bool card_lost = false;
    const uint16_t wiped = iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);

    CHECK_EQ(wiped, 63); // the refusal is not counted as a clear, however empty the block looks
    CHECK_EQ(inst.clone_failed_count, 0); // nor as a failure -- it is empty, so nothing is uncleared
    end();
}

// And the read-back is COMPARED, not merely performed. A block that answers reads, discards writes and
// says nothing about it is indistinguishable from one that took the write -- until someone looks at
// what it holds. Without the comparison every such block would be reported as written, which on a
// wipe means telling the user a card is clear while it still carries the previous card's data.
static void test_the_read_back_is_compared_not_just_attempted(void) {
    begin("a block that silently discards its write is not rescued by the read-back");
    fake_tag_init(64, 64, 4);
    fake_tag.requires_option = true;
    fake_tag.writes_are_unacknowledged = true;
    fake_tag_fill(0, 63, FAKE_MARKER);
    fake_tag_set_range(30, 30, FakeBlockSilentlyRefuses);

    Iso15693Poller inst;
    driver_init(&inst);
    bool card_lost = false;
    const uint16_t wiped = iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);

    CHECK_EQ(wiped, 63);
    CHECK_EQ(inst.clone_failed_count, 1); // it answers a read and still holds data -> a real failure
    CHECK(!iso15693_poller_block_is_empty(fake_tag.content[30], 4));
    end();
}

// The re-address takes its new address from an inventory, and that inventory is the SDK's 1-slot
// unaddressed one -- so with a second tag in the field it can answer for the bystander (#251).
// Re-addressing to a stranger would point every later frame at the wrong card, on a run that is
// midway through rewriting this one's identity.
//
// What makes the answer checkable is that we know what we wrote: block 56 carries uid[7..4] and 57
// carries uid[3..0], so there are exactly two honest replies -- the UID unchanged, or the UID our own
// write implies. A third value did not come from us and is refused.
static void test_a_uid_our_write_does_not_account_for_is_refused(void) {
    begin("an inventory answer the write does not imply is not taken as the new address");
    fake_tag_init(64, 64, 4);
    fake_tag.is_gen1_magic = true; // so zeroing 56 really does move this card's UID
    uint8_t before[ISO15693_3_UID_SIZE];
    memcpy(before, fake_tag.uid, sizeof(before));

    // A second card wins the slot, and its UID is neither the old one nor the one our write implies.
    fake_tag.bystander_answers_inventory = true;
    memset(fake_tag.bystander_uid, 0x77, sizeof(fake_tag.bystander_uid));

    Iso15693Poller inst;
    driver_init(&inst);
    bool card_lost = false;
    iso15693_poller_wipe_blocks(&inst, NULL, &card_lost);

    CHECK(memcmp(inst.address_uid, before, ISO15693_3_UID_SIZE) == 0); // held, not taken
    CHECK(!inst.uid_moved_by_write); // and nothing downstream is told the card moved
    end();
}

// ---- the gen1 backdoor sequence -------------------------------------------------------------------

// The magic sequence is four ordinary WRITE BLOCKs at four unusual addresses, and this is what it now
// puts on the wire. Byte for byte against the frame measured on `lri2k-keychain`, which answered UID
// E0 02 22 24 50 00 83 03 and returned an in-band refusal (01 10, block unavailable) to
// 22 21 03 83 00 50 24 22 02 E0 3E 00 00 00 00 while a UID one byte wrong got silence.
//
// Pinned as bytes rather than as behaviour because that refusal is what proves the frame arrived: the
// card address-matched and PARSED a write at block 62 and objected to the block, not to the frame.
static void test_the_gen1_backdoor_frame_is_addressed(void) {
    begin("a gen1 backdoor frame carries the card's UID, least significant byte first");
    const uint8_t uid[ISO15693_3_UID_SIZE] = {0xE0, 0x02, 0x22, 0x24, 0x50, 0x00, 0x83, 0x03};
    const uint8_t unlock[ISO15693_MAGIC_REGISTER_SIZE] = {0x00, 0x00, 0x00, 0x00};
    const uint8_t expected[] = {
        0x22, // SUBCARRIER_1 | DATA_RATE_HI | T4_ADDRESSED -- not the old unaddressed 0x02
        0x21, // WRITE BLOCK, the standard command: the address is what makes this magic
        0x03, 0x83, 0x00, 0x50, 0x24, 0x22, 0x02, 0xE0, // the UID, reversed
        0x3E, // unlock
        0x00, 0x00, 0x00, 0x00};

    fake_tag_init(28, 28, 4);
    Iso15693Poller inst;
    driver_init(&inst);
    memcpy(inst.address_uid, uid, ISO15693_3_UID_SIZE);
    iso15693_poller_send_gen1_frame(&inst, NULL, ISO15693_MAGIC_BLK_UNLOCK, unlock);

    CHECK_EQ(bit_buffer_get_size_bytes(inst.frame_tx), sizeof(expected));
    for(size_t i = 0; i < sizeof(expected) && i < bit_buffer_get_size_bytes(inst.frame_tx); i++) {
        CHECK_EQ(bit_buffer_get_byte(inst.frame_tx, i), expected[i]);
    }
    end();
}

// THE SEAM ADDRESSING THIS SEQUENCE CREATES, and the one thing it costs. Block 56 carries uid[7..4]
// and takes effect immediately -- no power-cycle, measured on NXP ICODE SLIX and ST LRi2K -- so by the
// time block 57 goes out the card has already stopped answering to the address the frame before it
// used. Without the re-address in the middle, 57 meets silence and the card is left wearing half the
// target UID and half its own. The unaddressed form had no such seam.
//
// A 28-BLOCK CARD deliberately, which is what the gen1 cards on the bench actually are: the four
// registers sit outside its memory map and answer no read ever, so nothing here can be a data write
// that happens to land. Both halves have to arrive as register writes or the UID does not complete.
static void test_the_gen1_sequence_readdresses_between_the_two_halves(void) {
    begin("the gen1 sequence re-addresses after block 56, so block 57 still reaches the card");
    fake_tag_init(28, 28, 4);
    fake_tag.is_gen1_magic = true;

    const uint8_t target[ISO15693_3_UID_SIZE] = {0xE0, 0x04, 0x01, 0x50, 0x11, 0x22, 0x33, 0x44};
    uint8_t half_moved[ISO15693_3_UID_SIZE];
    memcpy(half_moved, fake_tag.uid, sizeof(half_moved));
    memcpy(&half_moved[4], &target[4], 4); // what block 56 alone implies

    Iso15693Poller inst;
    driver_init(&inst);
    iso15693_poller_send_backdoor_uid_gen1(&inst, NULL, target);

    // Both halves landed, which is only possible if 57 was addressed to the moved UID.
    CHECK(memcmp(fake_tag.uid, target, ISO15693_3_UID_SIZE) == 0);
    // Re-addressed ONCE, after 56 -- not again after 57, which nothing asks for.
    CHECK(memcmp(inst.address_uid, half_moved, ISO15693_3_UID_SIZE) == 0);
    CHECK(inst.uid_moved_by_write);
    // Only the two UID registers took anything. unlock and commit are refused in band -- `0x10` on the
    // ST LRi2K, `0x0F` on both NXP parts, never once accepted on any card here -- and the sequence
    // carries on regardless, which is why its per-frame results are ignored.
    CHECK_EQ(fake_tag.writes_accepted, 2);
    end();
}

// WHAT ADDRESSING THE SEQUENCE IS FOR (#251). These four frames are plain WRITE BLOCKs at 56/57/62/63,
// which on any tag that big is ordinary user data, and they go out behind an opt-in whose warning is
// about the card in the user's hand. Unaddressed, a second tag in the field takes them too -- and on
// ISO15693 a bystander need only be in a wallet or a badge holder, not on the antenna.
//
// The control that could have failed: the same sequence, one byte wrong in the address, against a card
// that is genuinely gen1 and answering. Before this change it would have moved that card's UID.
static void test_the_gen1_sequence_addressed_elsewhere_moves_nothing(void) {
    begin("a gen1 sequence addressed to another card leaves this one's UID alone");
    fake_tag_init(28, 28, 4);
    fake_tag.is_gen1_magic = true;
    uint8_t before[ISO15693_3_UID_SIZE];
    memcpy(before, fake_tag.uid, sizeof(before));

    const uint8_t target[ISO15693_3_UID_SIZE] = {0xE0, 0x04, 0x01, 0x50, 0x11, 0x22, 0x33, 0x44};

    Iso15693Poller inst;
    driver_init(&inst);
    inst.address_uid[0] ^= 0x01; // one byte wrong

    iso15693_poller_send_backdoor_uid_gen1(&inst, NULL, target);

    CHECK(memcmp(fake_tag.uid, before, ISO15693_3_UID_SIZE) == 0);
    CHECK_EQ(fake_tag.writes_accepted, 0);
    CHECK(!inst.uid_moved_by_write);
    end();
}

int main(void) {
    printf("iso15693 addressed write\n");
    test_frame_layout();
    test_response_decode();
    test_wrong_address_is_answered_by_nothing();
    test_the_sweep_readdresses_when_the_uid_moves_under_it();
    test_only_the_uid_registers_cost_an_inventory();
    test_a_card_that_wants_the_option_flag_is_written_anyway();
    test_a_card_that_never_complains_never_gets_the_flag();
    test_only_the_option_refusal_sets_the_flag();
    test_a_uid_our_write_does_not_account_for_is_refused();
    test_a_card_that_never_acknowledges_is_still_wiped();
    test_silence_alone_is_still_a_failure();
    test_an_answered_refusal_is_not_overruled_by_a_read();
    test_the_read_back_is_compared_not_just_attempted();
    test_the_gen1_backdoor_frame_is_addressed();
    test_the_gen1_sequence_readdresses_between_the_two_halves();
    test_the_gen1_sequence_addressed_elsewhere_moves_nothing();
    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}
