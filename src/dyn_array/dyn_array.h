#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint64_t cap;
  uint64_t len;
  void **arr;
} DynamicArray;

void array_append(DynamicArray *, void *);

void array_init(DynamicArray *, size_t);

void array_free(DynamicArray *);

#endif
