# Round 11 — fork messages, and why these six sync points

`replay-to-fork.sh` makes one fork commit per message here, syncing the **tree at that dev commit**.
So the sync points are the lever that decides what the reviewer watches happen. Twelve dev commits
touch shipped files this round; six become fork commits.

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
| 01 | `848adfb` | the cut — source and release notes, final form |
| 02 | `1c54240` | the card-lost term is defensive |
| 03 | `9450aba` | four rewraps, and the mode-specific word |
| 04 | `a45c478` | no-latch covers three chips |
| 05 | `4075026` | the consent screen's missing note |
| 06 | `bbdf965` | the sweep's reach |

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
