# NFC Magic: Back during a Gen2/Classic write is inescapable, and silently restarts the write from block 0

> Field-by-field for the pack's bug-report form. The H1 above is the **Title** field.

### App

NFC Magic

### App version

2.0

### Describe the bug

Pressing Back during a Gen2/Classic write does not leave the write screen. It returns to it, at its
card-search state — **"Apply the same card to the back"** — and pressing Back again does the same. There
is no way out of the loop.

`nfc_magic_scene_gen2_write_check_on_enter` pushes the write scene from **`on_enter`** when the target
has no write problems:

```c
if(problems.all_problems == 0) {
    ...
    scene_manager_next_scene(instance->scene_manager, NfcMagicSceneWrite);
    return;
}
```

It is a pass-through scene but stays on the scene stack, so Back pops to it, its `on_enter` runs again,
and it immediately pushes the write scene forward again:

```
Write --Back--> Gen2WriteCheck --on_enter--> Write --Back--> Gen2WriteCheck --> ...
```

**Second effect: the write silently restarts.** `nfc_magic_scene_write_on_enter` allocates the poller and
`on_exit` frees it, so each trip round the loop is a **brand-new write starting from block 0**, not a
resumption. Lift the card mid-write, press Back, re-apply it, and the clone completes — having rewritten
the whole card from scratch. Any partial-write counts shown afterwards describe the fresh attempt, not
the interrupted one.

### Reproduction

1. Load a saved MIFARE Classic dump.
2. Start a clone onto a Gen2/Classic target that has no write problems, so the check scene passes
   straight through.
3. At **"Writing / Don't move..."**, press Back.
4. The write screen reappears at **"Apply the same card to the back"**.
5. Press Back again — the same screen. There is no exit.

Reachable without removing the card at all: any Back during a Gen2/Classic write hits it.

### Firmware version

_(fill in the Unleashed/Momentum version you tested on)_

### Logs

Four Back presses during one write, each tearing down and re-creating the poller, then the card
re-applied:

```
9370728 [D][GEN2] Stopping Gen2 poller
9409962 [D][GEN2] Stopping Gen2 poller
9410660 [D][GEN2] Stopping Gen2 poller
9410814 [D][GEN2] Stopping Gen2 poller
9410953 [D][GEN2] Stopping Gen2 poller
9413351 [D][GEN2] Block 0 is the same, skipping   <- restarted from block 0
```

### Anything else?

**Why it is not usually noticed.** The loop needs the write screen to still be on top when Back is
pressed. If the write completes first, the result screen replaces it and the check scene is never
re-entered from behind. It shows up when a write is slow or has stopped reporting — which is exactly
#ISSUE-STALL, so the two compound: the write does not resolve, and Back does not get you out.

**Possible directions.** The check scene should not remain on the stack when it has nothing to display:

- have the caller decide, so the check scene is only entered when `all_problems != 0`;
- or replace rather than push, so no frame is left behind to re-fire;
- or record in the scene state that it has already passed through, and have `on_enter` return to the
  menu on the way back rather than pushing forward again.

**Where.** `base_pack/nfc_magic/scenes/nfc_magic_scene_gen2_write_check.c` (`on_enter`). Worth checking
`nfc_magic_scene_mf_classic_write_check.c` for the same shape.

Found while testing #250, which does not touch this scene — it is byte-identical to 2.0.
