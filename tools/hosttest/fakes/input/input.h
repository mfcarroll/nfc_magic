#pragma once
// Host-test fake. Enumerator list copied from applications/services/input/input.h (Momentum 87.15) --
// the widget callback receives one of these and the scenes act only on InputTypeShort.
#include <furi.h>

typedef enum {
    InputTypePress,
    InputTypeRelease,
    InputTypeShort,
    InputTypeLong,
    InputTypeRepeat,
    InputTypeMAX,
} InputType;
