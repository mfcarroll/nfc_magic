# Round 16 — reply on #250 (POSTED 2026-10-08 in mfcarroll's own wording as [issuecomment-6051389961](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-6051389961); the draft below was the starting point)

Post between the `~~~~` markers, with the push of the release-notes commit. No internal process in
the payload.

~~~~
Thanks — done. The 2.3 entry is now a short one in the changelog's own style, and the rest is in
`ISO15693.md` beside it: the magic types, what each operation does, a table of the result screens,
the limits, and what it was tested on. It is the same material as before, laid out as a reference,
so later entries can say what changed while this file is kept current.

The entry keeps what someone should know before they touch a card: every ISO15693 tag shows as a
magic candidate until it is written to, a wipe can brick a gen3 card or move a gen1 card's UID, and
one tag in the field at a time.

This commit is documentation only. The struct simplification I mentioned is ready, but I'm holding it
until you've reviewed round 15's code, so it doesn't move code you're partway through.
~~~~
