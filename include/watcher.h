#ifndef WATCHER_H
#define WATCHER_H

#include "path.h"

typedef struct {
  int wd;
  VPath dirpath;
} WatchDescriptor;

void watchdog_init(const VPath *path, int fd, ZDeque *wds);
void watchdog_destroy(int fd, ZDeque *wds);

#endif
