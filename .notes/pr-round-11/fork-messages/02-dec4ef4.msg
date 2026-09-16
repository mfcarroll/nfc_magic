NFC Magic ISO15693: the write-fail card-lost term is defensive, and says so

The CardLost branch tests mode == Wipe && !uid_verified. The second term cannot
currently be false: uid_verified is set only in Iso15693WriteStateVerifyWipe,
whose one exit is success_or_partial, and that returns Fail, Partial or Success,
never CardLost.

It stays rather than going, because of where it sits. This scene reads a result
struct it did not fill and cannot enforce the poller's invariant, so the term is
worth having against a poller that grows that exit later.

What changes is the comment, which described the pairing as a case that occurs.
It now says the term is defensive and names the invariant that makes it dead.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
