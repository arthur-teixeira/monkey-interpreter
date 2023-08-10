#include "big_endian.h"
#include <assert.h>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>

void big_endian_as_uint16(uint16_t num, uint8_t* buf) {
  buf[0] = (num >> 8) & 0xFF;
  buf[1] = num & 0xFF;
}

void big_endian_push_uint16(IntArray *arr, uint16_t num) {
  int_array_append(arr, (num >> 8) & 0xFF);
  int_array_append(arr, num & 0xFF);
}

uint16_t big_endian_read_uint16(const IntArray *arr, size_t offset) {
    assert(arr->len >= 2 + offset);
    return (arr->arr[offset] << 8) | arr->arr[offset + 1];
}
