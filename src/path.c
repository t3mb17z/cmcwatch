#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"
#include "vstring.h"
#include "zds/deque.h"
#include "zds/vec.h"

#define DBG(x, y, z) \
  char *p = V##y##_to_cstr((x)); \
  printf((z "%s\n"), p); free(p)

ZVecResult VString_deep_copy(void *dest, const void *src) {
  const VString *source = src;
  VString *destination = dest;
  VString_copy(destination, source);
  return ZVEC_OK;
}

ZVecResult VString_drop(void *elem) {
  VString *string = elem;
  VString_destroy(string);
  return ZVEC_OK;
}

VPathResult VPath_init(VPath *path, const VString *text) {
  if (path == NULL || text == NULL)
    return VPATH_ERR_UNSPECIFIED_PATH;

  VString dot = VString_from_bytes("."),
          dotdot = VString_from_bytes("..");

  if (VString_eq(text, &dot) || VString_eq(text, &dotdot)) {
    path->_is_absolute = false;
    ZDeque_init(
      &path->_segments, 32,
      sizeof(VString), VString_deep_copy,
      VString_drop
    );
    VString_destroy(&dot);
    VString_destroy(&dotdot);
    return VPATH_OK;
  }

  char *cpath = VString_to_cstr(text);
  if (memchr(cpath, '/', VString_len(text)) == NULL) {
    path->_is_absolute = false;
    ZDeque_init(
      &path->_segments, VString_len(text),
      sizeof(VString), VString_deep_copy,
      VString_drop
    );
    ZDeque_push_back(&path->_segments, text);
    VString_destroy(&dot);
    VString_destroy(&dotdot);
    free(cpath), cpath = NULL;
    return VPATH_OK;
  }
  VString_destroy(&dot);
  VString_destroy(&dotdot);
  free(cpath), cpath = NULL;

  VPath res;
  VString delim = VString_from_bytes("/");
  ZDeque_init(
    &res._segments, VString_len(text),
    sizeof(VString), VString_deep_copy,
    VString_drop
  );

  if (VString_at(text, 0) == '/')
    path->_is_absolute =
      res._is_absolute = true;
  else
    path->_is_absolute =
      res._is_absolute = false;

  VString_split(text, &delim, &res._segments);
  if (ZDeque_len(&res._segments) == 1) {
    memcpy(path, &res, sizeof(VPath));
    return VPATH_OK;
  }
  VPath_normalize(path, &res);
  VString_destroy(&delim);

  return VPATH_OK;
}

VPathResult VPath_normalize(VPath *destpath, const VPath *path) {
  if (path == NULL || destpath == NULL)
    return VPATH_ERR_UNSPECIFIED_PATH;

  destpath->_is_absolute = path->_is_absolute;
  ZDeque_init(
    &destpath->_segments,
    ZDeque_len(&path->_segments),
    sizeof(VString),
    VString_deep_copy,
    VString_drop
  );

  VString pattern = VString_from_bytes("/"),
          empty = VString_from_bytes("");
  for (size_t i = 0; i < ZDeque_len(&path->_segments); i++) {
    VString pathbuf, buf;

    ZDeque_at(&path->_segments, i, &pathbuf);
    VString_new(&buf, VString_len(&pathbuf));
    VString_replace(&pathbuf, &pattern, &empty, &buf);
    VString_destroy(&pathbuf);

    pathbuf = VString_from_bytes(".");
    if (VString_eq(&buf, &pathbuf)) {
      VString_destroy(&pathbuf);
      continue;
    } else {
      VString_destroy(&pathbuf);
    }

    pathbuf = VString_from_bytes("..");
    if (VString_eq(&buf, &pathbuf)) {
      if (!ZDeque_empty(&destpath->_segments))
        ZDeque_pop_back(&destpath->_segments, NULL);
      VString_destroy(&pathbuf);
      continue;
    } else {
      VString_destroy(&pathbuf);
    }

    ZDeque_push_back(&destpath->_segments, &buf);
  }

  VString_destroy(&pattern);
  VString_destroy(&empty);

  return VPATH_OK;
}

VPathResult VPath_join(
  VPath *result,
  const VPath *path1,
  const VPath *path2
) {
  if (path1 == NULL || path2 == NULL || result == NULL)
    return VPATH_ERR_UNSPECIFIED_PATH;

  result->_is_absolute = path1->_is_absolute;
  ZDeque_init(
    &result->_segments,
    VPath_name_count(path1) + VPath_name_count(path2),
    sizeof(VString),
    VString_deep_copy,
    VString_drop
  );

  for (size_t i = 0; i < VPath_name_count(path1); i++) {
    const VString *segment = ZDeque_get(&path1->_segments, i);
    ZDeque_push_back(&result->_segments, segment);
  }

  for (size_t i = 0; i < VPath_name_count(path2); i++) {
    const VString *segment = ZDeque_get(&path2->_segments, i);
    ZDeque_push_back(&result->_segments, segment);
  }

  return 0;
}

