#!/usr/bin/env python3
"""Is a commit comment-only? Decided by stripping comments with a REAL preprocessor.

    comment-only.py <rev>...          classify each revision
    comment-only.py --range A..B      classify every revision in the range

The C deletion pass promises "code bytes unchanged", and that promise needs a check that cannot
pass by accident. This one had a false-PASS the first time it was used inline: on macOS `gcc` is
clang, clang has no `-fpreprocessed`, so the command errored, emitted NOTHING for both sides, and
comparing two empty strings reported "identical". Same shape as a `grep -q` false pass and just as
convincing. So: locate a real GCC, verify it produces non-empty output, and fail loudly otherwise.
"""
import hashlib
import os
import subprocess
import sys
import tempfile

PACK = [
    "magic", "scenes", "views", "helpers", "assets", "nfc_magic_app.c", "nfc_magic_app.h",
    "nfc_magic_app_i.h", "CHANGELOG.md", "application.fam",
]


def find_gcc():
    for pat in ("../Momentum-Firmware-slix/toolchain/arm64-darwin/bin/arm-none-eabi-gcc",
                "../Momentum-Firmware/toolchain/arm64-darwin/bin/arm-none-eabi-gcc"):
        if os.path.isfile(pat) and os.access(pat, os.X_OK):
            return pat
    sys.exit("no real GCC found in the toolchains -- clang cannot do this (-fpreprocessed)")


GCC = find_gcc()


def strip(ref, path, tmp):
    """md5 of `path` at `ref` with comments removed, or None if absent."""
    blob = subprocess.run(["git", "show", f"{ref}:{path}"], capture_output=True)
    if blob.returncode:
        return None
    f = os.path.join(tmp, "u" + os.path.splitext(path)[1])
    with open(f, "wb") as fh:
        fh.write(blob.stdout)
    out = subprocess.run([GCC, "-fpreprocessed", "-dD", "-E", "-P", f], capture_output=True)
    if not out.stdout:
        sys.exit(f"preprocessor produced nothing for {path}@{ref} -- refusing to call that a match")
    return hashlib.md5(out.stdout).hexdigest()


def classify(rev, tmp):
    files = subprocess.run(
        ["git", "show", "--pretty=", "--name-only", rev, "--"] + PACK,
        capture_output=True, text=True, check=True).stdout.split()
    changed = []
    for f in files:
        if not (f.endswith(".c") or f.endswith(".h")):
            changed.append(f + " (not a source file)")
        elif strip(rev + "^", f, tmp) != strip(rev, f, tmp):
            changed.append(f)
    return files, changed


def main(argv):
    if argv and argv[0] == "--range":
        revs = subprocess.run(["git", "log", "--reverse", "--format=%h", argv[1], "--"] + PACK,
                              capture_output=True, text=True, check=True).stdout.split()
    else:
        revs = argv
    if not revs:
        sys.exit(__doc__)
    n_c = n_x = 0
    with tempfile.TemporaryDirectory() as tmp:
        for rev in revs:
            subj = subprocess.run(["git", "log", "-1", "--format=%s", rev],
                                  capture_output=True, text=True, check=True).stdout.strip()
            files, changed = classify(rev, tmp)
            if changed:
                n_x += 1
                print(f"  CODE     {rev}  {subj[:62]}")
                for c in changed:
                    print(f"             -> {c}")
            else:
                n_c += 1
                print(f"  comment  {rev}  {subj[:62]}")
    print(f"\n{n_c} comment-only, {n_x} touching code or non-source content (via {GCC.split('/')[-1]})")


if __name__ == "__main__":
    main(sys.argv[1:])
