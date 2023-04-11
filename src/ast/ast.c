#include "ast.h"
#include <assert.h>
#include <stdlib.h>

char *program_token_literal(Program *p) {
  if (p->size == 0) {
    return "";
  }

  if (p->statements[0] == NULL) {
    assert(0 && "unreachable");
  }

  return p->statements[0]->token.literal;
}
