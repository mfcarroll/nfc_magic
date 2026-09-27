# The round-14 correction — PR comment (DRAFT, NOT POSTED)

Posted right after the force-push, before anything from round 15. Post between the `~~~~` markers,
with the compare link resolving once the push lands (head `1d411dec`). TWO dots: with three,
GitHub diffs from the common ancestor, which after a force-push is mishamyte's head, and shows the
whole of round 14 -- code included -- under a sentence saying comment-only. Nothing here is `[👤]`; mfcarroll reads it before it goes.

~~~~
## The last push, as it was meant to be

Round 14's seven commits went up from a stale sync, carrying an older draft of three comments than the one intended. I have replaced them with the commits as they should have been. **Comment-only: no code, no strings, no behaviour, and your twelve are untouched.**

https://github.com/mfcarroll/all-the-plugins/compare/dbc11980561bb56115a19d0f8ce31b4fd4fa4502..1d411decf00f11f870f9a8c2fe01d9298237eafe

What is different:

- **The clock-cut, clamp and consent comments are shorter.** The clamp keeps its three cases and your point that the CFG one matters most, because those bytes outlive the run.
- **The budget parameter's reason** named #253, which is about Back on Gen2/Classic. It now points at the KNOWN OVERLAP note above `failures_are_top_tail`, which is where the idea is. The same fix is in that commit's message; the other six messages are unchanged.
- **An eighth commit** rewords four comments that described their own edit history into the rule each one states, meaning unchanged. Three of them are yours, from `0c5a7d69`, `e32e6242` and `413ab5a2`.

If you have the branch checked out: `git fetch && git reset --hard origin/nfc-magic-iso15693`.
~~~~
