NFC Magic ISO15693: a SLIX save from the stock NFC app is a clone source

The stock NFC app saves an NXP ICODE SLIX tag -- SLIX, SLIX-S, SLIX-L or
SLIX2 -- under its SLIX protocol, and file select took only ISO15693-3 dumps as
the source of an ISO15693 clone. So an ordinary source was refused with "This is
wrong card": a tag read with the stock app and saved could not be cloned here.

SLIX is built on ISO15693-3 in the SDK's protocol tree, and a clone writes only
ISO15693-3 data: the UID, the blocks, AFI and DSFID, and on a gen2 card the
geometry and IC ref from the system information. nfc_device_get_data hands that
data over for a SLIX file when asked for the parent protocol, and the write
already asks for it that way. So file select now takes any protocol built on
ISO15693-3, through nfc_protocol_has_parent, and nothing downstream changes.

The SLIX-only settings the file carries -- its passwords, privacy mode,
protection and EAS state -- are not written, by this path or any other here.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
