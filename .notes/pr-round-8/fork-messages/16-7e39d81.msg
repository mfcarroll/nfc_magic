NFC Magic ISO15693: stop citing a test file this repo does not contain

nfc_magic_scene_iso15693_partial_details.c pointed at test_write_fail_scene.c,
which is not in this repository: not in the worktree the PR builds, not in git
ls-files, never added on any branch. A reader will go looking for it, and "a third
round-trip on this line is now a red test" does not hold for anyone reading here.

Swept for others -- scenes, magic, views, helpers, the root sources and the
CHANGELOG, for both test_*.c and hosttest. This was the only one, so the exposure
was one line.

What replaces it keeps the part of the clause that works without the file: this
boundary has been got backwards twice, which is why the derivation is written out
rather than left as a bare `<`. That is a warning the next editor can act on. "A
test guards it" was not, here.

Contributing the harness is a separate question and a separate PR; it is not a
reason to leave a dangling pointer in shipped code meanwhile. If it lands, the
citation comes back with it.

Comment-only, net +1 line.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
