#!/usr/bin/env bash
# Replay a round onto the PR fork: one fork commit per dev commit.
#
# sync-to-fork.sh transfers CONTENT, not history -- it deliberately does not commit. This does the
# other half: for each prepared message it syncs that dev commit and commits it on the fork, so the
# fork keeps the one-decision-per-commit shape the reviewer asked for in round 5.
#
# Usage:  tools/replay-to-fork.sh <message-dir> [fork-path]
#   message-dir  holds NN-<dev sha>.msg, one per fork commit, in replay order (see
#                .notes/pr-round-7/fork-messages/README.md for how those are prepared). An optional
#                NN-<dev sha>.date beside a message holds that commit's author date, for a
#                correction that rebuilds commits already on the branch and keeps their dates.
#   fork-path    default ../all-the-plugins
#
#   BASE=<ref>   reset the fork to this instead of origin/<branch>. Only for a CORRECTION that
#                replaces commits already pushed: the push is then a force-push, and the last line
#                says which lease to use. Never needed for an ordinary round.
#   PARTIAL=1    the message dir covers only part of dev's shipped history -- a correction to an
#                earlier round, with later rounds' shipped commits above it on dev. Those are then
#                reported instead of refused, and the replay is verified against its own last anchor.
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
BASE_REF="${BASE:-origin/$BRANCH}"
PARTIAL="${PARTIAL:-0}"

echo "dev  : $DEV @ $(git rev-parse --short HEAD)"
echo "fork : $FORK on $BRANCH, replaying onto $BASE_REF"
echo

MSGS=("$MSGDIR"/[0-9][0-9]-*.msg)
ANCHORS=()
for msg in "${MSGS[@]}"; do sha="$(basename "$msg" .msg)"; ANCHORS+=("${sha#*-}"); done
FIRST_ANCHOR="${ANCHORS[0]}"
LAST_ANCHOR="${ANCHORS[${#ANCHORS[@]}-1]}"

# --- every anchor must be a commit on the branch being replayed ---
# Round 14 was pushed from anchors a same-day rewrite had already replaced. The old commits still
# resolved, so the sync ran happily from trees dev no longer held, and the PR got an older draft of
# three comments. A rewrite moves every anchor after it; a name that still resolves proves nothing.
for sha in "${ANCHORS[@]}"; do
  if ! git -C "$DEV" merge-base --is-ancestor "$sha" HEAD 2>/dev/null; then
    echo "refusing to replay -- anchor $sha is not a commit on this branch:"
    echo "  $(git -C "$DEV" log -1 --format='%h %s' "$sha" 2>/dev/null || echo "$sha does not resolve")"
    echo "  a rewrite has moved it; rename the .msg to the commit that now carries that subject"
    exit 1
  fi
done

# --- the writing gate, BEFORE any fork commit exists ---
# The rules in .notes/WRITING-RULES.md were written down and broken anyway, because nothing ran them.
# This is the point where that stops being cheap to ignore: after here, prose becomes fork history.
echo "writing gate:"
if ! python3 "$DEV/tools/check-writing.py" forkmsg "$MSGDIR"; then
  echo "  refusing to replay -- fix the fork messages, or say why and re-run with SKIP_WRITING_GATE=1"
  [ "${SKIP_WRITING_GATE:-0}" = "1" ] || exit 1
