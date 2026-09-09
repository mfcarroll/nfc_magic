NFC Magic ISO15693: the private struct stops repeating the public one

struct Iso15693Poller carries a reporting field for all but one member of
Iso15693PollerResult, and every one of them had its own paragraph saying what
the header's paragraph already said. Two parallel doc sets means every
correction has to land twice -- which is the mechanism Round 6 was mostly made
of, in its most literal form.

iso15693_poller_get_result is already the mapping: one assignment per field,
nothing inferred. So the struct now says that once, points at the header for
the semantics, and comments only what the header cannot know:

  - pass_truncated keeps the trap. Both passes set it, scene_write.c mode-gates
    on it for that reason, and a reader who takes it for wipe-only deletes the
    gate and lands a cut clone on the wipe-specific screen.
  - gen1_attempted keeps the precision point: it equals attempt_gen1, and
    "unconditionally" would be too strong because two paths return from Start
    before the frames go out.
  - clone_afi_failed / clone_dsfid_failed keep the two-into-one note, since
    get_result ORs them behind a mode gate.
  - clone_source keeps why it is not `data`.
  - uid_changed keeps the pointer to Iso15693WriteStateVerifyWipe.

Also dropped: "NOTE: the gen1 path is NOT hardware-validated -- see the PR
note." The caveat lives on both gen1 entry points in the header, where a caller
sees it; "the PR note" was a reference to something no reader of this file has.

Comment -37, code +0. With comments stripped the file is byte-identical.
101 host tests before and after.

