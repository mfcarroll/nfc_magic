// A programmable ISO15693 tag for host-side tests.
//
// The point of this file is to stage the card behaviours no card we own can produce: a block that
// refuses a write while still answering a read, a stretch of memory that goes dead mid-sweep, a card
// lifted at a chosen moment, and a sweep slow enough to hit the wall-clock bound.
//
// Everything is deterministic. There is no wall clock and no randomness: `fake_tick` advances by a
// fixed cost per radio operation, so "10 seconds elapsed" means "the sweep did N operations".
#pragma once

#include <furi.h>
#include <lib/nfc/protocols/iso15693_3/iso15693_3.h>

typedef enum {
    // Accepts writes, answers reads. Ordinary memory.
    FakeBlockWritable,
    // Refuses the write with Iso15693_3ErrorInternal, still answers reads. Write-protected memory --
    // the case that discriminates "the card ends here" from "this block won't clear", and the one no
    // card on this PR can be made to produce.
    FakeBlockLocked,
    // Answers neither a write nor a read. Past physical capacity, or a phantom block on a card that
    // advertises more than it holds.
    FakeBlockAbsent,
    // Answers reads, discards writes, and says NOTHING about it -- no error frame, no acknowledgement.
    // The case that makes a read-back worth comparing rather than merely performing: on a card whose
    // writes are not acknowledged anyway, this is indistinguishable from a write that landed until
    // someone looks at what the block actually holds.
    FakeBlockSilentlyRefuses,
} FakeBlockKind;

typedef struct {
    // What the card ADVERTISES via Get System Info. Deliberately independent of which blocks are
    // actually present -- that gap is the whole subject of the sweep.
    uint16_t advertised;
    uint8_t block_size;
    uint8_t uid[ISO15693_3_UID_SIZE];

    FakeBlockKind kind[FAKE_MAX_BLOCKS];
    uint8_t content[FAKE_MAX_BLOCKS][FAKE_MAX_BLOCK_SIZE];

    // Radio operations (write/read/inventory) before the card leaves the field. 0 means it never does.
    // After this many, every operation fails and inventory reports the card gone.
    uint32_t ops_until_lifted;

    // Ticks (== ms) charged per radio operation. Bench figures put a refused block in the 40-70ms
    // range including its retries and read-back; a single op is a fraction of that. Set high to drive
    // the sweep into ISO15693_POLLER_PASS_MAX_MS.
    uint32_t tick_cost_per_op;

    // The fake holds a gen1 UID write until the next power-cycle. That is DELIBERATELY STRICTER than
    // the hardware, and no longer models it: measured on one card of each of three chips -- ST LRi2K,
    // NXP ICODE SLIX-S, NXP ICODE SLIX -- an inventory in the SAME field session as the write already
    // returns the new UID, so there is no latch to model. The pessimistic version is kept because it
    // makes a UID verify that skips its reset fail here rather than pass by luck.
    // The poller's reason for the reset is a clean re-activation, not a latch; see
    // ISO15693_MAGIC_BLK_UNLOCK in iso15693_poller.c.
    // Set by fake_tag_arm_gen1_uid(); applied by fake_tag_power_cycle().
    bool gen1_uid_pending;
    uint8_t gen1_pending_uid[ISO15693_3_UID_SIZE];

    // gen2 magic: the backdoor UID write takes effect immediately, no power-cycle needed. A tag that is
    // not magic at all leaves both of these false and keeps its UID whatever is written.
    bool is_gen2_magic;
    bool is_gen1_magic;

    // AFI / DSFID, the two identity fields a clone reproduces with WRITE AFI / WRITE DSFID. Modelled
    // separately from "does the tag ADVERTISE them", because iso15693_poller_write_identity requires
    // both -- a copy holding the right AFI while no longer reporting one is not a faithful clone, and
    // that distinction is the only thing standing between the verify and a false pass.
    uint8_t afi;
    uint8_t dsfid;
    bool advertises_afi;
    bool advertises_dsfid;

    // A tag that refuses the write IN BAND: it answers with a well-formed error frame, so
    // iso15693_3_poller_send_frame returns None and the refusal is invisible to the caller. This is the
    // exact failure the read-back exists to catch, and it cannot be produced by returning an error.
    bool refuses_afi;
    bool refuses_dsfid;

    // Refuse the first N identity writes, then start accepting -- a transient, which the retry loop is
    // supposed to ride out. 0 means never refuse for this reason.
    uint32_t identity_writes_refused;
    uint32_t identity_writes_seen;

    // Get System Info fails outright, so the verify never reaches an answer at all.
    bool sysinfo_fails;
    // Get System Info ANSWERS, but without the MEMORY flag -- so the card states no block count at
    // all. Distinct from sysinfo_fails, and the distinction is the point: a survey that treats an
    // unstated count as zero would call every readable block above the source an over-claim, and
    // that is reachable on a card that answers perfectly well. Inverted so the default advertises.
    bool hides_memory;

    // A SECOND tag answers the inventory. The SDK's inventory is 1-slot and unaddressed (#251), so
    // with two cards in the field it returns whichever wins the slot -- which may not be the one our
    // addressed writes are reaching. Set to make every inventory answer for the bystander while the
    // tag itself goes on behaving normally; that is the shape a stray UID has to be rejected in.
    bool bystander_answers_inventory;
    uint8_t bystander_uid[ISO15693_3_UID_SIZE];

    // Refuse any WRITE BLOCK whose OPTION flag is clear, with error 0x03 -- the tag naming the bit
    // rather than failing generically. Measured on TI Tag-it HF-I Plus (`white-coin`): flags 0x22 is
    // answered `01 03`, and the identical frame at 0x62 is taken.
    bool requires_option;

    // Apply the write and then say NOTHING. The other half of the same card: with OPTION set, the
    // answer is owed only after the reader sends a standalone EOF, which this SDK cannot do, so the
    // block is programmed and the acknowledgement never comes. Kept SEPARATE from requires_option,
    // although one card has both, so a test can put the two failures on their own -- the production
    // rescue is scoped to cards that asked for the flag, and that scoping needs a case of its own.
    bool writes_are_unacknowledged;

    // Counters, for assertions and for ops_until_lifted.
    uint32_t ops;
    uint32_t writes_attempted;
    uint32_t writes_accepted;
    uint32_t reads_attempted;
    uint32_t inventories;
} FakeTag;

