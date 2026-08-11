# Idea, parked: a bench that can stage the cases hardware can't

Raised 2026-08-11 while finishing round 3. Not started — recorded so it isn't lost.

## The problem it would solve

Several behaviours are now shipped on reasoning alone, because no card we own can be made to produce
them:

| behaviour | needs a card that... |
|---|---|
| the tail-drop fix's *positive* case | stops answering both a write and a read partway through its claimed range, and stays that way |
| every truncated-sweep screen | refuses every write while still answering reads, so the sweep runs to the clock |
| `uid_verified` false | fails to return from the field power-cycle, or stops answering inventory |
| a clone cut by the clock with the card still present | writes slowly enough to exceed 10s across 256 blocks |
| the whole gen1 path | is gen1 magic — neither side has one |

Each is a *reported* outcome, so getting them wrong is a wrong report rather than a crash, which is
exactly the class of bug this PR keeps finding.

## Option A — proxmark3 as a tag simulator

Drive `hf 15 sim` from the PM3 CLI while the Flipper runs the app.

Unknown worth checking first: how much ISO15693 *tag* simulation the PM3 actually supports. Its 14443a
emulation is mature; the 15693 side has historically been thinner than `hf 15 sim -u <uid>`, and what
this needs is selective per-block behaviour — block N answers reads but refuses writes, blocks 20..63
answer nothing — which may need firmware work rather than a script. Establish that before designing
anything around it.

## Option B — host-side tests of the decision logic

Probably the higher-value half, and independent of any radio. The sweep's arithmetic is pure logic over
the results of `write_block` / `read_block`: geometry in, counts and a bitmap out. With those two calls
behind a seam, every geometry mishamyte has traced by hand becomes a table-driven test — 28/64, 66/64,
200/64, advertised 0, advertised 256, lifted at block 5, a dead stretch that recovers, a dead stretch
that doesn't, and a clock cut.

That is precisely the set he has been re-deriving by hand each round, and the set I got wrong four times
on the poller-stall question.

Obstacle: the FAP builds against the firmware SDK, so a host build needs the seam plus stubs. Worth
scoping how much of `iso15693_poller_wipe_blocks` could move behind an injectable interface without
disturbing the shipped structure.

## Option C — drive both CLIs from one script

The end state the idea started from: a harness that scripts the PM3 and the Flipper together and asserts
on the Flipper's log output. Note the Flipper CLI needs a real TTY — piping into `./fbt cli` fails with
`termios.error: Inappropriate ioctl for device` — so this needs a pty wrapper (`pexpect` or similar)
rather than plain pipes.

The log lines are already shaped for it: `wipe: N blocks attempted, M cleared, Xms (advertised A)` is a
parseable assertion target, and the timing figure was added for exactly this kind of measurement.

## Order I'd suggest

B first — it needs no hardware, covers the cases that keep being wrong, and its output is deterministic.
A only after establishing what PM3 15693 simulation can actually do. C last, and only if A pans out.
