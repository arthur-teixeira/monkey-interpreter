#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void append_to_buf(char *buf, char *src) {
  if (strlen(buf) + strlen(src) + 1 >= sizeof(buf)) {
    char *new_buf = realloc(buf, sizeof(buf) * 2);
    if (new_buf == NULL) {
      printf("ERROR: Failed to reallocate string buffer: %s\n",
             strerror(errno));
      exit(EXIT_FAILURE);
    }
    buf = new_buf;
  }

  strcat(buf, src);
}