extern FakeTag fake_tag;

// The activation cache nfc_poller_get_data() hands back. Seeded separately from the live tag on
// purpose: a card that answered at activation and degraded during the sweep has real content here,
// while a card already degraded before activation has zeros -- and those two produce different,
// deliberately different, sweep outcomes.
extern Iso15693_3Data fake_activation_cache;

// Reset everything: a 0-block tag, no lift, 1 tick per op, empty log.
void fake_tag_reset(void);

// The byte fake_tag_init and fake_data_init fill with. Exposed because a test that wants to prove a
// block was NOT written has to name the value it expects to still be there -- asserting "non-zero" is
// not enough when the source is filled with the same marker, which is how two gen1 tests came to pass
// against a mutant that wrote straight over the backdoor blocks.
#define FAKE_MARKER (0xA5U)

// Give the tag `advertised` blocks of `block_size` bytes, `physical` of them present and writable,
// the rest absent. Blocks below `physical` are filled with FAKE_MARKER.
void fake_tag_init(uint16_t advertised, uint16_t physical, uint8_t block_size);

// Mark [first, last] inclusive as `kind`, leaving content alone.
void fake_tag_set_range(uint16_t first, uint16_t last, FakeBlockKind kind);

// Fill [first, last] inclusive with a non-zero marker, or with zeros.
void fake_tag_fill(uint16_t first, uint16_t last, uint8_t byte);

// Build the activation cache the way iso15693_3_poller_activate would: walk up from block 0 copying
// content, and stop at the first block that does not answer a read, leaving the remainder zeroed.
// This is the faithful default and most tests should call it.
void fake_tag_cache_from_activation(void);

// Seed the cache from the tag's CURRENT content for every advertised block, regardless of whether the
// block answers now. Models "these blocks were readable when the card was presented, and died during
// the sweep" -- which is what makes the tail-drop discriminator able to fire at all.
void fake_tag_cache_all_advertised(void);

// ---- source images, for the clone loop ------------------------------------------------------------
//
// A clone source is an Iso15693_3Data loaded from a .nfc file, so its geometry is whatever the file says
// -- hand-editable and bounded by nothing the app controls. These build one directly.

// `blocks` blocks of `block_size` bytes, every block filled with a non-zero marker.
void fake_data_init(Iso15693_3Data* data, uint16_t blocks, uint8_t block_size);

// Fill [first, last] inclusive with `byte`. Use 0 to make a block "empty", which is what lets a failed
// write be excused as past the card's capacity.
void fake_data_fill(Iso15693_3Data* data, uint16_t first, uint16_t last, uint8_t byte);

// ---- the field, for the write state machine ------------------------------------------------------
//
// The poller returns NfcCommandReset to power-cycle the RF field and re-activate, and every UID verify
// runs on the far side of one. These let a test drive that loop.

// Power-cycle the field: latch any pending gen1 UID, and rebuild the activation cache the way a fresh
// activation would. Call this wherever the poller asked for NfcCommandReset.
void fake_tag_power_cycle(void);

// Arm a gen1-style UID change: takes effect on the NEXT power_cycle, not now.
void fake_tag_arm_gen1_uid(const uint8_t* uid);

// Set the UID the card answers with right now, no power-cycle needed (the gen2 behaviour).
void fake_tag_set_uid_now(const uint8_t* uid);

// Captured FURI_LOG output, newest last, as one newline-joined buffer.
const char* fake_log_text(void);
void fake_log_clear(void);
