# Writing rules — run the gate before anything leaves this repo

These were learned expensively and were already written down, as prose, in a 264-line section of
`NEXT-SESSION.md` organised by **when we learned them**. We then broke two of them in one round.
They are here instead, organised by **which artifact you are about to write**, and the greppable
subset is a script rather than a paragraph.

    tools/check-writing.py comments <paths>    shipped source
    tools/check-writing.py forkmsg <dir>       fork messages
    tools/check-writing.py headings <files>    replies and threads
    tools/check-drafts.py <files>              replies and threads: refs, SHAs, line numbers

`replay-to-fork.sh` runs the first two and refuses to replay on a finding. Judgement rules cannot be
mechanised and are marked ⚖.

---

## Shipped comments

- ⚖ **Constraint, not argument.** Keep what breaks if someone undoes this. Drop the derivation that
  reached it. A comment earns its place only if it records something the code cannot show.
- **No history.** Not "used to", "previously", "before the cut". The reader is deciding whether they
  may change the line in front of them; how it got there is not their problem and goes stale
  silently. *Checked.*
- ⚖ **Say it once.** A rule stated in two places drifts. If two sites need it, one states it and the
  other points. A see-also to a sibling site means it wants one home.
- ⚖ **Scope every measurement.** "on the three chips tested", not "on gen1 silicon".
- **No dev SHAs.** They resolve here and nowhere he can see. *Checked.*
- **Blocks over 20 lines want a reason.** Not a failure; a prompt. *Warned.*

## Commit messages (dev)

- **Classify shipped vs dev-only, up front.** `tools/` and `.notes/` never reach the fork.
- ⚖ **State what changed and why it is right.** Reasoning is not process narration.
- **Say what is measured and what is reasoned**, especially for anything behavioural.

## Fork messages

Written fresh. Dev messages are NOT copyable — round 10's were, and he quoted one back.

- **Never describe a change absent from the diff.** Tests live in `tools/`, which does not sync, so
  no fork message cites one. *Checked.*
- **No dev SHAs.** *Checked.*
- ⚖ **One decision per commit**, and the sync points must not show him an error we then fix.

## Replies and threads

- ⚖ **Disposition, what he got wrong, what we found.** Nothing else.
- ⚖ **No internal process.** Not when we cut it, not that an earlier commit in the same push already
  removed it, not that we broke and restored something he never saw broken. "Cut" is a whole answer.
- **A heading states its SUBJECT, never a count or a status.** Four stale headings in four rounds,
  and "keep them in sync" does not work -- the body is what gets edited. "Your commits, and what the
  bench says" cannot rot; "two of the three benched" rots the moment the third is run. *Checked.*
- **No bare `:NNN`.** Cite behaviour, which does not drift. *Checked by check-drafts.*
- **Verify every SHA against the branch**, by subject, not existence.
- ⚖ **Correct what we told him that was wrong**, in one sentence, without the archaeology.

## The read-through — what to actually look for

The gate catches phrasings. It cannot catch a paragraph that is well-written, true, and has no
business being there. Every one of these got through the checkers and was caught by a human read, so
run them as questions against each paragraph before anything is posted.

1. **Can he see the thing I am comparing to?** "Yours is better than what I had written" compares his
   commit against a version that exists only in our unpushed history. He has never seen it and never
   will. This is internal process wearing the clothes of a concession, and it went through twice in
   one round. If the other half of the comparison is invisible to him, cut the comparison.
2. **Can he act on it?** A section telling him his commit broke a build he cannot see, whose fix is
   ours, in a tree that is not in this PR, asks nothing of him. Evidence belongs where it argues for
   something -- in that case, the other PR's own description.
3. **Does the conclusion follow from the sentence before it?** Two true paragraphs welded together do
   not make an argument. "The x=4 convention is deliberate" and "these twelve calls do not want a
   wrapper" are different claims; asserting the first answered the second made both unreadable.
4. **Is this question still open?** Phrasing carried forward from an earlier round outlives the
   decision it described. "The scoping call is yours" was true for three rounds and then was not.
5. **Am I telling him something he told me?** Restating the lesson from a defect HE found and
   analysed is not reporting, it is processing at him.
6. **Would this still be true if the round moved again?** The same rot that hits headings hits
   paragraphs; they are just harder to check.
7. **Is this paragraph carrying a disposition, a correction, or a finding?** If none of the three, it
   is reaching for something to say. Cut it.

The tell for most of them: the passage is about US -- what we tried, what we learned, how we feel
about it -- rather than about the code or the decision in front of him.

## The failure mode behind most of these

A claim that was true when written and went stale when the tree moved under it. The counter is not
care, it is re-deriving at the moment of publishing rather than trusting the sentence.
