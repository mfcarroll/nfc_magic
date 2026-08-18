#pragma once
// Host-test stub. MfUltralightType is an ENUM in the firmware and is stored by value in structs the app
// header pulls in, so it needs to be a complete type rather than an opaque struct. Nothing in the code
// under test reads it, so the underlying width is all that matters -- if a test ever needs a specific
// variant, copy the real enumerator list in rather than guessing at it here.
#include <furi.h>

typedef struct MfUltralightData MfUltralightData;
typedef uint8_t MfUltralightType;
