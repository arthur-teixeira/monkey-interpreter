#include "big_endian.h"
#include <stdint.h>

void big_endian_push_uint16(IntArray *arr, uint16_t num) {
  int_array_append(arr, (num >> 8) & 0xFF);
  int_array_append(arr, num & 0xFF);
}
