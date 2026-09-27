NFC Magic ISO15693: the write-fail layout note documents its y values and not its x

Twelve widget bodies in this file use two coordinate pairs, (0, 13) and (4, 20).
The LINE BUDGET note explains both y values in detail and never mentions x, so
the x=4 on two branches out of twelve reads as an inconsistency.

It is a convention. That pair appears in exactly three files in the repo -- here,
nfc_magic_scene_gen2_wipe_partial.c and nfc_magic_scene_uscuid_ul_partial.c --
and the two branches here that use it are the two that mirror those screens.
Making all twelve agree would break the match to remove an inconsistency that is
not one.

The claim rests on the repo-wide fact rather than on the two branch comments: one
says "mirroring the Gen2 / USCUID-UL partial screens", the other only "Concise
summary here, gen2-style", which names gen2 loosely and never mentions
USCUID-UL.

This is also the answer to whether those twelve calls want a wrapper: the
duplication is in describing the layout, not in the calls.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
