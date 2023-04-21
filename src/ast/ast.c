#include "ast.h"
#include <assert.h>
#include <stdint.h>
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

void program_token_literal(char *buf, Program *p) {
  if (p->statements->size == 0) {
    return;
  }
  assert(p->statements->tail != NULL);

  Statement *value = (Statement *)p->statements->tail->value;

  append_to_buf(buf, value->token.literal);
}

Program *new_program(void) {
  Program *program = malloc(sizeof(Program));
  program->statements = new_list();

  return program;
}

void free_program(Program *p) {
  free_list(p->statements);
  free(p);
}

void ident_expr_to_string(char *buf, Identifier *expr) {
  append_to_buf(buf, expr->value);
}

void value_to_string(char *buf, Expression *expr) {
  switch (expr->type) {
  case IDENT_EXPR:
    return ident_expr_to_string(buf, expr->value);
  case INT_EXPR:
    return int_to_string(buf, expr->value);
  case PREFIX_EXPR:
    return prefix_to_string(buf, expr->value);
  }
}

void let_to_string(char *buf, Statement *stmt) {
  assert(stmt->name != NULL);

  char formatted[255];
  sprintf(formatted, "%s %s = ", stmt->token.literal, stmt->name->value);
  append_to_buf(buf, formatted);

  if (stmt->expression != NULL) {
    value_to_string(buf, stmt->expression);
  }

  append_to_buf(buf, ";\n");
}

void return_to_string(char *buf, Statement *stmt) {
  append_to_buf(buf, stmt->token.literal);

  if (stmt->expression != NULL) {
    value_to_string(buf, stmt->expression);
  }

  append_to_buf(buf, ";\n");
}

void expr_to_string(char *buf, Statement *stmt) {
  if (stmt->expression != NULL) {
    value_to_string(buf, stmt->expression);
  }
  append_to_buf(buf, ";\n");
}

void int_to_string(char *buf, IntegerLiteral *lit) {
  char formatted[sizeof(long)];
  sprintf(formatted, "%ld", lit->value);

  append_to_buf(buf, formatted);
}

void prefix_to_string(char * buf, PrefixExpression *expr) {
  char formatted[MAX_LEN];
  sprintf(formatted, "(%s", expr->operator);
  append_to_buf(buf, formatted);

  value_to_string(buf, expr->right);

  append_to_buf(buf, ")");
}

void stmt_to_string(char *buf, Statement *stmt) {
  switch (stmt->type) {
  case LET_STATEMENT:
    return let_to_string(buf, stmt);
  case RETURN_STATEMENT:
    return return_to_string(buf, stmt);
  case EXPR_STATEMENT:
    return expr_to_string(buf, stmt);
  }
}

void program_string(char *buf, Program *p) {
  Node *cur = p->statements->tail;
  while (cur != NULL) {
    Statement *cur_stmt = cur->value;
    stmt_to_string(buf, cur_stmt);
    cur = cur->next;
  }
}
