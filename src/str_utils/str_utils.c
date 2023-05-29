#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str_utils.h"

// Stolen from
// https://android.googlesource.com/platform/system/core.git/+/master/libcutils/strlcpy.c
// Not using strncpy because it does not null-terminate the string,
// but converts a string into a raw character buffer. More can be found at
// https://devblogs.microsoft.com/oldnewthing/20050107-00/?p=36773
size_t strlcpy(char *dst, const char *src, size_t siz) {
  char *d = dst;
  const char *s = src;
  size_t n = siz;
  /* Copy as many bytes as will fit */
  if (n != 0) {
    while (--n != 0) {
      if ((*d++ = *s++) == '\0')
        break;
    }
  }
  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (siz != 0)
      *d = '\0'; /* NUL-terminate dst */
    while (*s++)
      ;
  }
  return (s - src - 1); /* count does not include NUL */
}

size_t max(size_t a, size_t b) {
  if (a > b) {
    return a;
  }

  return b;
}

void init_resizable_buffer(ResizableBuffer *buf, size_t initial_size) {
  buf->size = initial_size;
  buf->buf = malloc(initial_size * sizeof(char));
  buf->buf[0] = '\0';
}

void append_to_buf(ResizableBuffer *buf, char *src) {
  size_t buf_len_after_append = strlen(buf->buf) + strlen(src) + 1;

  if (buf_len_after_append > buf->size) {
    buf->size = max(buf->size * 2, buf_len_after_append);
    char *new_buf = realloc(buf->buf, buf->size);
    if (new_buf == NULL) {
      printf("ERROR: Failed to reallocate string buffer: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    buf->buf = new_buf;

  }
  strcat(buf->buf, src);
}
