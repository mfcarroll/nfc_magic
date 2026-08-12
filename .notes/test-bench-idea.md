# A bench that can stage the cases hardware can't

Raised 2026-08-11 while finishing round 3. **Option B is BUILT** — see [../tools/hosttest/README.md](../../tools/hosttest/README.md).
38 tests, and it found a real defect on its first run over the clone loop (the clock-cut capacity claim,
pushed as `f8eb8164`). Options A and C are still ideas.

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

## Option A — a tag simulator

**Better lead than the proxmark, found 2026-08-11:** the firmware's own unit tests already drive a poller
against a **listener** — `applications/debug/unit_tests/tests/nfc/nfc_test.c` allocates two `Nfc*`
instances and does `nfc_listener_alloc(listener, NfcProtocolSlix, slix_data)` + `nfc_listener_start(...)`,
then runs a SLIX poller against it. So ISO15693 tag simulation exists in-firmware, no second device
implied by the code.

Unverified, and check this first: whether one Flipper can be both poller and listener at once (it has a
single ST25R3916), or whether those tests assume something else. That is the gate on the whole idea.

Second gate: the stock `iso15693_3_listener` implements a *compliant* tag. It will not answer the magic
backdoor (`0xE0 0x09 ...`), and selective per-block refusal — block N answers reads but refuses writes,
blocks 20..63 answer nothing — is exactly what the host fakes do easily and a compliant listener does not
do at all. Extending the listener is firmware work.

What this route buys that `tools/hosttest` cannot: the radio layer. That is the one gap the host harness
is honest about not reaching.

Proxmark3 (`hf 15 sim`) remains a fallback rather than the first choice. Its 14443a emulation is mature;
the 15693 side has historically been thinner than `hf 15 sim -u <uid>`, and it needs the same selective
per-block behaviour. Establish the listener question before spending time here.

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

## Order

B is done and was the right call: no hardware, deterministic, and it caught a defect immediately.

What is left of B is `iso15693_poller_write_step` — the state machine around the three functions now
covered, which is where `uid_verified` false actually gets set, and the last testable item on the
reasoned-only list. Needs the poller callback driven rather than one function called.

A next, but only after answering the one-radio question above. C last, and only if A pans out — and note
the Flipper CLI needs a real TTY, so it needs a pty wrapper (`pexpect`), not plain pipes.
