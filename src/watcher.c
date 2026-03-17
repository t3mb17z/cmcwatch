#define _GNU_SOURCE
#include <linux/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>

#include "watcher.h"
#include "path.h"
#include "zds/deque.h"
#include "zds/stack.h"

/**
 * Watch for source and subdirectories recursively
 *
 * It returns the state of the operation
 */
void watchdog_init(const VPath *root, int fd, ZDeque *wds, watchdog_cb cb) {

  ZStack stack;
  ZStack_init(&stack, 4096, sizeof(VPath));
  ZStack_push(&stack, root);

  VPath path, full_path;
  while(!ZStack_empty(&stack)) {
    VPath_raw(&path, 32);
    ZStack_pop(&stack, &path);
    char *cpath = VPath_to_cstr(&path);
    DIR *dir = opendir(cpath);
    if(dir == NULL) {
      free(cpath);
      continue;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
      if(entry->d_type == DT_DIR) {
        if(strcmp(entry->d_name, ".") == 0 ||
          strcmp(entry->d_name, "..") == 0)
          continue;
        VPath tmp = VPath_from_cstr(entry->d_name);
        VPath_raw(&full_path, 32);
        VPath_append(&full_path, &path);
        VPath_append(&full_path, &tmp);
        VPath_destroy(&tmp);
        ZStack_push(&stack, &full_path);

        WatchDescriptor tmpwd;
        char *ctmp = VPath_to_cstr(&full_path);
        int wd = inotify_add_watch(fd, ctmp, IN_CLOSE_WRITE);
        tmpwd.wd = wd;
        VPath_raw(&tmpwd.dirpath, VPath_name_count(&full_path));
        VPath_append(&tmpwd.dirpath, &full_path);

        if(cb != NULL)
          cb(ctmp);
        free(ctmp);

        VPath_destroy(&full_path);

        ZDeque_push_back(wds, &tmpwd);
      }
    }

    VPath_destroy(&path);
    closedir(dir);
    free(cpath), cpath = NULL;
  }
}

void watchdog_destroy(int fd, ZDeque *wds) {
  int wd;
  for(size_t i = 0; i < ZDeque_len(wds); i++) {
    ZDeque_at(wds, i, &wd);
    inotify_rm_watch(fd, wd);
  }

  ZDeque_destroy(wds);
}
