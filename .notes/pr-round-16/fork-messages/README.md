# Round 16 — the survey's results as one struct

One sync point, S4 from [final-review-3.md](../../pr-round-15/final-review-3.md): the survey's eleven
result fields, kept in four places, become `Iso15693PollerSurvey`. It changes no behaviour.

**Why it is its own round.** Inside round 15 it would have had to be written at 06, where the survey
starts, to add no churn: a fold touching every later sync point that reads its fields. mfcarroll's call, 2026-09-30: push round 15 as it stood and put S4 in this PR as its own
commit, and the round-15 reply's closing list told the maintainer it was coming.

Round 16 may also carry whatever his review of round 15 asks for, and the release-notes move to
`ISO15693.md` if he takes up the question the reply asked (a draft is in
[../ISO15693.md](../ISO15693.md)); each would be a sync point of its own below this one.

| # | sync at | one decision |
|---|---|---|
| 01 | `571e41f` | the survey's results are one struct |

**Its base** is `c8eeaade`, round 15 as pushed: dev's tree before `571e41f` carries no shipped change
since round 15's last anchor, so the replay's base check holds.
