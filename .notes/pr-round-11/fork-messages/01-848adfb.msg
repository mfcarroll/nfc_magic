NFC Magic ISO15693: the comments state the constraints and stop arguing for them

The ISO15693 comments had grown to carry the reasoning that produced each rule
as well as the rule. A later reader needs the constraint -- what will break if
this is undone -- and can do without the derivation that reached it.

So the argument comes out and the constraints stay. 99 comment lines across the
feature, with no code change: the diff is comment-only and the preprocessed
output is byte-identical. The largest single cut is the pass-time bound, where
twenty lines comparing wipe and clone airtime become the one sentence the bound
actually rests on.

The 2.3 release notes get the same treatment, 177 lines to 109. They had begun
explaining their own reasoning rather than saying what changed.

Two things were deliberately not cut. Anything stating a constraint no test
enforces stays whatever its length, and so does every figure a reader would
otherwise have to re-measure.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
