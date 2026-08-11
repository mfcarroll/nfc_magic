#!/usr/bin/env bash
# POSTS TO GITHUB. Review pr-reply-body.md and pr-inline/*.md before running.
# Posts 7 threaded replies on PR #250, then the top-level comment.
set -euo pipefail
REPO=xMasterX/all-the-plugins
PR=250
cd "$(dirname "$0")"

read -rp "This posts 7 inline replies + 1 top-level comment to $REPO#$PR. Type 'post' to continue: " ok
[ "$ok" = "post" ] || { echo "aborted"; exit 1; }

for f in pr-inline/*.md; do
  id="$(basename "$f" .md)"
  echo "-> replying in thread $id"
  gh api --method POST "repos/$REPO/pulls/$PR/comments/$id/replies" -f body="$(cat "$f")" --jq '.html_url'
done

echo "-> top-level comment"
gh api --method POST "repos/$REPO/issues/$PR/comments" -f body="$(cat pr-reply-body.md)" --jq '.html_url'
echo "done"
