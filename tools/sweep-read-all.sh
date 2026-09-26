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
#   tools/sweep-read-all.sh /dev/cu.usbmodemiceman1 ~/slix2-fullsweep.txt [LAST_BLOCK]
#
# The pm3 port is exclusive: close the interactive client first or this will not connect.
PORT="${1:?usage: sweep-read-all.sh <port> <outfile> [last-block, default 255]}"
OUT="${2:?usage: sweep-read-all.sh <port> <outfile> [last-block, default 255]}"
LAST="${3:-255}"

CMDS=""
B=0
while [ "$B" -le "$LAST" ]; do
    CMDS="${CMDS}hf 15 rdbl -b $B;"
    B=$((B + 1))
done

echo "reading blocks 0..$LAST on $PORT -> $OUT"
pm3 -p "$PORT" -c "$CMDS" > "$OUT" 2>&1
echo "done: $(grep -c '^\[=\] #' "$OUT" 2>/dev/null || echo 0) blocks answered"
