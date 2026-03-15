#ifndef Z_queue_h
#define Z_queue_h

#include "vec.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct _ZQueue {
  ZVec data;
  size_t head, tail;
} ZQueue;

ZVecResult ZQueue_init(ZQueue *queue, size_t count, size_t size);
/**
 * Push element at end of this structure (at tail)
 */
ZVecResult ZQueue_enqueue(ZQueue *queue, const void *elem);
/**
 * Push element at begin of this structure (at head)
 */
ZVecResult ZQueue_dequeue(ZQueue *queue, void *buffer);
ZVecResult ZQueue_peek(ZQueue *queue, void *buffer);
size_t ZQueue_len(ZQueue *queue);
size_t ZQueue_cap(ZQueue *queue);
bool ZQueue_empty(ZQueue *queue);
ZVecResult ZQueue_destroy(ZQueue *queue);

#endif
