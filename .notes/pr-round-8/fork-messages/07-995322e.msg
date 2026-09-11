NFC Magic ISO15693: name the screen that states the cut, instead of "that screen"

WipeStopped's reason doc ended "the wipe short-circuits to NothingWiped before the
WipeStopped truncation reporting -- that screen states the cut itself." The
pronoun binds to WipeStopped, the screen the same sentence has just said is
skipped, which makes the reassurance vacuous. The screen that does state the cut
is NothingWiped (write_fail.c:284-288).

The round-7 fix removed this parenthetical's overreach and left the pronoun
pointing at the wrong end of the sentence it had just been rewritten around.

Comment-only; the trailing sentence rewrapped because the fix is one word longer.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
