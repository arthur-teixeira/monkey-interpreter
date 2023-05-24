#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const unsigned initial_size = 2;

const char *TOKEN_STRING[] = {
    FOREACH_TOKEN_TYPE(GENERATE_STRING)
};

//TODO: Implement with hash_map
TokenType lookup_ident(char *ident) {
  if (strcmp(ident, "fn") == 0) {
    return FUNCTION;
  }

  if (strcmp(ident, "let") == 0) {
    return LET;
  }

  if(strcmp(ident, "true") == 0) {
      return TRUE;
  }

  if(strcmp(ident, "false") == 0) {
      return FALSE;
  }

  if(strcmp(ident, "false") == 0) {
      return FALSE;
  }

  if(strcmp(ident, "if") == 0) {
      return IF;
  }

  if(strcmp(ident, "else") == 0) {
      return ELSE;
  }

  if(strcmp(ident, "return") == 0) {
      return RETURN;
  }

  return IDENT;
}
