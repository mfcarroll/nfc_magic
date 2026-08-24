# DRAFT — follow-up comment for #255. NOT POSTED.

Target: https://github.com/xMasterX/all-the-plugins/issues/255 (our own issue, so a plain comment).

**Why this is owed.** Two reasons, and the second is the one mishamyte will check:

1. The issue's own text overclaims. It says "the mitigation already implemented is the only one
   available -- re-read the UID after the wipe behind a field power-cycle, and report a change as
   Partial rather than promising the UID survived." That is true except on the one path where it is
   most needed, and the issue does not say so.
2. He asked for exactly this case to be filed. Round 6, thread `3823546230`:
   *"Gen1, untestable without a card, so the same bucket as everything else gen1 - file it rather than
   fix it."* The reply said "Filed, not fixed, as you asked" -- but what actually happened was a code
   note, not a filing. #255 is the gen1-register-hazard bucket, so this belongs on it.

The code and CHANGELOG halves are already corrected on the branch (`65e741a`, `7570ef7`).

---

## Comment body — paste as-is (everything between the ~~~~ markers)

~~~~
Correcting this issue against its own code, because the mitigation it describes has a gap and the issue
states it unconditionally.

**The claim.** Above: "the mitigation already implemented is the only one available — re-read the UID
after the wipe behind a field power-cycle, and report a change as Partial rather than promising the UID
survived."

**The gap.** `iso15693_poller.c` short-circuits on `wiped == 0`:

```c
// If not a single block accepted the zero-write, nothing was wiped ...
if(wiped == 0) {
    iso15693_poller_report(instance, Iso15693PollerEventFail);
    return NfcCommandStop;
}
```

That returns before the `NfcCommandReset`, so on this path there is no power-cycle and no UID re-read at
all. The wipe reports "Wipe failed" and says nothing about the identity — not even that the check was
skipped. The "UID not re-checked" note has two homes and `NothingWiped` reaches neither: it is an inline
third line on the wipe-complete body (`write_fail.c`, gated on `uid_verified`), and a line in the
scroll view (`partial_details.c`), which needs a Details button `has_details` does not give this reason.

**Why it is not benign.** The justification the code gave itself was that no write landed, so the UID
cannot have moved. That does not hold. Reaching `wiped == 0` means every write was *refused*, but the
sweep still sent three WRITE BLOCKs each at blocks 56 and 57 before giving up, and `write_identity`'s own
comment records that a tag can apply a write without answering. On an armed gen1 card, blocks 56/57 ARE
the UID registers. So this path can move the UID, report "Wipe failed", never run the check, and never
say the check did not run — which is the one combination the rest of the wipe's reporting is built to
avoid.

Credit where it is due: @mishamyte found this during review of #250 and asked for it to be filed rather
than fixed, on the grounds that the `wiped == 0` short-circuit predates that PR and gen1 is untestable
without a card. Both still hold, so this is a correction to the issue rather than a new request.

**What has changed on the PR branch.** The two places that repeated the unqualified claim are fixed —
the comment at the short-circuit now states what skipping the check costs instead of asserting it costs
nothing, and the CHANGELOG's wipe entry carries a `Limit:` clause naming the case. The short-circuit
itself is untouched.

**What this means for the proposed fix.** It does not change the gen3 half. It does change the armed-gen1
half of the table above: "post-wipe UID re-check reports a change" should read *reports a change on every
path except a wipe that cleared nothing*. Two candidate fixes, neither in scope here:

- run the power-cycle and the UID re-read even when `wiped == 0`, which costs one reset and one inventory
  on a path that is already a failure; or
- leave the short-circuit and give `NothingWiped` a route to the "UID not re-checked" note, so the
  absence of the check is at least stated.

The first is the one that actually detects the hazard. The second only stops the report from implying an
answer it never got. Worth noting they are not alternatives — the first makes the second unnecessary.
~~~~

---

## Then, once posted

Update the citation in `iso15693_poller.c` at the `wiped == 0` comment: it currently says "the one path
where the mitigation #255 describes does not run at all", which is accurate but reads as if #255 records
the gap. Once this comment is up, #255 does record it, and no code change is needed. Check the wording
still reads correctly rather than editing reflexively.
