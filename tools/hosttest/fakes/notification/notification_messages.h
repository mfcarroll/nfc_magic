#pragma once
// Host-test recorder. The scenes pick between two sequences and the choice is user-visible (a success
// chime over a partial result was a real round-4 finding), so which one was played is recorded.
#include <furi.h>

typedef struct NotificationApp NotificationApp;

// Complete type, not opaque: the fake defines two instances and the scenes compare their addresses.
typedef struct {
    int unused;
} NotificationSequence;

extern const NotificationSequence sequence_success;
extern const NotificationSequence sequence_error;

void notification_message(NotificationApp* app, const NotificationSequence* sequence);
