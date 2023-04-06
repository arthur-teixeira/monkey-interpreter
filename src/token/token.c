#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const unsigned initial_size = 2;

TokenType lookup_ident(char *ident) {
  if (strcmp(ident, "fn") == 0) {
    return FUNCTION;
  }

  if (strcmp(ident, "let") == 0) {
    return LET;
  }

  return IDENT;
}
