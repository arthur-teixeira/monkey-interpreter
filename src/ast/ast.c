#include "ast.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

Identifier *new_identifier(Token token, char *value) {
    Identifier *ident = malloc(sizeof(Identifier));

    if(ident == NULL) {
        printf("Error allocating identifier: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    ident->token = token;
    ident->value = value;

    return ident;
}

char *program_token_literal(Program *p) {
  if (p->statements->size == 0) {
    return "";
  }

  if (p->statements->tail == NULL) {
    assert(0 && "unreachable");
  }

  Statement *value = (Statement *)p->statements->tail->value;

  return value->token.literal;
}

Program *new_program(void) {
    Program *program = malloc(sizeof(Program));
    program->statements = new_list();

    return program;
}
