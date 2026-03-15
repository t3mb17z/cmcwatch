#ifndef Z_iter_h
#define Z_iter_h

#include <stdbool.h>
#include <stdint.h>

typedef enum _ZIterResult {
  ZITER_OK = 0,
  ZITER_ENOBUFFER,
  ZITER_INVALID_SIZE,
  ZITER_OUTOFBOUNDS
} ZIterResult;

typedef struct _ZIter {
  const uint8_t *cur;
  const uint8_t *end;
  size_t elem_size;
} ZIter;

typedef struct _ZIterMut {
  uint8_t *cur;
  const uint8_t *end;
  size_t elem_size;
} ZIterMut;

ZIterResult ZIter_init(ZIter *iter, size_t elem_size, size_t len, const void *data);
ZIterResult ZIterMut_init(ZIterMut *iter, size_t elem_size, size_t len, const void *data);
const void *ZIter_next(ZIter *iter);
void *ZIterMut_next(ZIterMut *iter);

#endif
