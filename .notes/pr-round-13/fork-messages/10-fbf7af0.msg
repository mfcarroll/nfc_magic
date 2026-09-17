NFC Magic ISO15693: the write-fail layout note documents its y values and not its x

Twelve widget bodies in this file use two coordinate pairs, (0, 13) and (4, 20).
The LINE BUDGET note explains both y values in detail and never mentions x, so
the x=4 on two branches out of twelve reads as an inconsistency.

It is a convention. nfc_magic_scene_gen2_wipe_partial.c and
nfc_magic_scene_uscuid_ul_partial.c both place their bodies at exactly (4, 20),
and the two branches here that use it are the two whose own comments say they
mirror those screens.

Worth recording because the alternative reading is to make all twelve agree,
which would silently break the match with two other screens in order to remove an
inconsistency that is not one.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
