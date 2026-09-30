# tools/fold — putting each change into the sync point that owns it

Dev-only; nothing here syncs to the fork. Every fold this round (review 2, review 3, folds 7-9) was run
with these, from a scratch directory `FOLD_WORK` that holds the rules and the maps -- never the repo.

1. **Rules**, `$FOLD_WORK/rulesN.py`: `RULES`, `WIDTH = {}`, `RANGE_BASE` (a commit before every owner).
   A rule is `dict(id=, path=, frm=<owner>, kind="sub", old=, new=[, until=])`, or `kind="add",
   content=` for a new file. Every rule must match EXACTLY ONCE at every commit from its owner to HEAD
   (or to `until`); text that varies across the range needs one rule per variant. Owner = the commit
   that wrote the line, or made it false. `rules9.py` and earlier are gone with their session; the
   pattern is in each fold's note in `.notes/`.
2. `FOLD_WORK=... RULES=rulesN python3 tools/fold/engine.py dry` (every rule OK), then `tip` (the tip
   diff you meant), then `map`.
3. On a throwaway branch: `git filter-branch -f --index-filter "sh $PWD/tools/fold/ifilter.sh" --
   <RANGE_BASE>..<branch>`, with FOLD_WORK and RULES exported. `--commit-filter` with `skip_commit`
   drops a commit a fold empties; `--msg-filter` rewrites one message. Not `--prune-empty`: the range
   holds commits that are empty on purpose.
4. `python3 tools/fold/verify_pairs.py $FOLD_WORK/indexmap-rulesN.txt <RANGE_BASE> <old> <new>` -- 0
   problems -- then keep a `wip-pre-foldN` branch, adopt with `git reset --keep`, and rename the fork
   messages to the new anchors from the shamap.
5. `FOLD_WORK=... zsh tools/fold/check_anchor.sh <id> <sha>` on every rewritten commit that touches
   shipped paths or the tests: conflict markers, host tests, and all 71 app units under the firmware's
   -Werror flags (reads the Momentum build's compile_commands.json, so do not run fbt meanwhile).
6. A test replay into a `--shared` clone of the fork whose origin is a bare clone pinned at the pushed
   base, then `python3 tools/fold/churn.py <clone> <base> <commits...>`, then the real replay.
