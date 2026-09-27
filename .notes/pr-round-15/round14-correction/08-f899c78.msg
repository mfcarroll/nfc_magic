NFC Magic ISO15693: four comments state the rule rather than its history

Comment-only. Four comments explained a rule by recounting the code it
replaced -- "was previously set", "used to fall through", "used to sit",
"previously disagreed". The reader deciding whether to change the line in
front of them needs the rule, not how the code got there, which goes stale
unnoticed. Each now says what holds, and what breaks if it is undone:

  - gen1_attempted is set at the send. Set any earlier it would be true for a
    run that never transmitted, and the scene would report spent registers on
    the likeliest outcome of the opt-in screen.
  - the write-state switch has no default, so -Wswitch makes a forgotten state
    a build error. A default sharing an arm would run that arm silently, and
    VerifyGen1 is the worst of the four to land in.
  - the gen1 opt-in carries the source-side warning because a clone never
    reaches the up-front confirm.
  - the write scene's header and its progress line share one wiping test,
    because a progress line testing only the USCUID-UL flag would call an
    ISO15693 wipe "Writing".

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
