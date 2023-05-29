#ifndef STR_UTILS_H
#define STR_UTILS_H
#include <stddef.h>

typedef struct {
  size_t size;
  char *buf;
} ResizableBuffer;

void init_resizable_buffer(ResizableBuffer *buf, size_t initial_size);

void append_to_buf(ResizableBuffer *buf, char *src);

size_t strlcpy(char *, const char *, size_t);

#endif
