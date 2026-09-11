NFC Magic ISO15693: stop citing a test file this repo does not contain

nfc_magic_scene_iso15693_partial_details.c pointed at test_write_fail_scene.c,
which is dev-only: not in the worktree the PR builds, not in git ls-files, never
added on any branch. You are right that a reader will go looking for it, and
that "a third round-trip on this line is now a red test" does not hold for anyone
reading this repository.

Swept for others -- scenes, magic, views, helpers, the root sources and the
CHANGELOG, for both test_*.c and hosttest. This was the only one, so the exposure
was one line.

One correction, since you raised it as a possibility: the citation is NOT also
wrong against our own harness. You noticed the commit that added the line named a
test FUNCTION, test_cut_at_the_claim_reads_as_past_it, and inferred the filename
might be off. It is not -- that function is at test_write_fail_scene.c:349, with
its runner call at :543. The file was right; only its absence from this repo was
the problem.

What replaces it keeps the part of the clause that works without the file: this
boundary has been got backwards twice, by both of us, which is why the derivation
is written out rather than left as a bare `<`. That is a warning the next editor
can act on. "A test guards it" was not, here.

Contributing the harness -- the larger ask in the body of your review -- is a
separate question and a separate delta; it is not a reason to leave a dangling
pointer in shipped code meanwhile. If it lands, the citation comes back with it.

Comment-only, net +1 line.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
