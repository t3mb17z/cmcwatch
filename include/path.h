#ifndef PATH_H
#define PATH_H

#include <stddef.h>
#include <stdbool.h>

#include "zds/deque.h"
#include "vstring.h"

typedef enum _vpath_res {
  VPATH_OK = 0,
  VPATH_EUNSPECIFIED_PATH,
  VPATH_ENOT_ENOUGH_SPACE,
} VPathResult;

typedef struct _vpath {
  ZDeque _segments;
  bool _is_absolute;
} VPath;

VPathResult VPath_init(VPath *path, const VString *text);
VPathResult VPath_normalize(const VPath *path, VPath *detspath);
VPathResult VPath_join(
  const VPath *path1,
  const VPath *path2,
  VPath *destpath
);
VPathResult VPath_append(VPath *path1, const VPath *path2);
VPathResult VPath_shift(VPath *path, VString *dest);
VPathResult VPath_pop(VPath *path, VString *dest);
size_t VPath_name_count(const VPath *path);
char *VPath_to_cstr(const VPath *path);
VString VPath_to_VString(const VPath *path);
VPath VPath_from_cstr(const char *cpath);
VPathResult VPath_raw(VPath *path, int nodes);
VPathResult VPath_destroy(VPath *path);

#endif
