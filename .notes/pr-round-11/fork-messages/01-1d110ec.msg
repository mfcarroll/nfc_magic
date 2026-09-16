NFC Magic ISO15693: the comments state the constraints and stop arguing for them

The ISO15693 comments had grown to carry the reasoning that produced each rule
as well as the rule. A later reader needs the constraint -- what will break if
this is undone -- and can do without the derivation that reached it.

So the argument comes out and the constraints stay. 84 comment lines across the
feature, with no code change: the diff is comment-only and the preprocessed
output is byte-identical. The largest single removal is cut_block's gap
derivation in the header, where eighteen lines working out the two bounds on
cut_block - blocks_total become nine that state them and keep the worked example.

The 2.3 release notes get the same treatment, 177 lines to 115. They had begun
explaining their own reasoning rather than saying what changed.

What stays, because a constraint is not an argument however long it runs: the
deadline gating behind "a cut WIPE has no back-fill", the interior-versus-
trailing distinction the tail-drop turns on, the over-claiming card beside the
cloned one, the estimate hedge on 40-70ms and the in-band-error mechanism that
ties it to ISO15693_POLLER_VERIFY_RETRY_MS, the applications/ prefix on a path
whose only job is locating a symbol that cannot be grepped from the SDK, and
0x6r1an0y's name on the claim that a wipe bricks a gen3 card permanently --
attribution for the strongest destructive warning this app makes, and one
nothing here has observed.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
