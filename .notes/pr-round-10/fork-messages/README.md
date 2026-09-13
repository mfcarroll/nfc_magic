
## A reference class worth adding to the checklist — 2026-09-13

**Our thread NUMBERING is dev-repo bookkeeping and must never reach him.** He numbers nothing: his
review refers to findings by `file:line` and by what they are. The numbers in
`pr-round-*/received/threads.md` are ours, assigned when the review was captured.

Four posted lines had leaked them — "the fix for threads 01/02", "from thread 01", "your thread 11",
"the same commit as thread 05". Each now names the finding instead, which is how he refers to them
and how a reader of the thread will recognise it.

Same class as a dev SHA and as a `tools/` path: an identifier that resolves on our side and nowhere
on his. `check-drafts.py` catches the first two and cannot catch this one, so it goes on the manual
sweep:

```bash
awk '/^~~~~$/{f=!f;next} f' .notes/pr-round-*/reply.md .notes/pr-round-*/thread-replies.md \
  | grep -niE "thread[s]? [0-9]"
```

## The width check was counting BYTES — 2026-09-13

`awk '{print length}'` on this machine returns **bytes**, not characters. Every em-dash, en-dash and
arrow in our prose is 3 bytes, so a line of 78 characters can measure 84+ and a width sweep
over-reports on exactly the text we write.

It is always conservative for a MAXIMUM — bytes >= characters, so nothing over-long can hide — which
is why it caused no harm here: the round-10 fork messages measure **zero over 84 characters**. But
it reported nine over-length lines in the reply when the real number was one, and a check that cries
wolf is a check people stop reading.

Use python for width from now on:

```bash
python3 -c "
import sys
for p in sys.argv[1:]:
    for i,l in enumerate(open(p),1):
        if i>1 and len(l.rstrip())>84: print(f'{p}:{i} = {len(l.rstrip())}')
" .notes/pr-round-*/fork-messages/*.msg
```
