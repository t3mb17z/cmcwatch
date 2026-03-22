#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>
#include <stdbool.h>

extern volatile sig_atomic_t running;

bool is_running(void);

#endif
