# Round 16 — the release notes move to ISO15693.md, ahead of S4

The maintainer took the question round 15's reply asked. mfcarroll's call, 2026-10-07: push the
changelog change and the new reference file now, and hold S4 (the survey struct, `571e41f` on
`iso15693-dev`) so it does not churn code still under review.

S4 sits below the docs on `iso15693-dev`, so this round is replayed from **`round16-docs-v2`**, a
branch from round 15's tip `13120668` carrying the tooling commit and one docs commit. The 2.3 entry
is the 14-line one drafted on 2026-09-29, with two corrections. (`round16-docs` was the first
attempt, with a 35-line entry; it is superseded.) `iso15693-dev` was left as it was; S4 and its
notes rebase onto this branch when it is wanted.

| # | sync at | one decision |
|---|---|---|
| 01 | `2235a9e` | a short 2.3 entry, and ISO15693.md as the reference |

**Its base** is `c8eeaade`, round 15 as pushed: the tree before `2235a9e` carries no shipped change
since round 15's last anchor.
