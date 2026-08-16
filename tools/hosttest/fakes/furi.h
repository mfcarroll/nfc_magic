// Host-side stand-in for the firmware's <furi.h>, for tools/hosttest only.
//
// Only the handful of primitives iso15693_poller.c actually calls. The clock is FAKE and
// deterministic -- see fake_tag.h -- because the sweep's wall-clock bound
// (ISO15693_POLLER_PASS_MAX_MS) is one of the behaviours no real card can be made to produce.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Advanced by the fake radio ops in fake_tag.c, so "elapsed time" is a function of how much work the
// sweep did rather than of how fast the host is. 1 tick == 1 ms.
extern uint32_t fake_tick;

#define furi_ms_to_ticks(ms) ((uint32_t)(ms))

static inline uint32_t furi_get_tick(void) {
    return fake_tick;
}

static inline uint32_t furi_kernel_get_tick_frequency(void) {
    return 1000U;
}

static inline void furi_delay_ms(uint32_t ms) {
    fake_tick += ms;
}

// The real macros abort the firmware. Aborting the test process is the equivalent: a tripped
// furi_check in the code under test is a test failure, not something to swallow.
#define furi_assert(...)                                                            \
    do {                                                                            \
        if(!(__VA_ARGS__)) {                                                        \
            fprintf(stderr, "furi_assert failed: %s:%d\n", __FILE__, __LINE__);     \
            abort();                                                                \
        }                                                                           \
    } while(0)

#define furi_check(...)                                                             \
    do {                                                                            \
        if(!(__VA_ARGS__)) {                                                        \
            fprintf(stderr, "furi_check failed: %s:%d\n", __FILE__, __LINE__);       \
            abort();                                                                \
        }                                                                           \
    } while(0)

// Log lines are captured rather than printed, so a test can assert on them -- the sweep's summary line
// ("wipe: N blocks attempted, M cleared, Xms") was added to be a parseable assertion target.
void fake_log(char level, const char* fmt, ...);

#define FURI_LOG_E(tag, fmt, ...) fake_log('E', fmt, ##__VA_ARGS__)
#define FURI_LOG_W(tag, fmt, ...) fake_log('W', fmt, ##__VA_ARGS__)
#define FURI_LOG_I(tag, fmt, ...) fake_log('I', fmt, ##__VA_ARGS__)
#define FURI_LOG_D(tag, fmt, ...) fake_log('D', fmt, ##__VA_ARGS__)
