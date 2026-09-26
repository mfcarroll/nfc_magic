#!/bin/sh
# Read every address in the 8-bit block space and keep the output.
#
# Exists because the two things that look like they already do this do not. `hf 15 dump` sweeps the
# ADVERTISED count and stops, and on a card whose advertised count is a setting rather than a
# capacity that leaves the region a write can reach with no record. probe_capacity binary-searches
# for the edge, so it neither visits every block nor keeps what it read.
#
# Reads only. Nothing here writes.
#
#   tools/sweep-read-all.sh /dev/cu.usbmodemiceman1 out.txt [LAST_BLOCK] [BATCH]
#
# The pm3 port is exclusive: close the interactive client first or this will not connect.
#
# BATCHED, because pm3 TRUNCATES A LONG -c STRING WITHOUT SAYING SO. A single 256-command line is
# silently cut -- the first run of this stopped at block 116 and the log looked complete, since every
# command it did run succeeded. A sweep that quietly covers half the space is worse than one that
# fails, so the batch size stays well under the limit and each chunk is appended and counted.
PORT="${1:?usage: sweep-read-all.sh <port> <outfile> [last-block, default 255] [batch, default 40]}"
OUT="${2:?usage: sweep-read-all.sh <port> <outfile> [last-block, default 255] [batch, default 40]}"
LAST="${3:-255}"
BATCH="${4:-40}"

: > "$OUT"
B=0
while [ "$B" -le "$LAST" ]; do
    END=$((B + BATCH - 1))
    [ "$END" -gt "$LAST" ] && END="$LAST"
    CMDS=""
    I="$B"
    while [ "$I" -le "$END" ]; do
        CMDS="${CMDS}hf 15 rdbl -b $I;"
        I=$((I + 1))
    done
    printf '  blocks %d..%d\n' "$B" "$END"
    pm3 -p "$PORT" -c "$CMDS" >> "$OUT" 2>&1
    B=$((END + 1))
done

GOT=$(grep -c '^\[=\] #' "$OUT" 2>/dev/null || echo 0)
WANT=$((LAST + 1))
echo "answered $GOT of $WANT"
[ "$GOT" -eq "$WANT" ] || echo "  WARNING: $((WANT - GOT)) block(s) missing -- do not treat this as a full sweep"
