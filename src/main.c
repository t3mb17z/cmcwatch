#include "fstools.h"
#include "path.h"
#include "utils.h"

#include <stdio.h>
#include <sys/inotify.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>


#define BUF_LEN (10 * (sizeof(struct inotify_event) + 256))

#define MAX_BUF 4096

int main(int argc, char **argv) {

  char *source = strdup(argv[1]);
  char *destination = strdup(argv[2]);
  int fd = inotify_init(), count = 0, i = 0;
  if(fd < 0) {
    return 1;
  }
  WatchDescriptor **wd = calloc(100, sizeof(WatchDescriptor *));
  int res = watchdog(source, fd, &wd, &count), nbytes;
  char buf[4096], *fullpath = NULL, *dirdest;
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
            path_join(wd[i]->dirpath, event->name, &fullpath);
            path_join(destination, fullpath, &dirdest);
            fs_mkdir(dirname(dirdest), 0700, 1);
            int success = fs_copy(dirdest, fullpath);
            if(!success) {
              fprintf(stderr, "Error...\n");
            } else {
              printf("Done with '%s' -> '%s'\n", fullpath, dirdest);

            }
          }
        }
        i += (sizeof(struct inotify_event)) + event->len;
      }
    }
  }

  return 0;
}
