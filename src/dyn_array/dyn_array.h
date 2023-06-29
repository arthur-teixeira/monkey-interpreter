#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  size_t cap;
  size_t len;
  void **arr;
} DynamicArray;

typedef struct {
  uint64_t cap;
  uint64_t len;
  int *arr;
} IntArray;

void int_array_append(IntArray *, int);
void int_array_init(IntArray *, size_t);
void int_array_free(IntArray *);

void array_append(DynamicArray *, void *);

void array_init(DynamicArray *, size_t);

void array_free(DynamicArray *);

#endif
