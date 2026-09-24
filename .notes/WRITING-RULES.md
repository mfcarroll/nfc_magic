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

## The failure mode behind most of these

A claim that was true when written and went stale when the tree moved under it. The counter is not
care, it is re-deriving at the moment of publishing rather than trusting the sentence.
