#pragma once
// Host-test stub for the generated icon header. Only the symbols the code under test takes the address
// of need to exist; nothing renders, so an empty object is enough.
#include <furi.h>

// The real header exposes these as Icon objects the app takes the address of. Nothing renders here, so
// the type is completed as an empty-ish struct rather than left opaque -- an opaque const object cannot
// be defined in the fake.
typedef struct Icon {
    int unused;
} Icon;

extern const Icon I_WarningDolphinFlip_45x42;
