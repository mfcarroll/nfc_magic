NFC Magic ISO15693: is_wipe stops pretending, and two shared fields stop being labelled for one protocol

is_wipe was `uscuid_ul_is_wipe_mode || iso15693_wipe`, read once, and the ISO15693
branch overwrites the title anyway -- so the second term had no observable effect
and the variable's name promised more than it delivered. It is now what it is.

The title itself is unchanged. "(gen1/gen2 only)" does not claim the wipe detects a
generation -- it performs no magic detection at any point -- it tells the user which
cards to use it on, and the bolded "can brick a gen3 card" directly above the button
says what happens otherwise. Read together they are a restriction the user applies,
not a gate the app enforces, and the comment now says so rather than leaving the
next reader to work it out.

Then the two shared progress fields. nfc_magic_app_i.h labelled
write_progress_current and write_progress_total "USCUID-UL:", but the ISO15693 write
drives both, from blocks_done and blocks_total, through the same WorkerProgress
event. The two fields below them really are USCUID-UL's, so the header marks which
is which rather than mislabelling all four.

The progress handler's comment had the same gap, and it is the one that matters: it
said the event arrives "while the USCUID-UL poller advances page by page", one
statement above the is_wiping ternary that exists precisely because that stopped
being true.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