fi
# The comments gate reads the TREES THAT SHIP -- every sync point's -- not dev's working tree. The
# working tree is only the end state: round 14 passed this gate on HEAD while syncing anchors whose
# trees still held the phrasing it exists to stop. The last anchor is gated in full, since it is what
# the branch ends up holding. Each earlier one is gated on what it ADDS over the tree before it: an
# intermediate tree can carry a finding already on the branch that a later sync point removes, and
# that is a correction working, not a new fault.
# The path set is the one sync-to-fork.sh overlays from, filtered to source files and the release
# notes: a gate whose file set is narrower than the thing it gates reports clean for the files it
# never opened. The notes were exactly that until 2026-09-28 -- the gate had never read them.
SRC_PATHS=(magic scenes views helpers nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h CHANGELOG.md ISO15693.md)
GATE_TMP="$(mktemp -d)"
trap 'rm -rf "$GATE_TMP"' EXIT
# only the paths the ref has: a tree from before ISO15693.md existed would fail git archive
tree_of() {
  local p present=()
  mkdir -p "$GATE_TMP/$2"
  for p in "${SRC_PATHS[@]}"; do git -C "$DEV" cat-file -e "$1:$p" 2>/dev/null && present+=("$p"); done
  git -C "$DEV" archive "$1" -- "${present[@]}" | tar -x -C "$GATE_TMP/$2"
}
tree_of "$FIRST_ANCHOR^" base
prev=base
gate_failed=0
for i in "${!ANCHORS[@]}"; do
  sha="${ANCHORS[$i]}"
  tree_of "$sha" "t$i"
  if [ "$i" -eq $((${#ANCHORS[@]} - 1)) ]; then
    files=()
    while IFS= read -r f; do files+=("$f"); done < <(find "$GATE_TMP/t$i" \( -name '*.[ch]' -o -name CHANGELOG.md -o -name ISO15693.md \) | sort)
    out="$(python3 "$DEV/tools/check-writing.py" comments "${files[@]}")" || gate_failed=1
    echo "$out" | { grep -v ' warn ' || true; } | sed "s|$GATE_TMP/t$i/||; s|^|  $sha (last, in full): |"
  else
    out="$(python3 "$DEV/tools/check-writing.py" comments-new "$GATE_TMP/$prev" "$GATE_TMP/t$i")" || gate_failed=1
    echo "$out" | sed "s|^|  $sha (what it adds): |"
  fi
  prev="t$i"
done
if [ "$gate_failed" = 1 ]; then
  echo "  refusing to replay -- fix the comments, or say why and re-run with SKIP_WRITING_GATE=1"
  [ "${SKIP_WRITING_GATE:-0}" = "1" ] || exit 1
fi
if ls "$DEV"/.notes/pr-round-*/reply.md >/dev/null 2>&1; then
  python3 "$DEV/tools/check-writing.py" headings "$DEV"/.notes/pr-round-*/reply.md \
    "$DEV"/.notes/pr-round-*/thread-replies.md || true   # advisory: old rounds are history
fi
echo

# --- the anchor check, BEFORE anything is built ---
# A sync point syncs the TREE at its commit, so a shipped commit landing above the LAST anchor
# reaches nobody. The verification at the end already catches it, but only after eight commits have
# been built and signed -- and it has caught it three times, each time because the coverage was
# checked before the last edits rather than after. Fail here instead, and say which commits.
UNCOVERED=""
for c in $(git -C "$DEV" rev-list --reverse "$LAST_ANCHOR..HEAD"); do
  if git -C "$DEV" show --name-only --format= "$c" \
     | grep -qE '^(magic/|scenes/|views/|helpers/|assets/|nfc_magic_app|application\.fam|CHANGELOG\.md|ISO15693\.md)'; then
    UNCOVERED="$UNCOVERED  $(git -C "$DEV" log -1 --format='%h %s' "$c")
"
  fi
done
if [ -n "$UNCOVERED" ]; then
  if [ "$PARTIAL" = "1" ]; then
    echo "partial replay -- shipped commits above $LAST_ANCHOR stay for a later round:"
    printf '%s' "$UNCOVERED" | sed -n 1,3p
    echo "  ... $(printf '%s' "$UNCOVERED" | grep -c . || true) in all"
  else
    echo "refusing to replay -- shipped commits sit ABOVE the last sync point ($LAST_ANCHOR):"
    printf '%s' "$UNCOVERED"
    echo "  fix: rename $MSGDIR/$(basename "${MSGS[${#MSGS[@]}-1]}") to the newest of them"
    exit 1
  fi
fi
# --- every sync point must be a tree someone can check out ---
# An auto-resolved rebase can COMMIT a conflict marker: the tip stays clean because a later commit
# removes it, every test passes, and two intermediate commits ship a file that does not compile.
# Found exactly that in two sync points, 2026-09-26.
BADMARK=""
for sha in "${ANCHORS[@]}"; do
  for f in $(git -C "$DEV" ls-tree -r --name-only "$sha" -- magic scenes views helpers \
             nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h CHANGELOG.md ISO15693.md); do
    if git -C "$DEV" show "$sha:$f" 2>/dev/null | grep -qE '^(<<<<<<< |=======$|>>>>>>> )'; then
      BADMARK="$BADMARK  $sha $f
"
    fi
  done
done
if [ -n "$BADMARK" ]; then
  echo "refusing to replay -- a sync point's tree carries a conflict marker:"
  printf '%s' "$BADMARK"
  exit 1
fi
echo "marker check: every sync point's tree is clean"
if [ "$PARTIAL" = "1" ]; then
  echo "anchor check: every anchor is on this branch; later shipped commits are left for their round"
else
  echo "anchor check: every anchor is on this branch, and no shipped commit is left uncovered"
fi
echo

# --- reset to the base, discarding regenerable sync output ---
git -C "$FORK" fetch origin "$BRANCH" >/dev/null 2>&1
ORIGIN="$(git -C "$FORK" rev-parse "origin/$BRANCH")"
BASE_SHA="$(git -C "$FORK" rev-parse "$BASE_REF")"
echo "resetting fork to $BASE_REF ($(git -C "$FORK" rev-parse --short "$BASE_SHA"))"
git -C "$FORK" reset -q --hard "$BASE_SHA"
git -C "$FORK" clean -qfd base_pack/nfc_magic

# --- the base check: the fork must already hold dev's tree from just before the first anchor ---
# Each fork commit is the difference between the fork and dev's tree at its anchor, so fork commit
# 01 carries EVERYTHING the fork lacks, not just its own change. After round 14 the PR held an older
# draft of three comments, and round 15's 01 would have delivered the difference under a subject
# about addressing -- 79 lines nothing described. Nothing compared the pushed tree with dev's.
SYNC_SRC="$FIRST_ANCHOR^" "$DEV/tools/sync-to-fork.sh" "$FORK" >/dev/null
if ! git -C "$FORK" diff --quiet -- base_pack/nfc_magic || \
   [ -n "$(git -C "$FORK" status --porcelain base_pack/nfc_magic)" ]; then
  echo "refusing to replay -- the fork's base is not dev's tree from before the first anchor"
  echo "  ($(git -C "$DEV" log -1 --format='%h %s' "$FIRST_ANCHOR^")), so fork commit 01 would carry:"
  git -C "$FORK" status --short base_pack/nfc_magic | sed -n 's/^/    /;1,10p'
  git -C "$FORK" checkout -q -- base_pack/nfc_magic
  git -C "$FORK" clean -qfd base_pack/nfc_magic
  exit 1
fi
echo "base check: the fork already holds dev's tree from before the first anchor"
echo

# --- replay ---
n=0
for msg in "${MSGS[@]}"; do
  sha="$(basename "$msg" .msg)"; sha="${sha#*-}"
  subj="$(head -1 "$msg")"
  SYNC_SRC="$sha" "$DEV/tools/sync-to-fork.sh" "$FORK" >/dev/null
  git -C "$FORK" add -A base_pack/nfc_magic
  if git -C "$FORK" diff --cached --quiet; then
    echo "  -- $sha  EMPTY, skipped: $subj"
    continue
  fi
  files=$(git -C "$FORK" diff --cached --name-only | wc -l | tr -d ' ')
  if [ -f "${msg%.msg}.date" ]; then
    GIT_AUTHOR_DATE="$(cat "${msg%.msg}.date")" git -C "$FORK" commit -q -F "$msg"
  else
    git -C "$FORK" commit -q -F "$msg"
  fi
  n=$((n+1))
  printf "  %02d %s  %2d files  %s\n" "$n" "$sha" "$files" "${subj:20:64}"
done
echo
echo "$n fork commits created."

# --- verify ---
# Against the LAST ANCHOR's tree. For an ordinary round the anchor check has already made that the
# same tree as HEAD's; for a partial replay it is the only tree the replay claims to deliver.
echo
echo "=== verification ==="
failed=0
SYNC_SRC="$LAST_ANCHOR" "$DEV/tools/sync-to-fork.sh" "$FORK" >/dev/null
if git -C "$FORK" diff --quiet -- base_pack/nfc_magic && \
   [ -z "$(git -C "$FORK" status --porcelain base_pack/nfc_magic)" ]; then
  echo "  OK   fork tree == a single full sync from $LAST_ANCHOR, the last anchor (replay lost nothing)"
else
  echo "  FAIL fork tree differs from a full sync from $LAST_ANCHOR:"
  git -C "$FORK" status --short base_pack/nfc_magic | tail -8
  failed=1
fi
git -C "$FORK" checkout -q -- base_pack/nfc_magic 2>/dev/null || true
git -C "$FORK" clean -qfd base_pack/nfc_magic 2>/dev/null || true
if git -C "$FORK" merge-base --is-ancestor "$ORIGIN" HEAD; then
  echo "  OK   origin/$BRANCH is still an ancestor (a push will fast-forward, not force)"
elif [ -n "${BASE:-}" ]; then
  echo "  NOTE this REPLACES $(git -C "$FORK" rev-list --count "$BASE_SHA..$ORIGIN") commit(s) on origin/$BRANCH."
  echo "       Pushing it is a force-push, and needs its own explicit go-ahead. The lease pins what it"
  echo "       replaces:  git -C $FORK push --force-with-lease=$BRANCH:$ORIGIN origin $BRANCH"
else
  echo "  FAIL origin/$BRANCH is NOT an ancestor -- do not push"
  failed=1
fi
u=$(git -C "$FORK" log --format='%G?' "$BASE_SHA"..HEAD | grep -cv '^N$' || true)
t=$(git -C "$FORK" rev-list --count "$BASE_SHA"..HEAD)
echo "  sign  $u of $t new commits carry a signature"
echo
echo "NOT PUSHED. Review with:  git -C $FORK log --oneline $BASE_SHA..HEAD"
exit "$failed"
