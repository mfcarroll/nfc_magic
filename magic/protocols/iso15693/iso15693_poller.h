#pragma once

#include <nfc/nfc_poller.h>
#include <lib/nfc/protocols/iso15693_3/iso15693_3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso15693Poller Iso15693Poller;

// Size (bytes) of the per-block failure bitmap; covers up to 256 blocks (the ISO15693 max).
#define ISO15693_POLLER_BLOCK_BITMAP_SIZE (32U)

typedef enum {
    Iso15693PollerModeInfo, // detect + read UID / system info
    Iso15693PollerModeWriteUid, // magic backdoor UID write (gen2 first, then gen1 if untouched)
    Iso15693PollerModeClone, // write UID + all data blocks from a source image
    Iso15693PollerModeWipe, // zero every data block (UID left unchanged)
} Iso15693PollerMode;

typedef enum {
    Iso15693PollerEventSuccess, // Info: card read. Write/clone: the target UID read back and matched
        // (only the UID is re-read; block contents are not compared). Wipe: every writable data block
        // accepted the zero write (backdoor registers 56/57/62/63 are skipped; the UID is untouched
        // and never re-read).
    Iso15693PollerEventPartial, // the operation mostly worked but isn't a clean result: a clone lost
        // some data blocks, fell back to gen1 (overwriting 56/57/62/63), or had its AFI/DSFID write
        // rejected; or a wipe couldn't clear every block.
    Iso15693PollerEventFail, // the operation didn't take: a write/clone backdoor was rejected (not a
        // magic tag), or a wipe cleared nothing.
    Iso15693PollerEventCardLost, // no card in the field / card removed before the operation finished
    Iso15693PollerEventCardDetected, // a magic candidate activated (drives the write popup UI)
    Iso15693PollerEventNotGen2, // clone: gen2 left the UID unchanged (not a gen2 magic card, or not
        // magic at all). Nothing was written; the scene offers the opt-in gen1 retry.
} Iso15693PollerEvent;

typedef void (*Iso15693PollerCallback)(Iso15693PollerEvent event, void* context);

Iso15693Poller* iso15693_poller_alloc(Nfc* nfc);

void iso15693_poller_free(Iso15693Poller* instance);

// Detect + read (Info mode). Emits Success once a card is read, or CardLost after a bounded number
// of activation attempts with no card in the field.
void iso15693_poller_start(
    Iso15693Poller* instance,
    Iso15693PollerCallback callback,
    void* context);

// Magic UID write. `uid` is ISO15693_3_UID_SIZE bytes, MSB-first (uid[0] must be 0xE0).
// The poller writes the gen2 backdoor sequence first (a harmless custom command on a non-magic tag)
// and, only if the gen2 write left the card's UID unchanged, falls back to the destructive gen1
// WRITE-BLOCK sequence. Before each read-back it power-cycles the field (like proxmark's
// switch_off + getUID) so a card that only latches the new UID after a reset is not misreported as a
// failure. Reports Success only if a read-back inventory returns the requested UID.
// The byte-level frames are defined in iso15693_poller.c (ported from proxmark3 armsrc/iso15693.c,
// SetTag15693Uid / SetTag15693Uid_v2).
void iso15693_poller_start_write_uid(
    Iso15693Poller* instance,
    const uint8_t* uid,
    Iso15693PollerCallback callback,
    void* context);

// Full clone (gen2 attempt): write `source`'s UID via the gen2 backdoor FIRST, and only once that UID
// reads back does it write every data block (standard WRITE BLOCK) -- so a non-magic tag is never
// clobbered by a doomed clone. `source` is an ISO15693-3 image loaded from a saved .nfc. Reports
// CardDetected (first activation), then Success (UID + all blocks), Partial (some blocks / the
// AFI/DSFID write failed), Fail (source has no data blocks) or CardLost. If gen2 leaves the UID
// unchanged (not a gen2 magic card) it reports NotGen2 without writing anything, so the caller can
// offer the destructive gen1 retry via iso15693_poller_start_clone_gen1().
void iso15693_poller_start_clone(
    Iso15693Poller* instance,
    const Iso15693_3Data* source,
    Iso15693PollerCallback callback,
    void* context);

// Opt-in gen1 clone retry (call after start_clone reported NotGen2 and the user confirmed). Writes the
// destructive gen1 UID sequence FIRST (stamping the UID/unlock/commit into blocks 56/57/62/63) and,
// only if that UID reads back, writes the data blocks -- skipping 56/57/62/63, which now hold the UID,
// so they can't match the source -> Partial. A card that can't do gen1 therefore loses at most those
// four blocks. Reports Success/Partial/Fail/CardLost like start_clone. NOTE: gen1 is NOT
// hardware-validated.
void iso15693_poller_start_clone_gen1(
    Iso15693Poller* instance,
    const Iso15693_3Data* source,
    Iso15693PollerCallback callback,
    void* context);

// After a clone, the per-block write result. A "failure" here means the block failed EVERY write
// retry (a transient glitch that later succeeded is not a failure):
//   blocks_total      - source block count.
//   failed_count      - blocks that failed and count as a real problem: they held source data (data
//                       lost), or were empty failures that weren't a clean top-of-card tail. -> Partial.
//   over_capacity     - empty blocks that failed and form a contiguous run at the top of the card
//                       (past physical capacity; nothing lost). -> Success with a note.
//   failed_bitmap     - bit N set = source block N failed (covers both buckets above).
//   used_gen1         - the gen1 fallback set the UID (which overwrites blocks 56/57/62/63).
//   capacity_confirmed- the failures are a persistent, contiguous run at the very top of the card,
//                       i.e. the source is genuinely larger than the card's physical capacity. False
//                       for a scattered/anomalous failure (reported generically, no capacity claim).
//   identity_failed   - the source reported an AFI / DSFID but the card rejected the WRITE AFI / WRITE
//                       DSFID, so that field may not be set (best-effort; can over-report on a card
//                       that accepts the write without answering). -> Partial.
// Any out param may be NULL. `failed_bitmap` must hold ISO15693_POLLER_BLOCK_BITMAP_SIZE bytes.
void iso15693_poller_get_clone_result(
    Iso15693Poller* instance,
    uint16_t* blocks_total,
    uint16_t* failed_count,
    uint16_t* over_capacity,
    uint8_t* failed_bitmap,
    bool* used_gen1,
    bool* capacity_confirmed,
    bool* identity_failed);

// True if the source stores real data in a gen1 backdoor block (56/57/62/63) that a gen1 fallback
// would overwrite -- so the write flow can warn before a possible gen1 clone. Source inspection only.
bool iso15693_poller_source_uses_gen1_blocks(const Iso15693_3Data* source);

// Wipe: write zeros to every data block on the card (UID left unchanged, like proxmark's
// 'hf 15 wipe'; the gen1 backdoor registers 56/57/62/63 are skipped so the UID / magic state is
// preserved). Reports CardDetected (first activation), then Success / Partial (some blocks failed) /
// Fail (nothing could be wiped) / CardLost. Per-block detail is available via
// iso15693_poller_get_clone_result().
void iso15693_poller_start_wipe(
    Iso15693Poller* instance,
    Iso15693PollerCallback callback,
    void* context);

void iso15693_poller_stop(Iso15693Poller* instance);

// The last Info-mode read result (UID + system info + block data). Owned by the poller; valid until
// the poller is freed, so the Info scene copies it out. Read-only.
const Iso15693_3Data* iso15693_poller_get_data(Iso15693Poller* instance);

#ifdef __cplusplus
}
#endif
