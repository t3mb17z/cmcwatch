#ifndef Z_deque_h
#define Z_deque_h

#include "vec.h"

typedef struct _ZDeque {
  ZVec data;
  size_t head, tail;
} ZDeque;

ZVecResult ZDeque_init(ZDeque *deque, size_t count, size_t size);
ZVecResult ZDeque_push_front(ZDeque *deque, void *elem);
ZVecResult ZDeque_pop_front(ZDeque *deque, void *out);
ZVecResult ZDeque_push_back(ZDeque *deque, void *elem);
ZVecResult ZDeque_pop_back(ZDeque *deque, void *out);
ZVecResult ZDeque_at(const ZDeque *deque, size_t idx, void *out);
size_t ZDeque_len(const ZDeque *deque);
size_t ZDeque_cap(const ZDeque *deque);
bool ZDeque_empty(const ZDeque *deque);
ZVecResult ZDeque_destroy(ZDeque *deque);

#endif
