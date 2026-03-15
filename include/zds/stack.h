#ifndef Z_stack_h
#define Z_stack_h

#include <stdbool.h>

#include "vec.h"

typedef struct _ZStack {
  ZVec data;
} ZStack;

ZVecResult ZStack_init(ZStack *stack, size_t elem_count, size_t elem_size);
ZVecResult ZStack_reserve(ZStack *stack, size_t new_size);
ZVecResult ZStack_push(ZStack *stack, const void *data);
ZVecResult ZStack_peek(const ZStack *stack, void *buffer);
ZVecResult ZStack_pop(ZStack *stack, void *out);
size_t ZStack_len(ZStack *stack);
size_t ZStack_cap(ZStack *stack);
bool ZStack_empty(ZStack *stack);
ZVecResult ZStack_destroy(ZStack *stack);

#endif
