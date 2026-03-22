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

ZVecResult VPath_deep_copy(void *dest, const void *src) {
  const VPath *source = src;
  VPath *destination = dest;
  VPath_copy(destination, source);
  return ZVEC_OK;
}

ZVecResult VPath_drop(void *path) {
  VPath *vpath = path;
  VPath_destroy(vpath);
  return ZVEC_OK;
}

/**
 * Watch for source and subdirectories recursively
 *
 * It returns the state of the operation
 */
void watchdog_init(const VPath *root, int fd, ZDeque *wds, watchdog_cb cb) {

  if(root == NULL) return;
  char *croot = VPath_to_cstr(root);
  int root_wd = inotify_add_watch(fd, croot, IN_CLOSE_WRITE | IN_CREATE);
  WatchDescriptor struct_root_wd = { 0 };
  struct_root_wd.wd = root_wd;
  VPath_copy(&struct_root_wd.dirpath, root);
  ZDeque_push_back(wds, &struct_root_wd);
  VPath_destroy(&struct_root_wd.dirpath);
  if(cb != NULL)
    cb(croot);
  free(croot);

  ZStack stack;
  ZStack_init(
    &stack, 4096,
    sizeof(VPath),
    VPath_deep_copy,
    VPath_drop
  );
  ZStack_push(&stack, root);

  while(!ZStack_empty(&stack)) {
    VPath path;
    ZStack_pop(&stack, &path);
    char *cpath = VPath_to_cstr(&path);
    DIR *dir = opendir(cpath);
    if(dir == NULL) {
      printf("Could not open: %s\n", cpath);
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
        VPath full_path;
        VPath_raw(&full_path, 32);
        VPath_append(&full_path, &path);
        VPath_append(&full_path, &tmp);
        VPath_destroy(&tmp);
        ZStack_push(&stack, &full_path);

        WatchDescriptor tmpwd;
        char *ctmp = VPath_to_cstr(&full_path);
        int wd = inotify_add_watch(fd, ctmp, IN_CLOSE_WRITE | IN_CREATE);
        WatchDescriptor_init(&tmpwd, wd, &full_path);

        if(cb != NULL)
          cb(ctmp);
        free(ctmp);

        VPath_destroy(&full_path);
        ZDeque_push_back(wds, &tmpwd);
        WatchDescriptor_destroy(&tmpwd);
      }
    }

    VPath_destroy(&path);
    closedir(dir);
    free(cpath), cpath = NULL;
  }

  ZStack_destroy(&stack);
}

void watchdog_destroy(int fd, ZDeque *wds) {
  WatchDescriptor wd;
  for(size_t i = 0; i < ZDeque_len(wds); i++) {
    ZDeque_at(wds, i, &wd);
    inotify_rm_watch(fd, wd.wd);
    VPath_destroy(&wd.dirpath);
  }

  ZDeque_destroy(wds);
}

bool WatchDescriptor_init(WatchDescriptor *watchd, int fd, VPath *path) {
  if(watchd == NULL) return false;

  VPath_copy(&watchd->dirpath, path);
  watchd->wd = fd;

  return true;
}

bool WatchDescriptor_destroy(WatchDescriptor *watchd) {
  if(watchd == NULL) return false;

  VPath_destroy(&watchd->dirpath);
  watchd->wd = -1;

  return true;
}
