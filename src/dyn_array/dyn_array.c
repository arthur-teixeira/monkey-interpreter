#include "./dyn_array.h"
#include <stdlib.h>

void array_append(DynamicArray *arr, void *value) {
  if (arr->cap == arr->len) {
    arr->cap *= 2;
    arr->arr = realloc(arr->arr, arr->cap);
  }

  arr->arr[arr->len++] = value;
}

void array_init(DynamicArray *arr, size_t initial_size) {
  arr->arr = calloc(initial_size, sizeof(void*));
  arr->cap = initial_size;
  arr->len = 0;
}

void array_free(DynamicArray *arr) {
  for (size_t i = 0; i < arr->len; i++) {
    free(arr->arr[i]);
  }
  free(arr->arr);
  arr->arr = NULL;
  arr->len = 0;
  arr->cap = 0;
}
