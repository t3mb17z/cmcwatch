#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include "fstools.h"
#include "path.h"
#include "text.h"

int fs_copy(const char *to, const char *from) {
  int fd_from, fd_to;
  char buf[4096];
  ssize_t nread;
  int saved_errno;
  fd_from = open(from, O_RDONLY);
  if(fd_from < 0) return -1;
  fd_to = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if(fd_to < 0) goto out_error;
  while(nread = read(fd_from, buf, sizeof buf), nread > 0) {
    char *out_ptr = buf;
    ssize_t nwritten;
    do {
      nwritten = write(fd_to, out_ptr, nread);
      if(nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if(errno != EINTR) {
        return -1;
      }
    } while (nread > 0);
  }
  if(nread == 0) {
    if(close(fd_to) < 0) {
      fd_to = -1;
      goto out_error;
    }
  }
  return 0;
out_error:
  saved_errno = errno;
  close(fd_from);
  if(fd_to >= 0) close(fd_to);
  errno = saved_errno;
  return -1;
}

int fs_mkdir(const char *dir, unsigned int mode, int recursive) {
  if(strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0) {
    return 0;
  }
  char *path = strdup(dir);
  char *directories = NULL;
  char *auxiliar = calloc(strlen(dir) + 1, 1);
  char **saveptr;

  if(!recursive) {
    if(mkdir(path, mode) != 0) {
      free(path); return 0;
    }
  } else {
    path_normalize(path, &directories);
    int res = text_split(directories, "/", &saveptr);

    if(strncmp(directories, "/", 1) == 0)
      strcat(auxiliar, "/");

    for(int i = 0; i < res; i++) {
      path_join(auxiliar, saveptr[i], &auxiliar);

      if(access(auxiliar, F_OK) != 0) {
        if(mkdir(auxiliar, mode) != 0 && errno != EEXIST) {
          free(auxiliar); free(path);
          free(directories);
          return 0;
        }
      }

    }
  }
  return 1;
}
