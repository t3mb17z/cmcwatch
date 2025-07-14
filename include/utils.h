#ifndef UTILS_H
#define UTILS_H

typedef struct {
  int wd;
  char *dirpath;
} WatchDescriptor;

int watchdog(const char *path, int fd, WatchDescriptor ***wd, int *count);
int copy(const char *src, const char *dest);

#endif
