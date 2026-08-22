// Host-side tests for the shared UID formatter.
//
// Four screens print a UID and, until this pass, each did it with its own loop -- three of them
// implementing the same two-group layout in two different spellings (`i == 4` before the byte, `i == 3`
// after it). The layout is not cosmetic: the write-fail and confirm screens share their line with other
// text, and a UID that overruns 128px is clipped, which on those screens is the only route back to a
// card that has stopped answering to the UID its owner knows.
//
// So the widths are asserted, not just the strings.

#include "../../magic/protocols/iso15693/iso15693_info.c" // NOLINT -- see README.md

#include <stdio.h>
#include <string.h>

static int tests_run;
static int tests_failed;
static const char* current_test;
static bool current_failed;

#define CHECK_STR(got, expected)                               \
    do {                                                       \
        if(strcmp((got), (expected)) != 0) {                   \
            printf(                                            \
                "  FAIL %s:%d  got \"%s\", expected \"%s\"\n", \
                __FILE__,                                      \
                __LINE__,                                      \
                (got),                                         \
                (expected));                                   \
            current_failed = true;                             \
        }                                                      \
    } while(0)

#define CHECK_LEN(got, expected)                               \
    do {                                                       \
        if((size_t)(got) != (size_t)(expected)) {              \
            printf(                                            \
                "  FAIL %s:%d  got %zu chars, expected %zu\n", \
                __FILE__,                                      \
                __LINE__,                                      \
                (size_t)(got),                                 \
                (size_t)(expected));                           \
            current_failed = true;                             \
        }                                                      \
    } while(0)

static void begin(const char* name) {
    current_test = name;
    current_failed = false;
    tests_run++;
}

static void end(void) {
    if(current_failed) {
        tests_failed++;
        printf("FAILED: %s\n", current_test);
    } else {
        printf("  ok  %s\n", current_test);
    }
}

static const uint8_t sample_uid[ISO15693_3_UID_SIZE] =
    {0xE0, 0x04, 0x01, 0x50, 0x12, 0x34, 0x56, 0x78};

static void test_spaced_separates_every_byte(void) {
    begin("the spaced form separates every byte");
    FuriString* out = furi_string_alloc();
    iso15693_info_cat_uid(out, sample_uid, Iso15693UidFormatSpaced);
    CHECK_STR(furi_string_get_cstr(out), "E0 04 01 50 12 34 56 78");
    furi_string_free(out);
    end();
}

static void test_grouped_breaks_once_in_the_middle(void) {
    begin("the grouped form breaks once, between bytes 3 and 4");
    FuriString* out = furi_string_alloc();
    iso15693_info_cat_uid(out, sample_uid, Iso15693UidFormatGrouped);
    CHECK_STR(furi_string_get_cstr(out), "E0040150 12345678");
    furi_string_free(out);
    end();
}

static void test_neither_form_leads_with_a_separator(void) {
    // The Info screen appends after "UID: ", so a leading space would double it. Every caller owns its
    // own label, which is only safe while this holds.
    begin("neither form leads with a separator");
    FuriString* labelled = furi_string_alloc();
    furi_string_set_str(labelled, "UID: ");
    iso15693_info_cat_uid(labelled, sample_uid, Iso15693UidFormatSpaced);
    CHECK_STR(furi_string_get_cstr(labelled), "UID: E0 04 01 50 12 34 56 78");
    FuriString* grouped = furi_string_alloc();
    furi_string_set_str(grouped, "Now reads:\n");
    iso15693_info_cat_uid(grouped, sample_uid, Iso15693UidFormatGrouped);
    CHECK_STR(furi_string_get_cstr(grouped), "Now reads:\nE0040150 12345678");
    furi_string_free(labelled);
    furi_string_free(grouped);
    end();
}

static void test_the_widths_are_what_the_layout_assumes(void) {
    // 17 against 23. The Info screen has a line to itself and can afford the spaced form; the write-fail
    // and confirm screens cannot, which is the whole reason two policies exist.
    begin("grouped is 17 chars and spaced is 23, which is why both exist");
    FuriString* grouped = furi_string_alloc();
    FuriString* spaced = furi_string_alloc();
    iso15693_info_cat_uid(grouped, sample_uid, Iso15693UidFormatGrouped);
    iso15693_info_cat_uid(spaced, sample_uid, Iso15693UidFormatSpaced);
    CHECK_LEN(furi_string_size(grouped), 17);
    CHECK_LEN(furi_string_size(spaced), 23);
    furi_string_free(grouped);
    furi_string_free(spaced);
    end();
}

static void test_a_zero_uid_still_renders_every_byte(void) {
    // A wipe that moved a gen1 UID can leave the card answering zeros, and that is exactly when the
    // user needs to read it. "%02X" must pad rather than collapse.
    begin("an all-zero UID renders as eight zero bytes, not as nothing");
    const uint8_t zero_uid[ISO15693_3_UID_SIZE] = {0};
    FuriString* out = furi_string_alloc();
    iso15693_info_cat_uid(out, zero_uid, Iso15693UidFormatGrouped);
    CHECK_STR(furi_string_get_cstr(out), "00000000 00000000");
    furi_string_free(out);
    end();
}

int main(void) {
    printf("iso15693 uid formatter\n");
    test_spaced_separates_every_byte();
    test_grouped_breaks_once_in_the_middle();
    test_neither_form_leads_with_a_separator();
    test_the_widths_are_what_the_layout_assumes();
    test_a_zero_uid_still_renders_every_byte();

    printf("\n%d run, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
