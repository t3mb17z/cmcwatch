#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#include "vsignal.h"

volatile sig_atomic_t running = 1;

bool is_running(void) {
    return running;
}

static inline void int_handler(int signl) {
    if(signl == SIGINT) {
        const char msg[] = "\rTerminating watchdog\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
        running = 0;
    }
}

bool sig_handle(void) {
    struct sigaction act = { 0 };
    act.sa_handler = int_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    sigfillset(&act.sa_mask);

    if(sigaction(SIGINT, &act, NULL) == -1)
        return false;

    return true;
}
