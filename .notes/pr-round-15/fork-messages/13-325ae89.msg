NFC Magic ISO15693: two simplifications of earlier-round code

Neither changes what the app does.

The Details page decided whether its block list reads "Didn't fit on the card"
on the capacity facts OR the over-capacity reason code. The reason term adds
nothing: the only route to that reason is a Success carrying an over-capacity
count, and a Success has no failed blocks, which is the capacity facts' own
test. The comment beside it already said the label has to come from those facts
rather than from the reason code, so the term goes, with the local behind it and
one that only renamed the result.

And Start took the card's presented UID twice, once inside the wipe's branch and
once after it, from the same activation data into the same two fields. It is
taken once, before the branch, with one comment for what each copy is for.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
