NFC Magic ISO15693: three claims corrected in one place and not in their twin

The same failure three times, and worse than not correcting them at all: a file
that contradicts itself sends the reader looking for which half is current.

- The in-band error code was scoped to the tested chip at its definition and left
  general at the frame-builder and in the wipe's OPEN QUESTION. The second is
  load-bearing: that an armed card stays armed while its UID moves rests on the
  commit write being REFUSED rather than silently dropped, and the in-band
  evidence for that is one chip. Both now name the chip.
- The release notes still promised the wipe reports "whether it moved", a binary
  answer the code cannot give, while the consent screen already said it reports a
  move when it sees one. uid_changed is set only from a positive observation.
- The card-lost note said Iso15693WriteStateVerifyWipe has one exit. It has two,
  the case body and the activation-error path that tests for that state, and both
  report success_or_partial -- so the invariant the note pins is better supported
  than the note claimed.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
