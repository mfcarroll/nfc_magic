NFC Magic ISO15693: the wipe title is ISO15693's, so stop it reaching USCUID-UL

REGRESSION, and it lands on a protocol this PR does not add. dbc6e4fa put the gen3 warning on the wipe confirm screen and moved the title
with it, but set it from is_wipe -- which line 24 defines as
`uscuid_ul_is_wipe_mode || iso15693_wipe`. The gen3 BODY is gated on
iso15693_wipe at the branch; the title was not.

So a magic Ultralight wipe (uscuid_ul_menu.c:99 sets the mode and goes straight
to this scene) rendered:

    Wipe? (gen1/gen2 only)
    Blank factory dump: config & password cleared, UID zeroed.

Worse than merely irrelevant, as you point out: in this app Gen1 and Gen2 already
name the MIFARE Classic magic protocols (NfcMagicProtocolGen1 / Gen2), so on an
Ultralight card the parenthetical reads as an assertion about a different chip
family. dbc6e4fa's own message argues the title purely in ISO15693 terms -- "the
title is a scope statement, not a description" -- which is the evidence it was
never meant to cross over.

is_wipe goes back to "Wipe card?", correct for both, and the ISO15693 title is
set inside the iso15693_wipe branch. That is the shape the file already uses:
the write-UID branch sets title, confirm_label and text_height the same way. The
one-token alternative -- a nested ternary at line 30 -- puts a protocol-specific
string on a line whose whole job is the protocol-neutral default, which is how
this happened once already.

Two comment lines say why the title is not set from is_wipe. The union is right
for the body dispatch and wrong for anything naming a generation, and that
distinction is invisible at the declaration.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
