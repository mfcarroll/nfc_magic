"""Thin serial-storage bridge to a connected Flipper Zero.

Reusable plumbing for the Goal-2 cross-read harness (flipper_ground_truth.py). Wraps the Flipper
firmware's own storage helper (scripts/flipper/storage.py) so we can, over the USB CLI:

  - list the top-level files in /ext/nfc   (to diff before/after and spot the new read-back file)
  - upload a source .nfc                    (to write onto a magic card with the nfc_magic app)
  - read back / download a .nfc             (the file the stock NFC app just saved)

Requires pyserial, which the system python here does NOT have. Run under the tools venv:

    python3 -m venv tools/.venv && tools/.venv/bin/pip install pyserial
    tools/.venv/bin/python tools/flipper_ground_truth.py ...

The Flipper firmware scripts are located automatically next to this repo
(../Momentum-Firmware/scripts, then ../Momentum-Firmware-slix/scripts); override with --scripts
or the MOMENTUM_SCRIPTS env var. Only one CLI client can hold the port at a time -- close qFlipper
and any other serial session first.
"""

import logging
import os
import sys

NFC_DIR = "/ext/nfc"


class FlipperBridgeError(RuntimeError):
    pass


def find_scripts(explicit=None):
    """Locate the Flipper firmware `scripts` dir that contains flipper/storage.py."""
    cands = []
    if explicit:
        cands.append(explicit)
    if os.environ.get("MOMENTUM_SCRIPTS"):
        cands.append(os.environ["MOMENTUM_SCRIPTS"])
    here = os.path.dirname(os.path.abspath(__file__))
    siblings = os.path.dirname(os.path.dirname(here))  # .../rfid
    for name in ("Momentum-Firmware", "Momentum-Firmware-slix"):
        cands.append(os.path.join(siblings, name, "scripts"))
    for c in cands:
        if c and os.path.isfile(os.path.join(c, "flipper", "storage.py")):
            return c
    raise FlipperBridgeError(
        "could not find the Flipper firmware scripts (flipper/storage.py).\n"
        "  Pass --scripts PATH or set MOMENTUM_SCRIPTS. Tried:\n    " + "\n    ".join(cands))


def _import_storage(scripts):
    if scripts not in sys.path:
        sys.path.insert(0, scripts)
    try:
        from flipper.storage import FlipperStorage
        from flipper.utils.cdc import resolve_port
    except ImportError as e:
        if "serial" in str(e).lower():
            raise FlipperBridgeError(
                "pyserial is not installed for this python.\n"
                "  Run under the tools venv:\n"
                "    python3 -m venv tools/.venv && tools/.venv/bin/pip install pyserial\n"
                "    tools/.venv/bin/python tools/flipper_ground_truth.py ...") from e
        raise FlipperBridgeError(
            "failed to import Flipper storage helpers from %s: %s" % (scripts, e)) from e
    return FlipperStorage, resolve_port


class Flipper:
    """Context manager around FlipperStorage. `with Flipper() as f: f.list_nfc() ...`"""

    def __init__(self, port="auto", scripts=None, logger=None):
        self.scripts = find_scripts(scripts)
        self._FS, self._resolve = _import_storage(self.scripts)
        self.log = logger or logging.getLogger("flipper_bridge")
        if port in (None, "auto"):
            self.port = self._resolve(self.log, "auto")
        else:
            self.port = port
        if not self.port:
            raise FlipperBridgeError(
                "no Flipper found on USB. Is it connected and unlocked, and is qFlipper / any other\n"
                "  serial session closed? You can force a port with --port /dev/cu.usbmodemflip_XXXX.")
        self.storage = None

    def __enter__(self):
        self.storage = self._FS(self.port)
        self.storage.start()
        return self

    def __exit__(self, *exc):
        try:
            if self.storage:
                self.storage.stop()
        except Exception:
            pass
        self.storage = False

    # ---- operations (all paths are bare names inside /ext/nfc unless absolute) ----
    def _abs(self, name):
        return name if name.startswith("/") else NFC_DIR + "/" + name

    def list_nfc(self):
        """Top-level (non-recursive) filenames in /ext/nfc. Returns a set of names (no subdirs)."""
        path, _dirs, nondirs = next(self.storage.walk(NFC_DIR))
        return set(nondirs)

    def upload(self, local_path, remote_name=None):
        """Send a local .nfc to /ext/nfc. Returns the remote name used."""
        remote_name = remote_name or os.path.basename(local_path)
        self.storage.send_file(local_path, self._abs(remote_name))
        return remote_name

    def read_text(self, remote_name):
        """Return the contents of a Flipper .nfc as text."""
        data = self.storage.read_file(self._abs(remote_name))
        return bytes(data).decode("utf-8", "replace")

    def download(self, remote_name, local_path):
        """Copy a Flipper .nfc to a local path."""
        self.storage.receive_file(self._abs(remote_name), local_path)
        return local_path

    def exists(self, remote_name):
        return self.storage.exist_file(self._abs(remote_name))

    def remove(self, remote_name):
        self.storage.remove(self._abs(remote_name))


def new_files(before, after, ignore=()):
    """Names present in `after` but not `before`, minus any in `ignore` (e.g. the file we uploaded)."""
    return sorted((set(after) - set(before)) - set(ignore))
