# Round 16 — the release notes move to ISO15693.md, ahead of S4

The maintainer took the question round 15's reply asked. mfcarroll's call, 2026-10-07: push the
changelog change and the new reference file now, and hold S4 (the survey struct, `571e41f` on
`iso15693-dev`) so it does not churn code still under review.

It was built on **`round16-docs-v2`**, a branch from round 15's tip `13120668` carrying the tooling
commit and one docs commit, and `iso15693-dev` was then rebuilt on it (2026-10-07, mfcarroll's
go-ahead, force-pushed): S4 and its notes now sit above the docs. The 2.3 entry
is mfcarroll's 2026-09-29 draft, reworked by mfcarroll on 2026-10-07 into one Added bullet in the
style of 2.1, with the Magic candidate line; ISO15693.md gained that line and four gaps (Info with
no memory report, the wrong-file screen, locks not copied, non-4-byte files), folded into the docs
commit on 2026-10-07 (`wip-pre-docs-fold` = the tip before the fold). The working branches are deleted;
`wip-pre-docs-reorder` keeps the old tip `c9c0d198` until round 16 is pushed.

| # | sync at | one decision |
|---|---|---|
| 01 | `ae3faf1` | a short 2.3 entry, and ISO15693.md as the reference |

**Its base** is `c8eeaade`, round 15 as pushed: the tree before `ae3faf1` carries no shipped change
since round 15's last anchor.

**Folded again 2026-10-07** (`wip-pre-docs-fold2` = the tip before): mfcarroll's rework of
ISO15693.md -- the Important list, the ISO15693 primer, gen1 first, plainer wording -- plus the
review against the old 2.3 entry and the gen2 tested-on fix, all into the docs commit `ae3faf1`.

**PUSHED 2026-10-08:** `c8eeaade..f9a30516`, a fast-forward, signed, by mfcarroll. Reply posted as
[issuecomment-6051389961](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-6051389961).
