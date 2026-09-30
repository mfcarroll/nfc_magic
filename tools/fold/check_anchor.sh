#!/bin/zsh
# check_anchor.sh <NN> <sha>: conflict markers, host tests, and every app unit compiled with the
# firmware's own flags (-Werror included) against that commit's tree.
NN=$1; SHA=$2
S="${FOLD_WORK:?set FOLD_WORK to a scratch directory}"
mkdir -p "$S/wt"
W=$S/wt/$NN
DEV=/Users/Shared/code/personal/rfid/nfc_magic_dev
FW=/Users/Shared/code/personal/rfid/Momentum-Firmware
git -C $DEV worktree remove --force $W 2>/dev/null
git -C $DEV worktree add -q --detach $W $SHA || exit 1
markers=$(grep -rn -E '^(<<<<<<<|>>>>>>>|\|\|\|\|\|\|\|) ' $W/magic $W/scenes $W/views $W/helpers $W/*.c $W/*.h $W/CHANGELOG.md 2>/dev/null | wc -l | tr -d ' ')
tests=$(cd $W/tools/hosttest && make clean >/dev/null 2>&1; make 2>&1; echo "MAKE_RC=$?")
echo "$tests" > $S/wt/$NN.tests.log
summary=$(echo "$tests" | awk '/ run, [0-9]+ failed/{r+=$1; f+=$3} /^MAKE_RC=/{rc=substr($0,9)} END{printf "%d run, %d failed, make rc=%s", r, f, rc}')
python3 - "$W" "$FW" > $S/wt/$NN.compile.log 2>&1 <<'PY'
import json, subprocess, sys, shlex
W, FW = sys.argv[1], sys.argv[2]
cc = json.load(open(f'{FW}/build/f7-firmware-C/compile_commands.json'))
app = [e for e in cc if e.get('file', '').startswith('applications_user/nfc_magic_dev/')]
ok = bad = 0
for e in app:
    args = e['arguments'] if 'arguments' in e else shlex.split(e['command'])
    out = []
    skip = False
    for i, a in enumerate(args):
        if skip: skip = False; continue
        if a == '-o': out += ['-o', '/dev/null']; skip = True; continue
        out.append(a.replace('applications_user/nfc_magic_dev/', W + '/').replace('applications_user/nfc_magic_dev', W))
    r = subprocess.run(out, cwd=FW, capture_output=True, text=True)
    if r.returncode or r.stderr.strip():
        bad += 1; print('FAIL', e['file']); print(r.stderr[:3000])
    else: ok += 1
print(f'compiled {ok} ok, {bad} failed')
PY
echo "$NN $SHA markers=$markers | tests: $summary | $(tail -1 $S/wt/$NN.compile.log)"
