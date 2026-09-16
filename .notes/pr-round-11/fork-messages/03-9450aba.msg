NFC Magic ISO15693: four mangled comment wraps, and a note naming one mode of three

ReflowComments is false, so the formatter cannot see a paragraph whose wrap was
broken by an edit to the text above it. Four had been left that way, in
iso15693_poller.c, iso15693_poller.h, nfc_magic_scene_iso15693_partial_details.c
and iso15693_info.c. Text unchanged, wraps only.

Separately, nfc_magic_scene_write.c described its card-lost branch as ISO15693
clone behaviour. CardLost also reaches it from the wipe and from the gen1 and
gen2 verifies, and everything the branch does is right for all three, so the
word goes.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
