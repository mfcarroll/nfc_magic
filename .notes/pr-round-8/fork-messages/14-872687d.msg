NFC Magic ISO15693: the line budget covers both y values, so it can own both derivations

The scope line claimed "LINE BUDGET for every body in this file" and then derived
only the y=13 case. It re-scopes itself two lines later, so it was self-limiting
rather than wrong -- but two bodies sit at y=20 and derived their own budget
inline, which is the duplication the consolidation was meant to remove. e191411b's
message also claimed the arithmetic "is stated once at the top and the two sites
point at it", which is not what the file showed: the timed-out branch still
restated the y=13 arithmetic in full.

The claim is now true rather than narrowed. Every body placement in the file was
enumerated first -- eleven at y=13, two at y=20, one title at y=0 -- so "every
body" is a closed set of two cases, both stated at the top, both coming out at
three lines. The two sites that derived their own now point.

The derivations they keep are the local ones: that the timed-out body already
reaches three when blocks failed, and that the y=20 body spends its third line on
exactly one qualifier out of four in priority order. Those are decisions about
content, not about pixels.

+11 -9 comment lines, net +2. The bulleted form costs more lines than it saves and
buys the property that matters: one owner, two pointers, and no arithmetic stated
twice to drift.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
