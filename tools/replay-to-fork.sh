#!/usr/bin/env bash
# Replay a round onto the PR fork: one fork commit per dev commit.
#
# sync-to-fork.sh transfers CONTENT, not history -- it deliberately does not commit. This does the
# other half: for each prepared message it syncs that dev commit and commits it on the fork, so the
# fork keeps the one-decision-per-commit shape the reviewer asked for in round 5.
#
# Usage:  tools/replay-to-fork.sh <message-dir> [fork-path]
#   message-dir  holds NN-<dev sha>.msg, one per fork commit, in replay order (see
#                .notes/pr-round-7/fork-messages/README.md for how those are prepared)
#   fork-path    default ../all-the-plugins
#
# It NEVER pushes. It resets the fork to origin/<branch> first, so a stale local base cannot turn
# the next push into a force-push over live review threads.
set -euo pipefail

MSGDIR="${1:?usage: replay-to-fork.sh <message-dir> [fork-path]}"
# absolute -- every git call below runs with -C "$FORK", so a relative message path would resolve
# against the fork instead of here
MSGDIR="$(cd "$MSGDIR" && pwd)"
FORK="${2:-../all-the-plugins}"
DEV="$(git rev-parse --show-toplevel)"
FORK="$(cd "$FORK" && pwd)"
BRANCH="$(git -C "$FORK" branch --show-current)"

echo "dev  : $DEV @ $(git rev-parse --short HEAD)"
echo "fork : $FORK on $BRANCH"
echo

# --- the writing gate, BEFORE any fork commit exists ---
# The rules in .notes/WRITING-RULES.md were written down and broken anyway, because nothing ran them.
# This is the point where that stops being cheap to ignore: after here, prose becomes fork history.
echo "writing gate:"
if ! python3 "$DEV/tools/check-writing.py" forkmsg "$MSGDIR"; then
  echo "  refusing to replay -- fix the fork messages, or say why and re-run with SKIP_WRITING_GATE=1"
  [ "${SKIP_WRITING_GATE:-0}" = "1" ] || exit 1
fi
if ! python3 "$DEV/tools/check-writing.py" comments \
     $(git -C "$DEV" ls-files 'magic/**/*.c' 'magic/**/*.h' 'scenes/*.c' 'views/*.c' | sed "s|^|$DEV/|"); then
  echo "  refusing to replay -- fix the comments, or say why and re-run with SKIP_WRITING_GATE=1"
  [ "${SKIP_WRITING_GATE:-0}" = "1" ] || exit 1
fi
if ls "$DEV"/.notes/pr-round-*/reply.md >/dev/null 2>&1; then
  python3 "$DEV/tools/check-writing.py" headings "$DEV"/.notes/pr-round-*/reply.md \
    "$DEV"/.notes/pr-round-*/thread-replies.md || true   # advisory: old rounds are history
fi
echo

# --- reset to the pushed base, discarding regenerable sync output ---
git -C "$FORK" fetch origin "$BRANCH" >/dev/null 2>&1
BASE="$(git -C "$FORK" rev-parse "origin/$BRANCH")"
echo "resetting fork to origin/$BRANCH ($(git -C "$FORK" rev-parse --short "origin/$BRANCH"))"
git -C "$FORK" reset -q --hard "origin/$BRANCH"
git -C "$FORK" clean -qfd base_pack/nfc_magic
echo

# --- replay ---
n=0
for msg in "$MSGDIR"/[0-9][0-9]-*.msg; do
  sha="$(basename "$msg" .msg)"; sha="${sha#*-}"
  subj="$(head -1 "$msg")"
  SYNC_SRC="$sha" "$DEV/tools/sync-to-fork.sh" "$FORK" >/dev/null
  git -C "$FORK" add -A base_pack/nfc_magic
  if git -C "$FORK" diff --cached --quiet; then
    echo "  -- $sha  EMPTY, skipped: $subj"
    continue
  fi
  files=$(git -C "$FORK" diff --cached --name-only | wc -l | tr -d ' ')
  git -C "$FORK" commit -q -F "$msg"
  n=$((n+1))
  printf "  %02d %s  %2d files  %s\n" "$n" "$sha" "$files" "${subj:20:64}"
done
echo
echo "$n fork commits created."

# --- verify ---
echo
echo "=== verification ==="
SYNC_SRC=HEAD "$DEV/tools/sync-to-fork.sh" "$FORK" >/dev/null
if git -C "$FORK" diff --quiet -- base_pack/nfc_magic; then
  echo "  OK   fork tree == a single full sync from dev HEAD (replay lost nothing)"
else
  echo "  FAIL fork tree differs from a full sync from dev HEAD:"
  git -C "$FORK" diff --stat -- base_pack/nfc_magic | tail -8
  git -C "$FORK" checkout -q -- base_pack/nfc_magic
fi
git -C "$FORK" checkout -q -- base_pack/nfc_magic 2>/dev/null || true
if git -C "$FORK" merge-base --is-ancestor "$BASE" HEAD; then
  echo "  OK   origin/$BRANCH is still an ancestor (a push will fast-forward, not force)"
else
  echo "  FAIL origin/$BRANCH is NOT an ancestor -- do not push"
fi
u=$(git -C "$FORK" log --format='%G?' "$BASE"..HEAD | grep -cv '^N$' || true)
t=$(git -C "$FORK" rev-list --count "$BASE"..HEAD)
echo "  sign  $u of $t new commits carry a signature"
echo
echo "NOT PUSHED. Review with:  git -C $FORK log --oneline origin/$BRANCH..HEAD"
