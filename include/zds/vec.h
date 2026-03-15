#ifndef Z_vec_h
#define Z_vec_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "iterator.h"

typedef enum _ZVecResult {
  ZVEC_OK = 0,
  ZVEC_FULL,
  ZVEC_EMPTY,
  ZVEC_NONEMPTY,
  ZVEC_ENOBUFFER,
  ZVEC_EALREADY_FREED,
  ZVEC_ENONMEMORY,
  ZVEC_EOVERFLOW,
  ZVEC_EMISMATCH,
  ZVEC_EOUTOFBOUNDS
} ZVecResult;

typedef struct _ZVec {
  uint8_t *data;
  size_t count, cap;
  size_t elem_size;
} ZVec;

typedef struct _ZVecIter {
  ZVec *vec;
  ZIter *iter;
} ZVecIter;

typedef struct _ZVecIterMut {
  ZVec *vec;
  ZIterMut *iter;
} ZVecIterMut;

typedef bool (*ZVecCB)(ZVec *vec, void *elem);

ZVecResult ZVec_init(ZVec *vec, size_t count, size_t size);
ZVecResult ZVec_reserve(ZVec *vec, size_t new_cap);
ZVecResult ZVec_push(ZVec *vec, const void *elem);
ZVecResult ZVec_pop(ZVec *vec, void *buf);
const void *ZVec_data(const ZVec *vec);
ZVecResult ZVec_at(const ZVec *vec, size_t idx, void *out);
ZVecResult ZVec_clear(ZVec *vec);
size_t ZVec_len(const ZVec *vec);
size_t ZVec_cap(const ZVec *vec);
bool ZVec_empty(const ZVec *vec);
ZVecResult ZVec_destroy(ZVec *vec);

#endif
