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
    clang-format --dry-run --Werror on every shipped .c/.h    95 files, 0 need formatting
      NOT the bare command -- it is not on PATH and this repo has no .clang-format, so the
      bare form reports ALL 95 as dirty. Use the toolchain's, with the firmware's style file:
      ../Momentum-Firmware/toolchain/current/bin/clang-format \
        --style=file:../Momentum-Firmware/.clang-format --dry-run --Werror <file>
    tools/check-writing.py comments <every shipped .c/.h>
    tools/check-writing.py forkmsg .notes/pr-round-15/fork-messages
    tools/check-drafts.py .notes/pr-round-15/reply.md
    FBT_NO_SYNC=1 ./fbt fap_nfc_magic_dev                 in ../Momentum-Firmware, zero warnings
      AFTER rm -rf build/f7-firmware-C/.extapps/nfc_magic_dev. SCons signs by content, so a build
      that is already up to date prints no CC line at all, and "zero warnings" then means nothing
      was compiled. Count the CC lines (71).
    tools/comment-only.py <each shipped commit>           prove what claims to be comment-only
    tools/check-stale-shas.py                             0 stale; notes citing dead commits
    tools/replay-to-fork.sh <msgdir> <throwaway clone>    its own verification is a real check
      NOT into ../all-the-plugins: it resets that fork, and the real replay waits on mfcarroll.
      git clone --shared the fork, point origin at its GitHub URL, commit.gpgsign false there.

    per sync point, not just the tip:  no conflict markers, compiles, passes its own tests

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

11. **A CHECK THAT ONLY LOOKS AT THE END STATE.** Bit this round twice, and it is the one that
    survives every other check because every other check agrees with it.
    - Two sync points shipped a file containing a committed `<<<<<<<` conflict marker. The TIP was
      clean, because a later commit removed those lines. 168 tests passed, clang-format passed, the
      FAP built, and the replay's own verification compares the FORK TREE AGAINST DEV HEAD -- which
      cannot see a broken intermediate commit by construction.
    - Shipped commits landed above the last sync point three separate times; the end-state diff
      caught it, but only after eight commits had been built and signed.

    **He checks out individual commits. Every sync point has to be a tree someone can build.** Both
    are pre-flight refusals in `replay-to-fork.sh` now, but the CLASS is wider than those two: when
    you verify something, ask what it compares, and whether an intermediate state could be wrong
    while the comparison still passes.

    Corollary, learned the same way: **resolving a run of rebase conflicts by script is how the
    marker got committed.** If a resolution is worth automating, its result is worth reading.

12. **A CONTROL THAT PROVES THE WEAKER CLAIM.** Six cards had "enforces the address" from the card
    not ANSWERING a mis-addressed write. Silence and non-write were the same thing here until a
    frame reported failure and wrote anyway. **Name what the control actually excludes, then check
    that is what the sentence claims.** Two ways it went wrong in one evening: the probe data was
    the same in both frames, so a landed write read identically to a refused one; and the block was
    restored before it was read, so the read confirmed the restore and nothing else. **A restore
    that runs before the measurement destroys the measurement.**

13. **AN ARGUMENT THAT COVERS THE WRONG POPULATION.** "0xE0 is proprietary, so a conforming tag
    rejects it on the command" — true, and the tags at risk are other MAGIC cards, which parse it
    exactly as the target does. Ask who the sentence is about, and whether they are the ones the
    hazard is about.

14. **EVIDENCE GATHERED BY A DIFFERENT METHOD FROM THE CONSTANTS IT IS COMPARED WITH.** A tag's
    config blocks were read with `hf 15 dump` and matched against values proxmark reads WITH the
    OPTION flag. It turned out identical, so nothing changed — but that was luck, and nobody had
    checked. When a comparison uses someone else's constants, use their method.

15. **A SCOPE RULE APPLIED IN THE WRONG DIRECTION.** The rule here is "do not count a card of
    unknown silicon as an identified chip". Applied backwards it became "three cards, one chip",
    when one of the three demonstrably differs from the other two. **Unidentified is not absent.**
    Count what is identified, say the rest separately, and never imply the rest is not there.

16. **"EVERY", "ONLY" AND "ALL", checked against the code rather than the sentence they answer.**
    Review 2 found three at one tip:
    - "Every other frame this app sends carries a UID": the SDK's reads, GET SYSTEM INFO and
      inventories go out unaddressed.
    - "Every write this app sends takes its flags from here": two lines under a comment saying the
      gen2 byte does not.
    - "the only chip that refuses anything here": every gen1 chip refuses 62/63.

17. **A MEASUREMENT GENERALIZED PAST ITS SAMPLE -- class 6's mirror.** Correcting "a card left
    armed" (too narrow) produced "those registers take a write with nothing in front of them", a law
    about gen1 silicon drawn from five cards of unknown history. State the sample ("every gen1 card
    measured took..."), then the conclusion a user needs ("a card's history cannot be known, so treat
    any gen1 card the sweep reaches as exposed").

18. **A PATTERN THE RULES NAME, STILL AT THE TIP.** WRITING-RULES named the running "what remains
    unaddressed" list as churn's cause and said nothing should hold it. The tip held it three times,
    and 07 re-added it in words that 08 retracted. When a rule names a pattern, grep the tip for that
    pattern's own words.

## Pass 3 — the read-through, which no checker replaces

The eight questions in `.notes/WRITING-RULES.md`, against every paragraph of the reply. The ones that
have actually caught things: can he SEE what I am comparing to; can he ACT on this; does the
conclusion follow from the sentence before; is this question still open; am I telling him something
he told me; would this still be true if the round moved; is this a disposition, a correction or a
finding; does it duplicate something whose home is elsewhere.

`[👤]` marks mfcarroll's own paragraphs. They are posted as-is, they are not yours to edit, and the
attribution and narration rules do not apply to them.

## Two habits that cost time rather than correctness

- **A note that cites a dev SHA is a claim that rots silently.** This round rebuilt five times.
  `tools/check-stale-shas.py` catches the unreachable ones; it cannot catch a SHA that still
  resolves and now names the wrong commit, which is what an anchor written into prose becomes.
  The `.msg` filenames and the fork README table are the only anchors anyone should read.
- **Rewrite a commit message before naming a file after its SHA.** 08's anchor moved three times in
  one evening and the file was renamed after each.

## What must NOT be re-opened

`.notes/NEXT-SESSION.md` "WHAT TONIGHT SETTLED" and the DO-NOT-WIPE on `slix2-gold-30mm`.

## Hard rules

Never push the PR branch and never post anything without an explicit go-ahead, every time.
