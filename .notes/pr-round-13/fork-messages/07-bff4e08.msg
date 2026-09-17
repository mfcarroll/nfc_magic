NFC Magic ISO15693: an over-capacity clone copied "exactly" what it never compared

Thirteen lines apart, the release notes said the blocks that fit are "copied
exactly" and that block contents are not compared, a data block counting as
written when the card acknowledges it. The second is the one the code supports;
"exactly" promises a verification the clone does not perform.

Now: every block the card acknowledged was written.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
