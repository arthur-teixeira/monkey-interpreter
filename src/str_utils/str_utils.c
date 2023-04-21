#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a > _b ? _a : _b;                                                         \
  })

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

void append_to_buf(char *buf, char *src) {
  if (strlen(buf) + strlen(src) + 1 >= sizeof(buf)) {
    size_t new_size = max(sizeof(buf) + sizeof(src), sizeof(buf) *2);
    char *new_buf = realloc(buf, new_size);
    if (new_buf == NULL) {
      printf("ERROR: Failed to reallocate string buffer: %s\n",
             strerror(errno));
      exit(EXIT_FAILURE);
    }
    buf = new_buf;
  }

  strcat(buf, src);
}
