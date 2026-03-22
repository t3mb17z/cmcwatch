#ifndef Z_vec_h
#define Z_vec_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum _ZVecResult {
  ZVEC_OK = 0,
  ZVEC_ERR_OOM,
  ZVEC_ERR_OVERFLOW,
  ZVEC_ERR_OUT_OF_BOUNDS,
  ZVEC_ERR_ALREADY_FREED,
  ZVEC_ERR_EMPTY,
  ZVEC_ERR_FULL,
  ZVEC_ERR_NULL_ARG,
  ZVEC_ERR_CALLBACK
} ZVecResult;

typedef ZVecResult (*Zds_copy_fn)(void *dest, const void *source);
typedef ZVecResult (*Zds_drop_fn)(void *elem);

typedef struct _ZVec {
  uint8_t *data;
  size_t count, cap;
  size_t elem_size;
  Zds_copy_fn copy;
  Zds_drop_fn drop;
} ZVec;

ZVecResult ZVec_init(
  ZVec *vec, size_t count, size_t size,
  Zds_copy_fn copy_fn, Zds_drop_fn drop_fn
);
ZVecResult ZVec_reserve(ZVec *vec, size_t new_cap);
ZVecResult ZVec_push(ZVec *vec, const void *elem);
ZVecResult ZVec_pop(ZVec *vec, void *buf);
ZVecResult ZVec_copy(ZVec *dst, const ZVec *src);
const void *ZVec_data(const ZVec *vec);
ZVecResult ZVec_at(const ZVec *vec, size_t idx, void *out);
const void *ZVec_get(const ZVec *vec, size_t idx);
ZVecResult ZVec_clear(ZVec *vec);
size_t ZVec_len(const ZVec *vec);
size_t ZVec_cap(const ZVec *vec);
bool ZVec_empty(const ZVec *vec);
ZVecResult ZVec_destroy(ZVec *vec);

#endif
