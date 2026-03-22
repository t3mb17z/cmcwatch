#define _POSIX_C_SOURCE 200809L
#include "zds/vec.h"
#include <linux/stat.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <poll.h>

#include "fstools.h"
#include "path.h"
#include "watcher.h"
#include "zds/deque.h"
#include "vsignal.h"

#define BUF_LEN (10 * (sizeof(struct inotify_event) + 256))

#define MAX_BUF 4096

ZVecResult WatchDescriptor_drop(void *elem) {
  WatchDescriptor *wd = elem;
  VPath_destroy(&wd->dirpath);
  wd->wd = -1;
  return ZVEC_OK;
}

ZVecResult WatchDescriptor_copy(void *dest, const void *src) {
  WatchDescriptor *destination = dest;
  const WatchDescriptor *source = src;
  VPath_copy(&destination->dirpath, &source->dirpath);
  destination->wd = source->wd;
  return ZVEC_OK;
}

void print(const char *str) {
  printf("Found '%s' and added to watchdog!\n", str);
}

extern bool sig_handle(void);

int main(int argc, char *argv[]) {

  if (argc < 2 || strcmp(argv[1], "--help") == 0) {
    fprintf(stderr,
    "Usage %s <src_dir> <dst_dir> <uid> <gid>\n"
    "  Example:\n"
    "  %s dist test 10277 1023\n",
    argv[0], argv[0]);
    return 1;
  }

  if (!sig_handle()) {
    puts("No SIGINT handle available");
    return 1;
  }

  VPath source = VPath_from_cstr(argv[1]);
  VPath destination = VPath_from_cstr(argv[2]);
  char *tmp = VPath_to_cstr(&source);
  if (access(tmp, F_OK) != 0) {
    fprintf(stderr, "Source directory '%s' doesn't exists\n", tmp);
    return 2;
  }
  free(tmp), tmp = NULL;

  tmp = VPath_to_cstr(&destination);
  if (access(tmp, F_OK) != 0) {
    fprintf(stderr, "Destination directory '%s' doesn't exists\n", tmp);
    return 2;
  }
  free(tmp), tmp = NULL;

  char *buf;
  char *uid, *gid;
  if (argc > 3) {
    uid = strdup(argv[3]);
    gid = strdup(argv[4]);
  } else {
    uid = strdup("10269");
    gid = strdup("1015");
  }

  int iuid = strtol(uid, &buf, 10);
  if (*buf != '\0') {
    puts("Invalid number at UID");
    return 1;
  }

  int igid = strtol(gid, &buf, 10);
  if (*buf != '\0') {
    puts("Invalod number at GID");
    return 1;
  }
  free(uid), free(gid);
  buf = NULL;

  int fd = inotify_init();
  if (fd < 0) {
    return 1;
  }

  ZDeque wds;
  ZDeque_init(
    &wds, 4096,
    sizeof(WatchDescriptor),
    WatchDescriptor_copy,
    WatchDescriptor_drop
  );
  watchdog_init(&source, fd, &wds, print);

  VPath file_path, dest_file, src_file, dest_dir;

  tmp = VPath_to_cstr(&source);
  printf("Watching: '%s' -> ", tmp);
  free(tmp), tmp = NULL;
  tmp = VPath_to_cstr(&destination);
  printf("'%s'\n", tmp);
  free(tmp), tmp = NULL;

  struct pollfd pfd = {
    .fd = fd,
    .events = POLLIN,
  };

  char buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event = NULL;
  ssize_t size = 0;
  
  while (is_running()) {
    int ret = poll(&pfd, 1, 500);

    if (ret > 0) {
      size = read(fd, buffer, BUF_LEN);
    } else {
      continue;
    }

    if (size < 0) {
      break;
    }

    for (char *ptr = buffer; ptr < buffer + size; ptr += sizeof(struct inotify_event) + event->len) {
      event = (const struct inotify_event *)ptr;
      for (size_t i = 0; i < ZDeque_len(&wds); i++) {
        const WatchDescriptor *wd = ZDeque_get(&wds, i);
        if (event->wd != wd->wd) continue;
        VPath_raw(&src_file, 32);
        VPath_raw(&file_path, 32);
        VPath_raw(&dest_dir, 32);
        VPath_raw(&dest_file, 32);

        VPath_destroy(&src_file);
        VPath_destroy(&file_path);
        VPath_destroy(&dest_dir);
        VPath_destroy(&dest_file);

        (void)iuid, (void)igid;
      }
    }
  }

  VPath_destroy(&source);
  VPath_destroy(&destination);
  watchdog_destroy(fd, &wds);
  close(fd);

  return 0;
}
