#pragma once

// Whether `call` aborts. furi_check and furi_crash abort the process in the fakes, as they halt the
// firmware, so a test of one runs the call in a forked child and reads how the child ended. The child's
// stderr goes to /dev/null: the fake reports the tripped check there, and the run's output stays clean.

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

static inline bool aborts(void (*call)(void)) {
    fflush(NULL);
    const pid_t pid = fork();
    if(pid == 0) {
        if(!freopen("/dev/null", "w", stderr)) _exit(2);
        call();
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT;
}
