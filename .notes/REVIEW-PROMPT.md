# The review prompt — run this in fresh context, before anything is posted

Written 2026-09-26 after three review passes each found defects the previous pass had been
explicitly asked to look for. **The checklist was incomplete, not ignored.** Every item below is a
defect that actually shipped into a draft in this round, and most were caught by a human read after a
pass that had already been told to check for "that kind of thing". State the classes explicitly;
"do a thorough review" does not work.

## How to run it

Single-threaded, no fan-out. Work through the passes in order and **report findings rather than
silently fixing**, except where a fix is mechanical and provable. For each finding give the site, the
class, and what it should say instead.

**Re-derive everything at the moment of reading.** Do not trust a number, a SHA, a count or a claim
because an earlier note asserts it. The recurring failure is a sentence that was true when written.

## Pass 1 — mechanical, and they are cheap

    (cd tools/hosttest && make clean && make)            168 tests, 0 failed
    clang-format --dry-run --Werror on every shipped .c/.h
    tools/check-writing.py comments <every shipped .c/.h>
    tools/check-writing.py forkmsg .notes/pr-round-15/fork-messages
    tools/check-drafts.py .notes/pr-round-15/reply.md
    FBT_NO_SYNC=1 ./fbt fap_nfc_magic_dev                 in ../Momentum-Firmware, zero warnings
    tools/comment-only.py <each shipped commit>           prove what claims to be comment-only
    tools/replay-to-fork.sh <msgdir>                      its own verification is a real check

A gate reporting clean means only that its patterns did not match. Four of the classes below have
no checker at all.

## Pass 2 — the classes that actually bite

Each one shipped into a draft this round.

1. **A CLAIM CORRECTED IN ONE PLACE AND LEFT IN ITS TWIN.** The most common defect here, six
   instances this round. Sweeping a correction across artifacts is the edit that causes it. **For
   every claim you change, grep the exact phrase across shipped source, CHANGELOG, the reply, every
   fork message and the squash message before moving on.**

2. **A FORK MESSAGE THAT CONTRADICTS ITS OWN TREE.** A fork message is prose about the tree at ITS
   anchor, not about the round. "Measured on three chips" is wrong in a message whose own commit says
   two. **Check each message's factual claims against `git show <its anchor>:<file>`.**

3. **NARRATION, IN TWO FLAVOURS, AND THE SECOND HAS NO CHECKER.**
   - *ours*: "I did not expect", "we built it to see what it would cost", "checks its answer NOW".
     `check-drafts.py` catches these in reply payloads only.
   - *the app's*: "the clone now converts itself to a gen1 clone from that point, stops writing the
     file into the registers, and reports the run as the gen1 clone it turned out to be." That is the
     run's internal sequence. A release note says where the card ENDS UP.
   Test: does this sentence describe the end state, or the route to it?

4. **AN OPEN ITEM NOBODY OPENED.** "#255 is still yours to call as its own PR" — he was never asked,
   in any round. **Worse than a stale claim**, because a stale one was true once and re-deriving
   catches it. Check every outward claim about what HE owes, decided or asked against what was
   actually posted: `gh api repos/xMasterX/all-the-plugins/issues/250/comments`.

5. **A NUMBER TRUE OF MOST CASES.** "A target differing in seven of its eight bytes" is six for one
   of three cards. State the PROPERTY the argument needs, not the count.

6. **A HEDGE HARDENED INTO A CLAIM.** "possibly still locked" became "he called it locked"; "armed
   gen1 card" scoped a hazard to a state nothing can detect. Ask what the evidence is, then whether
   the sentence claims more.

7. **CHIP VERSUS FAMILY.** Name the chip, say how many CARDS and how many CHIPS. The title of a
   measurement note is where this leaks first.

8. **SCOPE OF THE ARTIFACT.** A release note says what the user gets. A commit message says what its
   diff does. The reply says disposition, correction, finding. Evidence for a claim nobody disputes
   is padding wherever it lands.

9. **INTRA-PUSH CHURN.** Text written by one fork commit and rewritten by a later one in the same
   push. Measure it; it was 110 lines and is 91. The cause is structural: a running inventory of what
   is NOT yet done, and release notes written per commit rather than at the last commit that changes
   the behaviour.

10. **SYNC-POINT COVERAGE.** A shipped commit above the last anchor reaches nobody.
    `replay-to-fork.sh` refuses up front now, but check it rather than assuming.

## Pass 3 — the read-through, which no checker replaces

The eight questions in `.notes/WRITING-RULES.md`, against every paragraph of the reply. The ones that
have actually caught things: can he SEE what I am comparing to; can he ACT on this; does the
conclusion follow from the sentence before; is this question still open; am I telling him something
he told me; would this still be true if the round moved; is this a disposition, a correction or a
finding; does it duplicate something whose home is elsewhere.

`[👤]` marks mfcarroll's own paragraphs. They are posted as-is, they are not yours to edit, and the
attribution and narration rules do not apply to them.

## What must NOT be re-opened

`.notes/NEXT-SESSION.md` "WHAT TONIGHT SETTLED" and the DO-NOT-WIPE on `slix2-gold-30mm`.

## Hard rules

Never push the PR branch and never post anything without an explicit go-ahead, every time.
