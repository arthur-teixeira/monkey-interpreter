#include "../dyn_array/dyn_array.h"
#include <endian.h>
#include <stddef.h>
#include <stdint.h>

void big_endian_to_uint16(uint16_t *, int8_t *);
void uint16_to_big_endian(uint16_t, uint8_t *);
void big_endian_push_uint16(IntArray *, uint16_t);
uint16_t big_endian_read_uint16(const IntArray *, size_t);
