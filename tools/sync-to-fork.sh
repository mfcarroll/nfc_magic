#!/usr/bin/env bash
#
# Sync the ISO15693 app code + CHANGELOG from THIS dev repo (the source of truth) into an
# all-the-plugins checkout's base_pack/nfc_magic, applying the dev -> upstream transform. Run this
# whenever the PR needs refreshing, so you only ever edit here.
#
# Usage:  tools/sync-to-fork.sh [path-to-all-the-plugins]     (default: ../all-the-plugins)
#         SYNC_SRC=<ref> tools/sync-to-fork.sh ...            (default source ref: iso15693-dev)
#
# It:
#   - overlays the app files (magic/ scenes/ views/ helpers/ assets/ nfc_magic_app*.{c,h}
#     CHANGELOG.md) from the committed SRC ref into base_pack/nfc_magic
#   - rewrites the dev icon-header include (nfc_magic_dev_icons.h -> nfc_magic_icons.h), the only
#     code reference to the dev appid
#   - keeps the target's fap_version in step with dev (leaves its appid / name / description alone,
#     since the upstream app keeps those)
#   - never touches the dev-only tools/ .notes/ ONBOARDING.md .vscode/
#   - does NOT commit or push -- it prints `git status` so you review, then commit in the fork.
#
# Note: syncs from a COMMITTED ref, so commit in this repo first. It overlays (never deletes), so a
# file removed/renamed here must be handled by hand in the fork.
set -euo pipefail

SRC="${SYNC_SRC:-iso15693-dev}"
ATP="${1:-../all-the-plugins}"

REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "$REPO_ROOT"

DEST="$ATP/base_pack/nfc_magic"
[ -d "$DEST" ] || { echo "error: '$DEST' not found -- pass the all-the-plugins path as arg 1"; exit 1; }
git rev-parse --verify "$SRC" >/dev/null 2>&1 || { echo "error: source ref '$SRC' not found"; exit 1; }

# portable in-place sed (GNU vs BSD/macOS)
sedi() { if sed --version >/dev/null 2>&1; then sed -i "$@"; else sed -i '' "$@"; fi; }

APP_PATHS="magic scenes views helpers assets nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h CHANGELOG.md"

echo "Syncing '$SRC' -> $DEST"

# overlay the app files from the committed source ref
# shellcheck disable=SC2086
git archive "$SRC" -- $APP_PATHS | tar -x -C "$DEST"

# the dev appid generates nfc_magic_dev_icons.h; the upstream appid (nfc_magic) generates
# nfc_magic_icons.h -- rewrite it back in every file that includes it
for f in $(grep -rl "nfc_magic_dev_icons.h" "$DEST" 2>/dev/null || true); do
  [ -n "$f" ] && sedi 's/nfc_magic_dev_icons\.h/nfc_magic_icons.h/g' "$f"
done

# keep fap_version aligned with dev (do not touch appid/name/description)
DEV_VER="$(git show "$SRC:application.fam" | sed -n 's/.*fap_version="\([^"]*\)".*/\1/p' | head -1)"
[ -n "$DEV_VER" ] && sedi "s/fap_version=\"[^\"]*\"/fap_version=\"$DEV_VER\"/" "$DEST/application.fam"

# safety: no dev identity should survive in the synced code
if grep -rn "nfc_magic_dev" "$DEST" >/dev/null 2>&1; then
  echo "WARNING: 'nfc_magic_dev' still present after sync (review these):"
  grep -rn "nfc_magic_dev" "$DEST"
fi

echo
echo "Done. Changes in the fork (review, then commit):"
( cd "$DEST" && git status --short . )
