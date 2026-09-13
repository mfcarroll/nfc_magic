NFC Magic ISO15693: gen1 is validated across three chips, and four claims the code no longer supports

The Validation section leads with gen1, since that is what changed, and states what
is true rather than how it came to be known: validated across ST LRi2K, NXP SLIX and
NXP SLIX-S, the UID takes effect immediately, and the four backdoor addresses are
not memory -- they answer no read, so a card's advertised block count is its
capacity and 56/57/62/63 are write-only registers outside it. That last point
separates two things these notes otherwise conflate: a gen2 card really can hold
data above its advertised count, which is why the wipe sweeps past the claim, and a
gen1 card's four magic addresses are not the same phenomenon.

Four corrections elsewhere in the notes.

The de-arm question was "flagged in the code as an open question pending a gen1 card
to test against". The hazard has been reproduced and the code reaches a conclusion:
no blind write ordering can de-arm a card, because the only route to the armed state
is the sequence that sets it.

#251's scope was a wipe writing data blocks. It is every ISO15693 write the app
sends, the clone's AFI/DSFID pass included -- and those are standard commands, so
they reach a bystander of any size and a changed AFI can drop a tag out of selective
inventory.

The wipe's time-limit bullet promised a retry unqualified. A wipe that cleared
nothing is a failure and offers none.

"Each outcome has its own screen: "Card removed", ..." quoted a title that does not
exist. A lost card renders "Write failed", with "Card removed before the write could
finish" in the body.

And the armed-gen1 hazard now says which cards it applies to. The sweep reaches
56/57 only from a claim of 49 or more, which of the cards tested is the 56-block one
alone.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
