# Round 11 — fork messages, and why these seven sync points

`replay-to-fork.sh` makes one fork commit per message here, syncing the **tree at that dev commit**.
So the sync points are the lever that decides what the reviewer watches happen. Fifteen dev commits
touch shipped files this round; seven become fork commits.

## The churn these points avoid

A 1:1 replay would show him us breaking our own work and fixing it:

| dev | then | net |
|---|---|---|
| `d04f20d` deletes the sweep's carve-out | `848adfb` restores it | a reword |
| `d04f20d`/`1d86a9c` leave four sentences unparseable | `6a1a265`, `fa31120` fix them | nothing |
| `5c62d6d` drops the "claim of 49" sentence | `bbdf965` states the rule properly | a rewrite |

Every one of those is dev bookkeeping. The first fork commit syncs at **`848adfb`**, by which point
the cut, the parse fixes, the sense pass and the carve-out are all in their final state — so no fork
commit ever contains a wrong intermediate.

## The points

| # | sync at | one decision |
|---|---|---|
| 01 | `1d110ec` | the cut — source and release notes, restorations folded in |
| 02 | `dec4ef4` | the card-lost term is defensive |
| 03 | `1ae6e0f` | three rewraps, and the mode-specific word |
| 04 | `c541b67` | no-latch covers three chips |
| 05 | `7c38832` | the consent screen's missing note |
| 06 | `b415009` | the sweep's reach, stated as the safe case |
| 07 | `776806e` | block 57 needs one more silent block than 56 |

## Why the review round's corrections do not get their own commits

Dev history was REORDERED so the restorations sit immediately after the cut, which is what lets 01
sync at a tree that already has them. Without that, 01 would have shipped a dropped negation and a
later commit would have put it back -- the exact churn these points exist to avoid.

Two corrections stay separate on purpose. The reach correction folds into 06 because it corrects
06's own sentence. The 56/57 boundary does NOT fold into 01: it is a factual fix to a claim older
than this round, and one he quoted approvingly in round 11, so it needs to be visible as its own
decision rather than buried in a commit about comment volume.

The CHANGELOG is touched in 01 and again in 06. That is two decisions, not churn: after 01 it is
silent on the reach, which is not wrong, and 06 answers his thread on it.

## What these messages must not carry

The dev messages are **not** copyable this round. Several describe internal process that is
meaningless on the fork and reads as noise:

- `848adfb` — "the cut deleted the sweep's carve-out". The fork never sees it deleted.
- `1c54240` — half its message is about a host-harness test, which `sync-to-fork.sh` excludes, so it
  would describe a change absent from its own diff.
- `4075026` — cites a dev SHA and narrates a commit-message defect.
- `bbdf965` — "the cut removed the number without replacing the rule".

Each message here states the delta and the reason it is right, and nothing about when we did what.
