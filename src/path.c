#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"
#include "vstring.h"
#include "zds/deque.h"

#define DBG(x, y, z) \
  char *p = V##y##_to_cstr((x)); \
  printf((z "%s\n"), p); free(p)

VPathResult VPath_init(VPath *path, const VString *text) {
  if(path == NULL || text == NULL)
    return VPATH_EUNSPECIFIED_PATH;

  VString dot = VString_from_bytes("."),
          dotdot = VString_from_bytes("..");
  char *cpath = VString_to_cstr(text);
  if(VString_eq(text, &dot) || VString_eq(text, &dotdot) || memchr(cpath, '/', VString_len(text)) == NULL) {
    path->_is_absolute = false;
    ZDeque_init(&path->_segments, VString_len(text), sizeof(VString));
    ZDeque_push_back(&path->_segments, text);
    free(cpath), cpath = NULL;
    return VPATH_OK;
  }
  free(cpath), cpath = NULL;

  VPath res;
  VString delim = VString_from_bytes("/");
  ZDeque_init(&res._segments, VString_len(text), sizeof(VString));
  if(VString_at(text, 0) == '/')
    path->_is_absolute =
      res._is_absolute = true;
  else
    path->_is_absolute =
      res._is_absolute = false;
  VString_split(text, &delim, &res._segments);
  if(ZDeque_len(&res._segments) == 1) {
    memcpy(path, &res, sizeof(VPath));
    return VPATH_OK;
  }
  VPath_normalize(&res, path);

  return VPATH_OK;
}

VPathResult VPath_normalize(const VPath *path, VPath *destpath) {
  if(path == NULL || destpath == NULL)
    return VPATH_EUNSPECIFIED_PATH;

  destpath->_is_absolute = path->_is_absolute;
  ZDeque_init(&destpath->_segments, ZDeque_len(&path->_segments), sizeof(VString));
  for(size_t i = 0; i < ZDeque_len(&path->_segments); i++) {
    VString pathbuf, buf, res, empty = VString_from_bytes("");
    res = VString_from_bytes("/");

    ZDeque_at(&path->_segments, i, &pathbuf);
    VString_new(&buf, VString_len(&pathbuf));
    VString_replace(&pathbuf, &res, &empty, &buf);

    pathbuf = VString_from_bytes(".");
    if(VString_eq(&buf, &pathbuf))
      continue;

    pathbuf = VString_from_bytes("..");
    if(VString_eq(&buf, &pathbuf)) {
      if(!ZDeque_empty(&destpath->_segments))
        ZDeque_pop_back(&destpath->_segments, NULL);
      continue;
    }

    ZDeque_push_back(&destpath->_segments, &buf);
  }

  return VPATH_OK;
}

VPathResult VPath_join(
  const VPath *path1,
  const VPath *path2,
  VPath *result
) {
  if(path1 == NULL || path2 == NULL || result == NULL)
    return VPATH_EUNSPECIFIED_PATH;

  result->_is_absolute = path1->_is_absolute;
  ZDeque_init(
    &result->_segments,
    VPath_name_count(path1) + VPath_name_count(path2),
    sizeof(VString)
  );

  for(size_t i = 0; i < VPath_name_count(path1); i++) {
    VString segment;
    ZDeque_at(&path1->_segments, i, &segment);
    ZDeque_push_back(&result->_segments, &segment);
  }

  for(size_t i = 0; i < VPath_name_count(path2); i++) {
    VString segment;
    ZDeque_at(&path2->_segments, i, &segment);
    ZDeque_push_back(&result->_segments, &segment);
  }

  return 0;
}

VPathResult VPath_append(VPath *path1, const VPath *path2) {
  if(path1 == NULL || path2 == NULL)
    return VPATH_EUNSPECIFIED_PATH;

  if(
    VPath_name_count(path1) +
    VPath_name_count(path2) >
    ZDeque_cap(&path1->_segments)
  )
    return VPATH_ENOT_ENOUGH_SPACE;
  if(VPath_name_count(path1) == 0)
    path1->_is_absolute = path2->_is_absolute;

  VString temp;
  for(size_t i = 0; i < VPath_name_count(path2); i++) {
    ZDeque_at(&path2->_segments, i, &temp);
    ZDeque_push_back(&path1->_segments, &temp);
  }

  return VPATH_OK;
}

VPathResult VPath_shift(VPath *path, VString *dest) {
  if(path == NULL || dest == NULL)
    return VPATH_EUNSPECIFIED_PATH;
  if(ZDeque_empty(&path->_segments))
    return VPATH_ENOT_ENOUGH_SPACE;

  ZDeque_pop_front(&path->_segments, dest);

  return VPATH_OK;
}

VPathResult VPath_pop(VPath *path, VString *dest) {
  if(path == NULL || dest == NULL)
    return VPATH_EUNSPECIFIED_PATH;
  if(ZDeque_empty(&path->_segments))
    return VPATH_ENOT_ENOUGH_SPACE;

  ZDeque_pop_back(&path->_segments, dest);

  return VPATH_OK;
}

size_t VPath_name_count(const VPath *path) {
  return ZDeque_len(&path->_segments);
}

char *VPath_to_cstr(const VPath *path) {
  VString cur, res, slash = VString_from_bytes("/");
  size_t len = 0;
  for(size_t i = 0; i < VPath_name_count(path); i++) {
    ZDeque_at(&path->_segments, i, &cur);
    len += VString_len(&cur);
  }

  VString_new(&res, len + VPath_name_count(path));

  if(path->_is_absolute)
    VString_append(&res, &slash);
  puts("Ok");
  for(size_t i = 0; i < VPath_name_count(path); i++) {
    ZDeque_at(&path->_segments, i, &cur);
    char *t = VString_to_cstr(&cur);
    printf("Valid string: %s\n", t);
    free(t);
    VString_append(&res, &cur);
    VString_append(&res, &slash);
  }

  char *r = VString_to_cstr(&res);
  printf("cur: %s\n", r);
  return r;
}

VString VPath_to_VString(const VPath *path) {
  VString cur, res, slash = VString_from_bytes("/");
  size_t len = 0;
  for(size_t i = 0; i < VPath_name_count(path); i++) {
    ZDeque_at(&path->_segments, i, &cur);
    len += VString_len(&cur);
  }

  VString_new(&res, len + VPath_name_count(path) - 1);

  if(path->_is_absolute)
    VString_append(&res, &slash);
  for(size_t i = 0; i < VPath_name_count(path); i++) {
    ZDeque_at(&path->_segments, i, &cur);
    VString_append(&res, &cur);
    VString_append(&res, &slash);
  }

  return res;
}

VPathResult VPath_raw(VPath *path, int nodes) {
  if(path == NULL)
    return VPATH_EUNSPECIFIED_PATH;
  if(nodes < 1) {
    *path = (VPath){ 0 };
    return VPATH_OK;
  }

  ZDeque_init(&path->_segments, nodes, sizeof(VString));
  path->_is_absolute = false;

  return VPATH_OK;
}

VPath VPath_from_cstr(const char *cpath) {
  if(cpath == NULL)
    return (VPath){ 0 };

  VPath res;
  VString vstr = VString_from_bytes(cpath);
  VPath_init(&res, &vstr);

  return res;
}

VPathResult VPath_destroy(VPath *path) {
  if(path == NULL)
    return VPATH_OK;

  ZDeque_destroy(&path->_segments);
  path->_is_absolute = false;

  return VPATH_OK;
}
