#include <endian.h>
#include <stddef.h>
#include <stdint.h>
#include "../dyn_array/dyn_array.h"

void big_endian_push_uint16(IntArray *, uint16_t);
uint16_t big_endian_read_uint16(const IntArray *arr, size_t);
