# NFC Magic: Back during a Gen2/Classic write is inescapable, and silently restarts the write from block 0

**Pre-existing** — present on `main`, unrelated to #250, found while testing that PR.

## What

`nfc_magic_scene_gen2_write_check_on_enter()` pushes the write scene from **on_enter** when the target
has no write problems:

```c
if(problems.all_problems == 0) {
    if(instance->gen2_poller_is_wipe_mode) {
        scene_manager_next_scene(instance->scene_manager, NfcMagicSceneWipe);
        return;
    } else {
        scene_manager_next_scene(instance->scene_manager, NfcMagicSceneWrite);
        return;
    }
}
```

It is a pass-through scene, but it stays on the scene stack. So Back from the write scene pops to it,
its `on_enter` runs again, and it immediately pushes the write scene forward again:

```
Write ──Back──▶ Gen2WriteCheck ──on_enter──▶ Write ──Back──▶ Gen2WriteCheck ──▶ …
```

The user sees the write screen return to its card-search state, "Apply the same card to the back", and
no amount of pressing Back escapes it.

## Second effect: the write silently restarts

`nfc_magic_scene_write_on_enter()` allocates the poller and `on_exit` frees it, so each trip round the
loop is a **brand-new write starting from block 0** — not a resumption.

That is easy to mistake for a resumed write. Lift the card mid-write, press Back, re-apply the card, and
the clone completes — but it has rewritten the entire card from scratch, and any partial-write counts
shown afterwards describe the fresh attempt rather than the interrupted one.

## Confirmed by log

Four Back presses during one stuck Gen2 write, each tearing down and re-creating the poller:

```
9370728 [D][GEN2] Stopping Gen2 poller
9409962 [D][GEN2] Stopping Gen2 poller
9410660 [D][GEN2] Stopping Gen2 poller
9410814 [D][GEN2] Stopping Gen2 poller
9410953 [D][GEN2] Stopping Gen2 poller
9413351 [D][GEN2] Block 0 is the same, skipping   <- card re-applied: restarted from block 0
```

The last line is the silent restart: re-applying the card began a fresh write at block 0 rather than
resuming where the interrupted one stopped.

## Repro

1. Gen2 / Classic clone, onto a target with no write problems (so the check scene passes straight
   through).
2. At "Writing / Don't move...", press Back.
3. The write screen reappears at "Apply the same card to the back". Back again does the same. There is
   no exit.

Reachable without removing the card at all — any Back during a Gen2/Classic write hits it.

## Why it is not usually noticed

The loop needs the write screen to still be on top when Back is pressed. If the write completes first,
the result screen replaces it and the check scene is never re-entered from behind. It shows up when a
write is slow or has stopped reporting — which is exactly the situation in the companion issue about
Gen2/USCUID writes never terminating, so the two compound: the write does not resolve, and Back does not
get you out.

## Direction

The check scene shouldn't remain on the stack when it has nothing to display. Options for discussion:

- have the caller decide, so the check scene is only entered when `all_problems != 0`;
- or replace rather than push — `scene_manager_search_and_switch_to_another_scene` — so no frame is left
  behind to re-fire;
- or record in the scene state that it has already passed through, and have `on_enter` return to the
  menu on the way back rather than pushing forward again.

## Where

`base_pack/nfc_magic/scenes/nfc_magic_scene_gen2_write_check.c` (`on_enter`). Worth checking
`nfc_magic_scene_mf_classic_write_check.c` for the same shape.
