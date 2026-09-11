NFC Magic ISO15693: the wipe title is ISO15693's, so stop it reaching USCUID-UL

REGRESSION, on a protocol this PR does not add. dbc6e4fa put the gen3 warning on
the wipe confirm screen and moved the title with it, but set it from is_wipe --
which line 24 defines as `uscuid_ul_is_wipe_mode || iso15693_wipe`. The gen3 BODY
is gated on iso15693_wipe at the branch; the title was not.

So a magic Ultralight wipe (uscuid_ul_menu.c:99 sets the mode and goes straight
to this scene) rendered:

    Wipe? (gen1/gen2 only)
    Blank factory dump: config & password cleared, UID zeroed.

Worse than merely irrelevant: in this app Gen1 and Gen2 already name the MIFARE
Classic magic protocols (NfcMagicProtocolGen1 / Gen2), so on an Ultralight card
the parenthetical asserts something about a different chip family.

is_wipe goes back to "Wipe card?", correct for both, and the ISO15693 title is
set inside the iso15693_wipe branch -- the shape this file already uses, since
the write-UID branch sets title, confirm_label and text_height the same way. NOT
the one-token alternative: a nested ternary at line 30 leaves a protocol-specific
string on the line whose whole job is the protocol-neutral default, which is how
this happened once already.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
