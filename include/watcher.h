#ifndef WATCHER_H
#define WATCHER_H

#include "path.h"

typedef struct {
  int wd;
  VPath dirpath;
} WatchDescriptor;

typedef void (*watchdog_cb)(const char *str);

void watchdog_init(const VPath *root, int fd, ZDeque *wds, watchdog_cb cb);
void watchdog_destroy(int fd, ZDeque *wds);

#endif
