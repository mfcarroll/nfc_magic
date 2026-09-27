#!/usr/bin/env python3
"""Drive the address-enforcement measurement on one card, end to end.

    tools/enforce-bench.py <label>            one card on the antenna
    tools/enforce-bench.py <label> --block 8 --port /dev/cu.usbmodemiceman1

WHAT IT MEASURES, and why the earlier controls could not. Five of six cards on this shelf have
"enforces the address" from the card not ANSWERING a mis-addressed write. That was the same thing as
not writing until 2026-09-26, when the gen2 backdoor with OPTION set reported failure and moved the
UID anyway. So silence is measured; a non-write is inferred. This reads the block back with data
that DIFFERS from what is in it, BEFORE any restore, which is the only way the two come apart.

The right-address write goes AFTER the wrong-address one: it proves the frame shape was good, so a
silence at that step was the ADDRESS rather than a malformed frame (BENCH-RULE 2 -- a control that
cannot fail is not a control).

BENCH-RULE 0: the pm3 port is exclusive. Close the interactive client first or every step fails
identically to a card that is not there. This checks the FIRST step and stops (BENCH-RULE 0b).

Nothing is written outside <block>, which defaults to 8, and the original contents are restored and
confirmed. The flag probe writes the block's OWN current value back, so it cannot change anything
whether it is accepted or refused.
"""
import argparse, os, re, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_PM3 = os.path.normpath(os.path.join(HERE, "..", "..", "proxmark3", "pm3"))


def run(pm3, port, cmds, log):
    line = ";".join(cmds)
    p = subprocess.run([pm3, "-p", port, "-c", line], capture_output=True, text=True, timeout=120)
    out = p.stdout + p.stderr
    log.append("$ pm3 -c '%s'\n%s" % (line, out))
    return out


def uid_of(text):
    m = re.search(r"UID[.\s]*:?\s*((?:[0-9A-Fa-f]{2}[ ]){7}[0-9A-Fa-f]{2})", text)
    return m.group(1).strip() if m else None


def block_of(text):
    m = re.search(r"^\[=\]\s*((?:[0-9A-Fa-f]{2} ){3}[0-9A-Fa-f]{2})\s*\|", text, re.M)
    return m.group(1).replace(" ", "").upper() if m else None


def raw_reply(text):
    if re.search(r"command failed|⚠", text):
        return None
    m = re.search(r"\[\+\]\s*\(\d+\)\s*((?:[0-9A-Fa-f]{2}\s*)+)", text)
    return m.group(1).split() if m else None


def wire(uid_hex):
    b = bytes.fromhex(uid_hex)
    return "".join("%02X" % x for x in reversed(b))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("label")
    ap.add_argument("--block", type=int, default=8)
    ap.add_argument("--port", default="/dev/cu.usbmodemiceman1")
    ap.add_argument("--pm3", default=os.environ.get("PM3", DEFAULT_PM3))
    a = ap.parse_args()

    if not os.access(a.pm3, os.X_OK):
        sys.exit("pm3 not executable at %s -- it is a shell alias, which scripts do not inherit.\n"
                 "Pass PM3=/path/to/proxmark3/pm3" % a.pm3)
    if not os.path.exists(a.port):
        sys.exit("no such port: %s" % a.port)

    log, blk = [], a.block
    print("== %s, block %d ==" % (a.label, blk))

    # 0. the card, and what the block holds. FIRST unit of work, checked before anything else runs.
    out = run(a.pm3, a.port, ["hf 15 reader", "hf 15 rdbl -b %d" % blk], log)
    uid = uid_of(out)
    if not uid:
        sys.exit("no UID. Card on the antenna? Interactive client closed? (BENCH-RULE 0)\n" + out[-500:])
    holds = block_of(out)
    if holds is None:
        sys.exit("could not read block %d\n%s" % (blk, out[-500:]))
    addr, bad = wire(uid.replace(" ", "")), None
    b = bytearray(bytes.fromhex(uid.replace(" ", "")))
    b[0] ^= 0x01
    bad = wire(b.hex())
    probe = "55667788" if holds != "55667788" else "A1B2C3D4"
    print("   UID        %s" % uid)
    print("   block %-4d %s   (restore value)" % (blk, holds))

    # 1. flag probe -- write the block's OWN value back. Cannot change anything either way.
    out = run(a.pm3, a.port, ["hf 15 raw -ackw -d 2221%s%02X%s" % (addr, blk, holds)], log)
    r = raw_reply(out)
    if r and len(r) >= 2 and r[0] == "01" and r[1] == "03":
        flags, why = 0x62, "wants OPTION (error 0x03)"
    elif r:
        flags, why = 0x22, "no OPTION needed (%s)" % " ".join(r[:3])
    else:
        sys.exit("flag probe got no answer -- stopping rather than guessing\n" + out[-500:])
    print("   flags      %02X   %s" % (flags, why))

    # 2. WRONG address, data that differs from what is in there. 3. read it BEFORE restoring.
    out = run(a.pm3, a.port,
              ["hf 15 raw -ckw  -d %02X21%s%02X%s" % (flags, bad, blk, probe),
               "hf 15 rdbl -b %d" % blk], log)
    wrong_reply, after_wrong = raw_reply(out), block_of(out)
    print("   wrong addr %s" % ("SILENT" if wrong_reply is None else " ".join(wrong_reply[:3])))
    print("   block now  %s" % after_wrong)

    # 4. RIGHT address, same data -- the control, and it must be able to fail.
    out = run(a.pm3, a.port,
              ["hf 15 raw -ackw -d %02X21%s%02X%s" % (flags, addr, blk, probe),
               "hf 15 rdbl -b %d" % blk], log)
    right_reply, after_right = raw_reply(out), block_of(out)
    print("   right addr %s" % ("SILENT" if right_reply is None else " ".join(right_reply[:3])))
    print("   block now  %s" % after_right)

    # 5. restore and confirm.
    out = run(a.pm3, a.port,
              ["hf 15 raw -ackw -d %02X21%s%02X%s" % (flags, addr, blk, holds),
               "hf 15 rdbl -b %d" % blk], log)
    final = block_of(out)
    print("   restored   %s %s" % (final, "OK" if final == holds else "*** NOT RESTORED ***"))

    ok_enforced = (after_wrong == holds)
    ok_control = (right_reply is not None and after_right == probe)
    print()
    if not ok_control:
        print("   VERDICT: INCONCLUSIVE -- the right-address write did not land, so the silence")
        print("            at the wrong address proves nothing about the address.")
    elif ok_enforced:
        print("   VERDICT: ENFORCED, MEASURED. The mis-addressed write changed nothing, and the")
        print("            same frame with the right address did. Not an inference.")
    else:
        print("   VERDICT: *** THE MIS-ADDRESSED WRITE LANDED *** -- block went %s -> %s."
              % (holds, after_wrong))
        print("            Stop. The round's safety claim rests on this not happening.")

    path = os.path.join(HERE, "..", ".notes", "pr-round-15",
                        "enforce-%s.txt" % re.sub(r"[^A-Za-z0-9_-]", "-", a.label))
    with open(path, "w") as f:
        f.write("\n".join(log))
    print("   transcript %s" % os.path.normpath(path))
    return 0 if (ok_control and ok_enforced and final == holds) else 1


if __name__ == "__main__":
    sys.exit(main())
