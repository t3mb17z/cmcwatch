#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <libgen.h>

#include "fstools.h"
#include "path.h"
#include "vstring.h"
#include "zds/deque.h"

bool fs_copy(const VPath *to, const VPath *from) {
  char *in = VPath_to_cstr(to),
       *out= VPath_to_cstr(from);
  char buf[8192];
  ssize_t n;

  int fd_in = open(in, O_RDONLY);
  int fd_out = open(out, O_WRONLY | O_CREAT | O_TRUNC);

  while((n = read(fd_in, buf, sizeof(buf))) > 0) {
    char *p = buf;
    while(n > 0) {
      ssize_t w = write(fd_out, p, n);
      if(w < 0) {
        close(fd_in);
        close(fd_out);
        return false;
      }

      p += w;
      n -= w;
    }
  }

  if(n < 0)
    return false;

  return true;
}

bool fs_mkdir(const VPath *dir, unsigned int mode, bool recursive) {
  if(dir == NULL)
    return false;

  char *path = VPath_to_cstr(dir);
  bool res = false;
  if(!recursive) {
    if(mkdir(path, mode) == 0)
      res = true;

    free(path);
    return res;
  }
  free(path), path = NULL;

  VString part, full_path;
  VString_new(&full_path, 4096);

  res = true;
  for(size_t i = 0; i < VPath_name_count(dir); i++) {
    ZDeque_at(&dir->_segments, i, &part);
    VString_append(&full_path, &part);
    path = VString_to_cstr(&full_path);
    if(mkdir(path, mode) == 0) {
      free(path), path = NULL;
      continue;
    }

    res = false;
    free(path);
    break;
  }

  return true;
}
