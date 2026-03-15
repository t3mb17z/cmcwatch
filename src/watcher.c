#include <linux/stat.h>
#include <stdbool.h>
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

/**
 * Watch for source and subdirectories recursively
 *
 * It returns the state of the operation
 */
void watchdog_init(const VPath *path, int fd, ZDeque *wds) {

  char *cpath = NULL;
  VPath buf;
  VPath path_copy;
  VPath_raw(&path_copy, 32);
  VPath_append(&path_copy, path);
  size_t root_idx = VPath_name_count(path), cur_idx = root_idx + 1;

  ZDeque_init(wds, 4096, sizeof(WatchDescriptor));

  while(cur_idx > root_idx) {
    cpath = VPath_to_cstr(&path_copy);
    DIR *dir = opendir(cpath);
    if(dir == NULL)
      continue;

    struct dirent *diren;
    while((diren = readdir(dir)) != NULL) {
      if(strcmp(diren->d_name, ".") == 0 || strcmp(diren->d_name, "..") == 0)
        continue;

      if(diren->d_type == DT_DIR) {
        buf = VPath_from_cstr(diren->d_name);
        VPath_append(&path_copy, &buf);
        cur_idx++;
        free(cpath);
        cpath = NULL;

        cpath = VPath_to_cstr(&path_copy);
        int wd = inotify_add_watch(fd, cpath,
          IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE
        );
        free(cpath);
        cpath = NULL;

        WatchDescriptor wdesc = { 0 };
        wdesc.wd = wd;
        VPath_append(&wdesc.dirpath, &buf);

        ZDeque_push_back(wds, &wdesc);
      }
    }

    if(cpath != NULL)
      free(cpath);
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
