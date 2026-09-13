
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
