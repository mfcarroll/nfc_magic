NFC Magic ISO15693: two warnings reach the confirm screen, so say which one names the cost

"The gen3 line is the only warning that reaches the person holding the card" was
true when it was written and stopped being true in the same commit: dbc6e4fa also
changed the title to "Wipe? (gen1/gen2 only)", which is on the same screen, in
front of the same person, doing warning work. You found the pair.

Your replacement is the right one -- "the only warning that names the cost" is what
the paragraph is actually arguing, since the rest of it is about this being a
static line rather than a detection. The title warns about SCOPE and cannot say
anything about consequence, so the two are not redundant and the distinction is
worth one clause.

Comment-only, net +1 line: the paragraph rewrapped around the edit.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
