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
#   PM3=/path/to/pm3 tools/sweep-read-all.sh ...
#
# The pm3 port is exclusive: close the interactive client first or this will not connect.
#
# BATCHED, because pm3 TRUNCATES A LONG -c STRING WITHOUT SAYING SO. A single 256-command line is
# silently cut -- the first run of this stopped at block 116 and the log looked complete, since every
# command it did run succeeded.
#
# AND IT CHECKS THE FIRST BATCH BEFORE RUNNING THE REST. The first version did not: `pm3` is a shell
# ALIAS in the author's interactive shell and aliases do not exist in scripts, so every batch failed
# with "command not found" while the script cheerfully worked through all seven and then reported a
# malformed count. A sweep that fails silently is the exact thing this file was written to prevent.
set -u

PORT="${1:?usage: sweep-read-all.sh <port> <outfile> [last-block, default 255] [batch, default 40]}"
OUT="${2:?usage: sweep-read-all.sh <port> <outfile> [last-block, default 255] [batch, default 40]}"
LAST="${3:-255}"
BATCH="${4:-40}"

# $PM3, then the sibling checkout, then PATH. Resolved ONCE and checked, rather than discovered
# batch by batch in the output file.
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
PM3_BIN="${PM3:-}"
[ -z "$PM3_BIN" ] && [ -x "$HERE/../proxmark3/pm3" ] && PM3_BIN="$HERE/../proxmark3/pm3"
[ -z "$PM3_BIN" ] && PM3_BIN=$(command -v pm3 2>/dev/null)
if [ -z "$PM3_BIN" ] || [ ! -x "$PM3_BIN" ]; then
    echo "pm3 not found. It is probably a shell alias, which scripts do not inherit." >&2
    echo "Pass it explicitly:  PM3=/path/to/proxmark3/pm3 $0 $PORT $OUT" >&2
    exit 1
fi
echo "using $PM3_BIN"

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
    "$PM3_BIN" -p "$PORT" -c "$CMDS" >> "$OUT" 2>&1

    # Fail on the FIRST batch rather than at the end. A wrong port, a busy port, a missing binary or
    # a card off the antenna all look identical after 256 silent failures.
    if [ "$B" -eq 0 ]; then
        FIRST=$(grep -c '^\[=\] #' "$OUT" || true)
        if [ "${FIRST:-0}" -eq 0 ]; then
            echo "first batch read NOTHING -- stopping. See $OUT for why." >&2
            echo "  common causes: the interactive pm3 client still holds $PORT; no card on the antenna." >&2
            exit 1
        fi
    fi
    B=$((END + 1))
done

# grep -c prints 0 AND exits 1 when there are no matches, so `|| echo 0` appends a SECOND zero and
# the comparison below then fails on "0\n0". `|| true` is the whole fix.
GOT=$(grep -c '^\[=\] #' "$OUT" || true)
WANT=$((LAST + 1))
echo "answered ${GOT:-0} of $WANT"
[ "${GOT:-0}" -eq "$WANT" ] || echo "  WARNING: blocks missing -- do not treat this as a full sweep" >&2
