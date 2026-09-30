#!/bin/sh
# The index filter for a fold: writes each mapped blob into the commit being rewritten. filter-branch
# runs it from its own temp directory, so pass this script's absolute path, and export FOLD_WORK and
# RULES -- the map is $FOLD_WORK/indexmap-$RULES.txt, written by `engine.py map`.
grep "^$GIT_COMMIT " "$FOLD_WORK/indexmap-$RULES.txt" | while read c mode blob path; do
  git update-index --add --cacheinfo "$mode,$blob,$path" || exit 1
done
