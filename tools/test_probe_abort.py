#!/usr/bin/env python3
"""Declining a probe's confirmation prompt must not throw away what it already measured.

The defect this pins, found 2026-09-13 on `white-card`: Ctrl-C at the gen1 arming prompt raised
KeyboardInterrupt, which is not an Exception, so it escaped the campaign loop's per-probe guard and
skipped BOTH the result store and inventory_update. The V3 and gen2 results the same probe had just
taken were discarded -- manifest.json held `"cards": []`, the inventory still read "unclassified",
and the only surviving evidence was raw/*.txt. Answering "no" to a destructive prompt is the
cautious move; it must not cost the measurement.

Touches no hardware: the pm3 preflight and the probe itself are stubbed.

    python3 tools/test_probe_abort.py                  # the real tool
    python3 tools/test_probe_abort.py path/to/copy.py  # a mutant, to check this test can fail

To mutation-test: copy the tool, change `except KeyboardInterrupt:` in the campaign loop to some
exception that cannot fire, run this against the copy, and watch it report the empty classification.
"""
import importlib.util, io, json, os, sys, tempfile

SRC = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else
                      os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                   "iso15693_magic_probe.py"))
spec = importlib.util.spec_from_file_location("probe_under_test", SRC)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

tmp = tempfile.mkdtemp()
inv_path = os.path.join(tmp, "inv.json")
json.dump({"tags": {}, "updated": ""}, io.open(inv_path, "w"))
m.INVENTORY_MD = os.path.join(tmp, "inv.md")       # never touch the real .notes/ copy
m.pm3_probe = lambda *a, **k: (None, "")           # no Proxmark in this test


def fake_magictype(ctx):
    """V3 and gen2 measured, then the operator declines the gen1 arming frame."""
    res = {"v3_config_mode": False, "gen1_write": None, "gen2_write": None, "magic_method": None}
    ctx["partial"] = res        # the probe publishes as it goes, exactly as the real one does
    res["gen2_write"] = False   # measured, and the thing that used to be lost
    raise KeyboardInterrupt     # declined at the prompt


m.PROBES = dict(m.PROBES)
m.PROBES["magictype"] = (fake_magictype, True, "faked for the abort test")

sys.argv = ["probe", "--probes", "magictype", "--destructive", "--card", "testcard",
            "--pm3", "true", "--inventory", inv_path, "--out-dir", tmp]
sys.stdin = io.StringIO("\n" * 5)                  # the "place card on antenna" prompt
_real_stdout = sys.stdout
sys.stdout = io.StringIO()                         # the campaign banner is not the point
try:
    m.main()
except SystemExit as e:
    sys.stdout = _real_stdout
    print("the tool exited early: %s" % e)
finally:
    sys.stdout = _real_stdout

cls = json.load(io.open(inv_path))["tags"].get("testcard", {}).get("classification", {})
ok = cls.get("gen2_write") is False and cls.get("verdict") == "not gen2; gen1 untested"
print("classification recorded: %s" % (json.dumps(cls, sort_keys=True) or "{}"))
print("%s -- %s" % ("PASS" if ok else "FAIL",
                    "the declined run kept its measurements" if ok else "measurement lost"))
sys.exit(0 if ok else 1)
