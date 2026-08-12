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
    // the sweep into ISO15693_POLLER_WIPE_MAX_MS.
    uint32_t tick_cost_per_op;

    // A UID written into the gen1 registers latches only on the NEXT power-up: until then the card keeps
    // answering the old one. That is the whole reason every UID verify in the poller sits behind a
    // NfcCommandReset, so the fake has to model it or those tests prove nothing.
    // Set by fake_tag_arm_gen1_uid(); applied by fake_tag_power_cycle().
    bool gen1_uid_pending;
    uint8_t gen1_pending_uid[ISO15693_3_UID_SIZE];

    // gen2 magic: the backdoor UID write takes effect immediately, no power-cycle needed. A tag that is
    // not magic at all leaves both of these false and keeps its UID whatever is written.
    bool is_gen2_magic;
    bool is_gen1_magic;

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

// Give the tag `advertised` blocks of `block_size` bytes, `physical` of them present and writable,
// the rest absent. Blocks below `physical` are filled with a recognisable non-zero marker.
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
