NFC Magic ISO15693: a proxmark function that does not exist, and an omission note naming the wrong id

Three claims in iso15693_info.c, checked against proxmark3 rather than against
memory of it.

getTagInfo_15 has never existed. Nothing in client/ or armsrc/ defines it and no
commit ever added it. The function that walks the masked-UID table is
printTagInfo_15 and the table is `uidmapping`. The other provenance note in this
same file already says "uidmapping", so the file carried one real name and one
invented one -- and the invented one is what a maintainer would go looking for.

The ST25TV omission note named the wrong IC id. It said the "0x08 / 0x23-C variants"
are omitted because their ids collide, but 0x23 is the NON-C pair and it is in the
table three lines below the sentence saying it is absent. In proxmark both -C
variants sit at 0x08, which collides with LRI2K above. Reconciling the comment with
the table would have deleted the 0x23 entry and broken the ST25TV02K / ST25TV512
decode.

"A wider mask pins more chip-id bits" describes an intent the comparison does not
enforce: masks are compared numerically, which coincides with bit count only because
every mask in this table is a contiguous high-bit run. The file uses a sparse mask
thirty lines further down.

Comment-only.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
