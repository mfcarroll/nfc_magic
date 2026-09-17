NFC Magic ISO15693: the pass-cut helper says why it is named for what it writes

iso15693_poller_cut_pass_if_expired returns a bool, and a bool-returning name in
the `..._expired` shape would read as a query. It is not one: it sets
pass_truncated and pass_cut_block, and C gives the call site no signal of that.

What makes it worth saying rather than leaving to the signature is the second
field. pass_cut_block is the EXCLUSIVE END OF THE ATTEMPTED RANGE, which the
clone's back-fill and the wipe's tail-drop both read. A third caller adopting
this from inside a loop over anything other than the block cursor -- the re-probe
is the tempting one, ninety lines away -- would record a cut index that is not
that, and every screen downstream would state it as fact.

Precedent in the same file is iso15693_poller_wipe_note_present, which also
mutates and says so in its name.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
