#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zds/deque.h"
#include "vstring.h"

#define DBG(x, z) \
  char *p = VString_to_cstr((x)); \
  printf((z "%s\n"), p); free(p)

VStringResult VString_new(VString *buffer, size_t cap) {
  if (buffer == NULL)
    return VSTRING_ENOBUFFER;

  buffer->_buffer = calloc(cap, sizeof(char));
  buffer->_len = 0;
  buffer->_cap = cap;

  return VSTRING_OK;
}

VStringResult VString_set(VString *buffer, char *text) {
  if (buffer == NULL || text == NULL)
    return VSTRING_ENOBUFFER;
  size_t len = strlen(text);
  if (len > buffer->_cap)
    return VSTRING_EOVERFLOW;

  memcpy(buffer->_buffer, text, len);
  buffer->_len = len;

  return VSTRING_OK;
}

VStringResult VString_copy(VString *dest, const VString *src) {
  if (dest == NULL || src == NULL)
    return VSTRING_ENOBUFFER;

  VString_new(dest, VString_len(src));
  memcpy(dest->_buffer, src->_buffer, VString_len(src));
  dest->_cap = src->_cap;
  dest->_len = src->_len;

  return VSTRING_OK;
}

VStringResult VString_append(VString *buffer, const VString *text) {
  if (buffer == NULL || text == NULL)
    return VSTRING_ENOBUFFER;
  size_t len = text->_len;
  if (buffer->_len + len > buffer->_cap)
    return VSTRING_EOVERFLOW;

  memcpy(buffer->_buffer + buffer->_len, text->_buffer, len);
  buffer->_len += len;

  return VSTRING_OK;
}

VStringResult VString_append_char(VString *buffer, char chr) {
  if (buffer->_len + 1 < VString_cap(buffer))
    return VSTRING_EOVERFLOW;

  buffer->_buffer[buffer->_len] = chr;
  buffer->_len++;

  return VSTRING_OK;
}

VStringResult VString_split(
  const VString *buffer,
  const VString *delim,
  ZDeque *res
) {
  if (buffer == NULL || delim == NULL || res == NULL)
    return VSTRING_ENOBUFFER;

  bool eq = true;
  size_t start = 0, end = 0;
  VString token;
  for (size_t i = 0; i < buffer->_len; i++) {
    if (i + VString_len(delim) > buffer->_len)
      break;
    if (VString_at(buffer, i) == VString_at(delim, 0)) {
      eq = true;
      for (size_t j = 0; j < VString_len(delim); j++) {
        if (VString_at(buffer, j + i) != VString_at(delim, j)) {
          eq = false;
          break;
        }
      }

      if (eq) {
        end = i - start;
        if (end > 0) {
          token = VString_slice(buffer, start, end);
          ZDeque_push_back(res, &token);
          VString_destroy(&token);
        }

        start = i + VString_len(delim);
        i += VString_len(delim) - 1;
      }
    }
  }

  if (start < buffer->_len) {
    token = VString_slice(buffer, start, buffer->_len - start);
    ZDeque_push_back(res, &token);
    VString_destroy(&token);
  }

  return VSTRING_OK;
}

VStringResult VString_replace(
  const VString *src,
  const VString *pat,
  const VString *rep,
  VString *dest
) {
  if (src == NULL || pat == NULL ||
      rep == NULL || dest == NULL)
    return VSTRING_ENOBUFFER;

  VString slice;
  bool eq = true;
  size_t start = 0, end = 0;
  for (size_t i = 0; i < VString_len(src); i++) {
    if (i + VString_len(pat) > VString_len(src))
      break;
    if (VString_at(src, i) == VString_at(pat, 0)) {
      eq = true;
      for (size_t j = 0; j < VString_len(pat); j++) {
        if (VString_at(src, j + i) != VString_at(pat, j)) {
          eq = false;
          break;
        }
      }

      if (eq) {
        end = i - start;
        slice = VString_slice(src, start, end);
        VString_append(dest, &slice);
        if (VString_len(rep) > 0)
          VString_append(dest, rep);
        i += VString_len(&slice) + VString_len(rep) - 1;
        VString_destroy(&slice);
      }
    }
  }

  if (start == end) {
    VString_copy(dest, src);
  }

  return VSTRING_OK;
}

char VString_at(const VString *buffer, size_t idx) {
  if (idx > buffer->_len || idx < 0)
    return 0;
  return buffer->_buffer[idx];
}

size_t VString_len(const VString *buffer) {
  return buffer->_len;
}

size_t VString_cap(const VString *buffer) {
  return buffer->_cap;
}

bool VString_eq(const VString *str1, const VString *str2) {
  if (str1 == NULL || str2 == NULL)
    return false;
  if (VString_len(str1) != VString_len(str2))
    return false;

  return memcmp(str1->_buffer, str2->_buffer, VString_len(str1)) == 0;

  return false;
}

VString VString_slice(
  const VString *buffer,
  size_t idx, size_t len
) {
  if (buffer == NULL || buffer->_buffer == NULL)
    return (VString){ 0 };
  if (buffer->_len == 0)
    return (VString){ 0 };

  VString ret_res;
  ret_res._buffer = malloc(len);
  if (ret_res._buffer == NULL)
    return (VString){ 0 };
  ret_res._len = len;
  ret_res._cap = len;
  memcpy(ret_res._buffer, buffer->_buffer + idx, len);
  return ret_res;
}

VString VString_from_bytes(const char *text) {
  if (text == NULL)
    return (VString){ 0 };

  VString buf;
  size_t len = strlen(text);
  buf._buffer = malloc(len);
  memcpy(buf._buffer, text, len);
  buf._cap = len;
  buf._len = len;

  return buf;
}

char *VString_to_cstr(const VString *buffer) {
  if (buffer == NULL || buffer->_buffer == NULL)
    return NULL;

  char *ret_res = malloc(buffer->_len + 1);
  if (ret_res == NULL)
    return NULL;

  memcpy(ret_res, buffer->_buffer, buffer->_len);
  ret_res[buffer->_len] = '\0';

  return ret_res;
}

VStringResult VString_destroy(VString *buffer) {
  if (buffer == NULL)
    return VSTRING_ALREADYFREED;

  free(buffer->_buffer);
  buffer->_buffer = NULL;
  buffer->_len = 0;
  buffer->_cap = 0;

  return VSTRING_OK;
}
