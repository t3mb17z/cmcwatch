#ifndef TEXT_H
#define TEXT_H

#include "zds/deque.h"
#include <stddef.h>

typedef enum _vstring_result {
  VSTRING_OK = 0,
  VSTRING_ENOBUFFER,
  VSTRING_EOVERFLOW,
  VSTRING_ALREADYFREED,
} VStringResult;

typedef struct _vstring {
  char *_buffer;
  size_t _cap;
  size_t _len;
} VString;

// typedef struct {
//   int sindex;
//   int eindex;
//   int size;
// } splitinfo;

VStringResult VString_new(VString *buffer, size_t cap);
VStringResult VString_set(VString *buffer, char *text);
VStringResult VString_append(VString *buffer, const VString *text);
VStringResult VString_append_char(VString *buffer, char chr);
VStringResult VString_split(
  const VString *buffer,
  const VString *delim,
  ZDeque *result
);
VStringResult VString_replace(
  const VString *src,
  const VString *pat,
  const VString *rep,
  VString *dest
);
size_t VString_len(const VString *buffer);
size_t VString_cap(const VString *buffer);
bool VString_eq(const VString *str1, const VString *str2);
char VString_at(const VString *buffer, size_t idx);
VString VString_slice(
  const VString *buffer,
  size_t idx, size_t len
);
VString VString_from_bytes(const char *text);
char *VString_to_cstr(const VString *buffer);
VStringResult VString_destroy(VString *buffer);

#endif
