NFC Magic ISO15693: the survey's results are one struct

The clone's survey reports eleven fields: the residue it found, whether the card
holds more than it claims, and the block count and IC reference it goes on
reporting against the file's. Each was kept in four places -- declared in the
poller's state and again in Iso15693PollerResult, reset one field at a time at
the start of a run, and copied one field at a time into the result -- so a field
added to the survey had four places to go, and leaving it out of the reset or
the copy would still compile.

They are one struct now, Iso15693PollerSurvey, declared once in the header with
the comments that were on the result's fields, and embedded in both. A run
resets it with one memset, get_result hands it over with one assignment, and
the scenes read the same fields through `.survey`.

The flag saying whether the card's block count is a claim at all stays outside
it: it qualifies that count for the size note and was never part of the result.

No behaviour changes.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
