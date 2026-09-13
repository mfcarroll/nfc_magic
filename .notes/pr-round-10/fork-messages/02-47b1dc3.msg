NFC Magic ISO15693: the gen1 consent screen stops reporting how well gen1 is validated

Both bodies ended "Gen1 is not hardware-tested." It is tested: the four-frame
sequence sets the UID, it reads back, and the original restores byte-identically.

The screen does not get a corrected version of that sentence -- it loses it. The
caveat was load-bearing only while gen1 had never been validated at all. What the
user consents to is the write and its blast radius: gen1 sets the UID with an
ordinary WRITE BLOCK into 56/57/62/63, which any writable tag accepts, so on a tag
that is not magic those four blocks are destroyed. How well the path is validated is
not something they can act on at the moment of consent.

Both bodies now end at the consequence. A note at the branch says why there is no
validation line, so a later pass does not add one back.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
