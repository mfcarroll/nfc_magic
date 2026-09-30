NFC Magic ISO15693: the write-fail reason switches name every reason

Whether a write-fail reason has Details, and what its screen is titled, are
two switches on the reason. Each ended in a default -- no Details, and "Write
failed" -- and each switched on the scene state's uint32_t, where -Wswitch
checks nothing, so a reason added to the enum took the default's answer
without anyone choosing it.

Both now switch on the enum, list every reason and have no default. The
firmware builds with -Wall -Werror, so a reason left out of either is a build
error, and a value that is not a reason falls out of the switch into a
furi_crash -- the shape the poller's write-state switch already has.

And NotMagic was 0, which is also every scene state's starting value, so a
screen entered without its reason set said "Not a magic tag". The enum now
opens with Unset, which nothing sets and both switches crash on.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
