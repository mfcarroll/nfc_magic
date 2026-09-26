NFC Magic ISO15693: the release notes overstate what a gen1 clone reproduces

Two user-facing claims in the release notes that are not true as written.

The clone bullet said it writes the source's identity -- IC ref, block geometry,
AFI, DSFID -- "so the copy advertises the same chip", without qualification.
That is the gen2 path. The gen2 backdoor has a CFG register that programs what
the card reports; gen1 has none, so a gen1 clone carries the source's UID and
data on silicon that goes on truthfully reporting its own size and IC reference.

MEASURED. A 40-block SLIX-S cloned from a 28-block SLIX source with gen1 answers
to the source's UID and carries its data while still reporting 40 blocks and IC
ref 0x02. The type line a reader prints for it is decoded from that UID rather
than from the chip.

The gen1-fidelity bullet said a clone that used gen1 reports Partial and flags
blocks 56/57/62/63. That is now conditional: it is Partial only where the source
actually reached those blocks, so the line says so.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