VPathResult VPath_copy(VPath *dest, const VPath *src) {
  VPath_raw(dest, VPath_name_count(src));
  dest->_is_absolute = src->_is_absolute;
  ZVecResult res = ZDeque_copy(&dest->_segments, &src->_segments);
  if (res != ZVEC_OK)
    return VPATH_ERR_NOT_ENOUGH_SPACE;

  return VPATH_OK;
}

VPathResult VPath_append(VPath *path1, const VPath *path2) {
  if (path1 == NULL || path2 == NULL)
    return VPATH_ERR_UNSPECIFIED_PATH;

  if (
    VPath_name_count(path1) +
    VPath_name_count(path2) >
    ZDeque_cap(&path1->_segments)
  )
    return VPATH_ERR_NOT_ENOUGH_SPACE;
  if (VPath_name_count(path1) == 0)
    path1->_is_absolute = path2->_is_absolute;

  const VString *temp = NULL;
  for (size_t i = 0; i < VPath_name_count(path2); i++) {
    temp = ZDeque_get(&path2->_segments, i);
    ZDeque_push_back(&path1->_segments, temp);
  }

  return VPATH_OK;
}

VPathResult VPath_shift(VPath *path, VString *dest) {
  if (path == NULL)
    return VPATH_ERR_UNSPECIFIED_PATH;
  if (ZDeque_empty(&path->_segments))
    return VPATH_ERR_NOT_ENOUGH_SPACE;

  ZDeque_pop_front(&path->_segments, dest);

  return VPATH_OK;
}

VPathResult VPath_pop(VPath *path, VString *dest) {
  if (path == NULL)
    return VPATH_ERR_UNSPECIFIED_PATH;
  if (ZDeque_empty(&path->_segments))
    return VPATH_ERR_NOT_ENOUGH_SPACE;

  ZDeque_pop_back(&path->_segments, dest);

  return VPATH_OK;
}

size_t VPath_name_count(const VPath *path) {
  return ZDeque_len(&path->_segments);
}

char *VPath_to_cstr(const VPath *path) {
  VString res, slash = VString_from_bytes("/");
  size_t len = 0;
  const VString *buf = NULL;
  for (size_t i = 0; i < VPath_name_count(path); i++) {
    buf = ZDeque_get(&path->_segments, i);
    len += VString_len(buf);
  }

  VString_new(&res, len + VPath_name_count(path));

  if (path->_is_absolute)
    VString_append(&res, &slash);

  for (size_t i = 0; i < VPath_name_count(path); i++) {
    buf = ZDeque_get(&path->_segments, i);
    VString_append(&res, buf);
    if (i < VPath_name_count(path) - 1)
      VString_append(&res, &slash);
  }
  VString_destroy(&slash);

  char *r = VString_to_cstr(&res);
  return r;
}

VString VPath_to_VString(const VPath *path) {
  VString res, slash = VString_from_bytes("/");
  const VString *cur = NULL;
  size_t len = 0;
  for (size_t i = 0; i < VPath_name_count(path); i++) {
    cur = ZDeque_get(&path->_segments, i);
    len += VString_len(cur);
  }

  VString_new(&res, len + VPath_name_count(path));

  if (path->_is_absolute)
    VString_append(&res, &slash);
  for (size_t i = 0; i < VPath_name_count(path); i++) {
    cur = ZDeque_get(&path->_segments, i);
    VString_append(&res, cur);
    if (i < VPath_name_count(path) - 1)
      VString_append(&res, &slash);
  }
  VString_destroy(&slash);

  return res;
}

VPathResult VPath_raw(VPath *path, int nodes) {
  if (path == NULL)
    return VPATH_ERR_UNSPECIFIED_PATH;
  if (nodes < 1) {
    *path = (VPath){ 0 };
    return VPATH_OK;
  }

  ZDeque_init(
    &path->_segments, nodes,
    sizeof(VString), VString_deep_copy,
    VString_drop
  );
  path->_is_absolute = false;

  return VPATH_OK;
}

VPath VPath_from_cstr(const char *cpath) {
  if (cpath == NULL)
    return (VPath){ 0 };

  VPath res;
  VString vstr = VString_from_bytes(cpath);
  VPath_init(&res, &vstr);

  return res;
}

VPathResult VPath_destroy(VPath *path) {
  if (path == NULL)
    return VPATH_OK;

  ZDeque_destroy(&path->_segments);
  path->_is_absolute = false;

  return VPATH_OK;
}
