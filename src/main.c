#include <errno.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>

#include "fstools.h"
#include "path.h"
#include "watcher.h"
#include "zds/deque.h"

#define BUF_LEN (10 * (sizeof(struct inotify_event) + 256))

#define MAX_BUF 4096

int main(int argc, char **argv) {

  if(argc < 2 || strcmp(argv[1], "--help") == 0) {
    fprintf(stderr,
    "Usage %s <src_dir> <dst_dir> <uid> <gid>\n"
    "  Example:\n"
    "  %s dist test 10277 1023\n",
    argv[0], argv[0]);
    return 1;
  }

  VPath source = VPath_from_cstr(argv[1]);
  VPath destination = VPath_from_cstr(argv[2]);
  char *tmp = VPath_to_cstr(&source);
  if(access(tmp, F_OK) != 0) {
    fprintf(stderr, "Source directory '%s' doesn't exists\n", tmp);
    return 2;
  }
  free(tmp);

  tmp = VPath_to_cstr(&destination);
  if(access(tmp, F_OK) != 0) {
    fprintf(stderr, "Destination directory '%s' doesn't exists\n", tmp);
    return 2;
  }
  free(tmp);

  char *buf;
  char *uid = strdup(argv[3]);
  char *gid = strdup(argv[4]);

  int iuid = strtol(uid, &buf, 8);
  if(*buf != '\0') {
    puts("Invalid number at UID");
    return 1;
  }

  int igid = strtol(gid, &buf, 8);
  if(*buf != '\0') {
    puts("Invalod number at GID");
    return 1;
  }

  int fd = inotify_init(), count = 0, i = 0;
  if(fd < 0) {
    return 1;
  }

  ZDeque wds;
  watchdog_init(&source, fd, &wds);
  char buffer[4096];
  int nbytes;
  VPath file_path, dest_file, src_file, dest_dir;
  VPath_raw(&dest_file, 32);
  VPath_raw(&src_file, 32);
  VPath_raw(&dest_dir, 32);

  tmp = VPath_to_cstr(&source);
  printf("Watching source directory: '%s' ...\n", tmp);
  free(tmp);

  WatchDescriptor wd;
  while(1) {
    i = 0, nbytes = read(fd, buffer, BUF_LEN);
    if(nbytes < 0) {
      return 1;
    }

    while(i < nbytes) {
      struct inotify_event *event = (struct inotify_event *)&buffer[i];
      if(event->len > 0) {
        for(int i = 0; i < count; i++) {
          ZDeque_at(&wds, i, &wd);
          if(event->wd == wd.wd) {
            file_path = VPath_from_cstr(event->name);

            VPath_append(&src_file, &source);
            VPath_append(&src_file, &wd.dirpath);
            VPath_append(&src_file, &file_path);

            VPath_append(&dest_file, &destination);
            VPath_append(&dest_file, &wd.dirpath);
            VPath_append(&dest_file, &file_path);

            VPath_append(&dest_dir, &destination);
            VPath_append(&dest_dir, &wd.dirpath);

            bool success;
            success = fs_mkdir(&dest_dir, 0700, true);
            success = fs_copy(&src_file, &dest_file);
            if(!success) {
              fprintf(stderr, "Error: %s\n", strerror(errno));
            } else {
              tmp = VPath_to_cstr(&dest_file);
              buf = VPath_to_cstr(&src_file);
              printf("Done with '%s' -> '%s'\n", buf, tmp);
              free(tmp), tmp = NULL;
              free(buf), buf = NULL;

              tmp = VPath_to_cstr(&dest_dir);
              chown(tmp, iuid, igid);
            }
          }
        }
        i += (sizeof(struct inotify_event)) + event->len;
      }
    }
  }

  watchdog_destroy(fd, &wds);

  return 0;
}
