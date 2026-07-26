# Worklog — offline hardening pass

Branch `slix-v2`. Started 2026-07-26. All changes are static-review-only (no `ufbt` build here);
each is matched to the SDK API + existing patterns by inspection. On-hardware validation is tracked
in [hardware-plan.md](hardware-plan.md).

## Planned commits (offline)
1. docs: add `.notes/` (analysis, protocol reference, hardware plan, worklog).
2. #6 generic "ISO15693 / NfcV" labels.
3. #4 enforce entered UID starts with `0xE0`.
4. #5 SLI / SLIX / SLIX2 chip decode.
5. #2 + #3 + #1(poller) rewrite the write path: power-cycle-before-verify state machine,
   gated gen1 fallback, distinct Success / Fail / CardLost outcomes; #7 Info-mode timeout.
6. #1(UI) SLIX-specific write-fail scene (not-magic vs card-lost).
7. groundwork: `slix_write_confirm` scene before the irreversible write.
8. #8 cleanup: portability `#define`, stale header docs, dead enum removal.
9. docs: update this worklog + CHANGELOG.

## Status log
_(filled in as commits land — see "Done" below)_

## Done
- [x] Commit 1 — `.notes/` docs added.
