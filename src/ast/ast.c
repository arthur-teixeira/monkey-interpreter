#include "ast.h"
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../str_utils/str_utils.h"

Identifier *new_identifier(Token token, char *value) {
  Identifier *ident = malloc(sizeof(Identifier));
  if (ident == NULL) {
    printf("Error allocating identifier: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  ident->value = malloc(sizeof(value));
  if (ident == NULL) {
    printf("Error allocating identifier value: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  ident->token = token;
  strlcpy(ident->value, value, sizeof(ident->value));

  return ident;
}

void free_identifier(Identifier *ident) {
  free(ident->value);
  free(ident);
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

char *ident_expr_to_string(Identifier *expr) { return expr->value; }

char *value_to_string(Expression *expr) {
  switch (expr->type) {
  case IDENT_EXPR:
    return ident_expr_to_string(expr->value);
  default:
    return "TODO: value_to_string\n";
    assert(0 && "unreachable");
  }
}

char *let_to_string(Statement *stmt) {
  if (stmt->name == NULL) {
    assert(0 && "unreachable");
  }

  char *buf = malloc(2 * MAX_LEN);
  sprintf(buf, "%s %s = ", stmt->token.literal, stmt->name->value);

  if (stmt->value != NULL) {
    append_to_buf(buf, value_to_string(stmt->value));
  }

  append_to_buf(buf, ";\n");

  return buf;
}

char *return_to_string(Statement *stmt) {
  char *buf = malloc(MAX_LEN);

  append_to_buf(buf, stmt->token.literal);

  if (stmt->value != NULL) {
    append_to_buf(buf, value_to_string(stmt->value));
  }

  append_to_buf(buf, ";\n");
  return buf;
}

char *expr_to_string(Statement *stmt) {
  char *buf = malloc(MAX_LEN);

  if (stmt->value != NULL) {
    append_to_buf(buf, value_to_string(stmt->value));
  }

  append_to_buf(buf, ";\n");
  return buf;
}

char *stmt_to_string(Statement *stmt) {
  switch (stmt->type) {
  case LET_STATEMENT:
    return let_to_string(stmt);
  case RETURN_STATEMENT:
    return return_to_string(stmt);
  case EXPR_STATEMENT:
    return expr_to_string(stmt);
  default:
    assert(0 && "unreachable");
    return "";
  }
}

char *program_string(Program *p) {
  char *buf = malloc(MAX_LEN);

  Node *cur = p->statements->tail;
  while (cur != NULL) {
    Statement *cur_stmt = cur->value;
    append_to_buf(buf, stmt_to_string(cur_stmt));
    cur = cur->next;
  }

  return buf;
}
