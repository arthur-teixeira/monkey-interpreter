#include "ast.h"
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Identifier *new_identifier(Token token, char *value) {
  Identifier *ident = malloc(sizeof(Identifier));

  if (ident == NULL) {
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

FILE *open_stream(char **buf) {
  size_t size;
  FILE *stream = open_memstream(buf, &size);
  if (stream == NULL) {
    printf("ERROR: could not open stream: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  return stream;
}

void flush_stream(FILE *stream) {
  int result = fflush(stream);
  if (result == EOF) {
    printf("ERROR: could not write to stream: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void close_stream(FILE *stream) {
  int result = fclose(stream);
  if (result == EOF) {
    printf("ERROR: could not close stream: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

char *ident_expr_to_string(Identifier *expr) {
  return expr->value;
}

char *value_to_string(Expression *expr) {
  switch (expr->type) {
    case IDENT_EXPR:
      return ident_expr_to_string(expr->value);
    default:
      assert(0 && "unreachable");
      return "";
  
  }
  return "TODO: value_to_string\n";
}

char *let_to_string(Statement *stmt) {
  if (stmt->name == NULL) {
    assert(0 && "unreachable");
  }

  char *buf = "";
  FILE *stream = open_stream(&buf);

  fprintf(stream, "%s %s = ", stmt->token.literal, stmt->name->value);
  if (stmt->value != NULL) {
    fprintf(stream, "%s", value_to_string(stmt->value));
  }

  fprintf(stream, ";\n");
  flush_stream(stream);
  close_stream(stream);
  return buf;
}

char *return_to_string(Statement *stmt) {
  char *buf = "";

  FILE *stream = open_stream(&buf);
  fprintf(stream, "%s ", stmt->token.literal);

  if (stmt->value != NULL) {
    fprintf(stream, "%s", value_to_string(stmt->value));
  }

  fprintf(stream, ";\n");
  flush_stream(stream);
  close_stream(stream);
  return buf;
}

char *expr_to_string(Statement *stmt) {
  char *buf = "";

  FILE *stream = open_stream(&buf);
  if (stmt->value != NULL) {
    fprintf(stream, "%s ", value_to_string(stmt->value));
  }

  fprintf(stream, ";\n");
  flush_stream(stream);
  close_stream(stream);
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
  char *buf = "";

  FILE *stream = open_stream(&buf);

  Node *cur = p->statements->tail;
  while (cur != NULL) {
    Statement *cur_stmt = cur->value;

    fprintf(stream, "%s", stmt_to_string(cur_stmt));
    flush_stream(stream);

    cur = cur->next;
  }

  close_stream(stream);

  return buf;
}
