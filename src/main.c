#include "fstools.h"
#include "path.h"
#include "utils.h"

#include <errno.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>


#define BUF_LEN (10 * (sizeof(struct inotify_event) + 256))

#define MAX_BUF 4096

int main(int argc, char **argv) {

  if(argc < 5 || strcmp(argv[1], "--help") == 0) {
    fprintf(stderr, "\
Usage %s <src_dir> <dst_dir> <uid> <gid>\n\
  Example:\n\
  %s dist test 10277 1023\n", argv[0], argv[0]);
    return 1;
  }
  char *source = strdup(argv[1]);
  char *destination = strdup(argv[2]);
  if(access(source, F_OK) != 0) {
    fprintf(stderr, "Source directory '%s' doesn't exists\n", source);
    return 2;
  }
  if(access(destination, F_OK) != 0) {
    fprintf(stderr, "Destination directory '%s' doesn't exists\n", destination);
    return 2;
  }
  char *uid = strdup(argv[3]);
  char *gid = strdup(argv[4]);
  int iuid = strtod(uid, NULL);
  int igid = strtod(gid, NULL);
  int fd = inotify_init(), count = 0, i = 0;
  if(fd < 0) {
    return 1;
  }
  WatchDescriptor **wd = calloc(100, sizeof(WatchDescriptor *));
  int res = watchdog(source, fd, &wd, &count), nbytes;
  char buf[4096], *fullpath = NULL, *dirdest;
  char *pathname;
  printf("Watching source directory: '%s' ...\n", source);
  while(1) {
    i = 0;
    nbytes = read(fd, buf, 10 * (sizeof(struct inotify_event) + 256));
    if(nbytes < 0) {
      return 1;
    }
    while(i < nbytes) {
      struct inotify_event *event = (struct inotify_event *)&buf[i];
      if(event->len > 0) {
        for(int i = 0; i < count; i++) {
          if(event->wd == wd[i]->wd) {
            path_shift(wd[i]->dirpath, &pathname);
            path_join(wd[i]->dirpath, event->name, &fullpath);
            path_join(destination, pathname, &dirdest);
            path_join(dirdest, event->name, &dirdest);
            fs_mkdir(dirname(dirdest), 0700, 1);
            int success = fs_copy(dirdest, fullpath);
            if(success == -1) {
              fprintf(stderr, "Error: %s\n", strerror(errno));
            } else {
              printf("Done with '%s' -> '%s'\n", fullpath, dirdest);
              chown(dirdest, iuid, igid);
            }
          }
        }
        i += (sizeof(struct inotify_event)) + event->len;
      }
    }
  }

  return 0;
}
