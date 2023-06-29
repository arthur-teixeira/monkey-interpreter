#include "./dyn_array.h"
#include <stdlib.h>

void array_append(DynamicArray *arr, void *value) {
  if (arr->cap == arr->len) {
    arr->cap *= 2;
    arr->arr = realloc(arr->arr, arr->cap * sizeof(void*));
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

void int_array_append(IntArray *arr, int value) {
    if (arr->cap == arr->len) {
        arr->cap *= 2;
        arr->arr = realloc(arr->arr, arr->cap * sizeof(int));
    }
    
    arr->arr[arr->len++] = value;
}

void int_array_init(IntArray *arr, size_t initial_size) {
    arr->arr = calloc(initial_size, sizeof(int));
    arr->cap = initial_size;
    arr->len = 0;
}

void int_array_free(IntArray *arr) {
    free(arr->arr);
    arr->arr = NULL;
    arr->len = 0;
    arr->cap = 0;

}
