#pragma once
// Host-test fake. The enumerator list is COPIED VERBATIM from lib/nfc/protocols/nfc_protocol.h
// (Momentum 87.15) rather than trimmed to what this app uses.
//
// It was previously a two-member subset declared in fakes/nfc/nfc_poller.h, which put
// NfcProtocolIso15693_3 at 1 where the firmware has it at 4. Nothing compares these numerically -- the
// pollers all test the symbol -- so no test was wrong because of it. But a fake that quietly disagrees
// with the firmware about a value is the thing that makes the NEXT test wrong, so it now matches.
#include <furi.h>

typedef enum {
    NfcProtocolIso14443_3a,
    NfcProtocolIso14443_3b,
    NfcProtocolIso14443_4a,
    NfcProtocolIso14443_4b,
    NfcProtocolIso15693_3,
    NfcProtocolFelica,
    NfcProtocolMfUltralight,
    NfcProtocolMfClassic,
    NfcProtocolMfPlus,
    NfcProtocolMfDesfire,
    NfcProtocolSlix,
    NfcProtocolSt25tb,
    NfcProtocolNtag4xx,
    NfcProtocolType4Tag,
    NfcProtocolEmv,

    NfcProtocolNum,
    NfcProtocolInvalid,
} NfcProtocol;
