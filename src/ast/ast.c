#include "ast.h"
#include "../str_utils/str_utils.h"
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../dyn_array/dyn_array.h"

Identifier *new_identifier(Token token, char *value) {
  Identifier *ident = malloc(sizeof(Identifier));
  ident->type = IDENT_EXPR;
  if (ident == NULL) {
    printf("Error allocating identifier: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  ident->token = token;
  ident->value = strdup(value);

  return ident;
}

void free_identifier(Identifier *ident) {
  free(ident->value);
  free(ident);
}

Program *new_program(void) {
  Program *program = malloc(sizeof(Program));
  assert(program != NULL);
  array_init(&program->statements, 1);
  
  return program;
}

void free_program(Program *p) {
  array_free(&p->statements);
  free(p);
}

void ident_expr_to_string(ResizableBuffer *buf, Identifier *expr) {
  append_to_buf(buf, expr->value);
}

void block_to_string(ResizableBuffer *buf, BlockStatement *block) {
  for (uint32_t i = 0; i < block->statements.len; i++) {
      stmt_to_string(buf, block->statements.arr[i]);
  }
}

void if_to_string(ResizableBuffer *buf, IfExpression *expr) {
  append_to_buf(buf, "if");
  value_to_string(buf, expr->condition);
  append_to_buf(buf, " ");
  block_to_string(buf, expr->consequence);

  if (expr->alternative != NULL) {
    append_to_buf(buf, "else");
    block_to_string(buf, expr->alternative);
  }
}

void fn_to_string(ResizableBuffer *buf, FunctionLiteral *fn) {
  append_to_buf(buf, fn->token.literal);
  if (fn->name) {
    append_to_buf(buf, "<");
    append_to_buf(buf, fn->name);
    append_to_buf(buf, ">");
  }
  append_to_buf(buf, "(");

  uint32_t i;
  for (i = 0; i < fn->parameters.len - 1; i++) {
    Identifier *ident = fn->parameters.arr[i];
    append_to_buf(buf, ident->value);
    append_to_buf(buf, ", ");
  }

  Identifier *ident = fn->parameters.arr[++i];
  append_to_buf(buf, ident->value);

  append_to_buf(buf, ")");
}

void call_to_string(ResizableBuffer *buf, CallExpression *call) {
  value_to_string(buf, call->function);
  append_to_buf(buf, "(");

  for (uint32_t i = 0; i < call->arguments.len; i++) {
    value_to_string(buf, call->arguments.arr[i]);

    if (i < call->arguments.len - 1) {
      append_to_buf(buf, ", ");
    }
  }

  append_to_buf(buf, ")");
}

void string_literal_to_string(ResizableBuffer *buf, StringLiteral *str) {
  append_to_buf(buf, str->token.literal);
}

void array_to_string(ResizableBuffer *buf, ArrayLiteral *array_literal) {
  append_to_buf(buf, "[");

  for (size_t i = 0; i < array_literal->elements->len; i++) {
    value_to_string(buf, array_literal->elements->arr[i]);

    if (i < array_literal->elements->len - 1) {
      append_to_buf(buf, ", ");
    }
  }

  append_to_buf(buf, "]");
}

void index_expression_to_string(ResizableBuffer *buf, IndexExpression *expr) {
  append_to_buf(buf, "(");
  value_to_string(buf, expr->left);
  append_to_buf(buf, "[");
  value_to_string(buf, expr->index);
  append_to_buf(buf, "])");
}

int hash_literal_iterator(void *buf, hashmap_element_t *pair) {
  Expression *key = (void *)pair->key;
  value_to_string(buf, key);

  append_to_buf(buf, ":");

  Expression *value = pair->data;
  value_to_string(buf, value);

  append_to_buf(buf, ", ");

  return 1;
}

void hash_literal_to_string(ResizableBuffer *buf, HashLiteral *hash) {
  append_to_buf(buf, "{");
  hashmap_iterate_pairs(&hash->pairs, &hash_literal_iterator, &buf);
  append_to_buf(buf, "}");
}

void while_to_string(ResizableBuffer *buf, WhileLoop *loop) {
  append_to_buf(buf, "while (");
  value_to_string(buf, loop->condition);
  append_to_buf(buf, ") {");
  block_to_string(buf, loop->body);
  append_to_buf(buf, "}");
}

void for_to_string(ResizableBuffer *buf, ForLoop *loop) {
  append_to_buf(buf, "for (");

  if (loop->initialization != NULL) {
    stmt_to_string(buf, loop->initialization);
  }
  append_to_buf(buf, ";");

  if (loop->condition != NULL) {
    value_to_string(buf, loop->condition);
  }
  append_to_buf(buf, ";");

  if (loop->update != NULL) {
    stmt_to_string(buf, loop->update);
  }
  append_to_buf(buf, ") { ");
  block_to_string(buf, loop->body);
  append_to_buf(buf, "}");
}

void reassign_to_string(ResizableBuffer *buf, Reassignment *expr) {
  append_to_buf(buf, expr->name->value);
  append_to_buf(buf, " = ");
  value_to_string(buf, expr->value);
}

void value_to_string(ResizableBuffer *buf, Expression *expr) {
  switch (expr->type) {
  case IDENT_EXPR:
    return ident_expr_to_string(buf, (Identifier *)expr);
  case INT_EXPR:
    return int_to_string(buf, (NumberLiteral *)expr);
  case PREFIX_EXPR:
    return prefix_to_string(buf, (PrefixExpression *)expr);
  case INFIX_EXPR:
    return infix_to_string(buf, (InfixExpression *)expr);
  case BOOL_EXPR:
    return bool_to_string(buf, (BooleanLiteral *)expr);
  case IF_EXPR:
    return if_to_string(buf, (IfExpression *)expr);
  case FN_EXPR:
    return fn_to_string(buf, (FunctionLiteral *)expr);
  case CALL_EXPR:
    return call_to_string(buf, (CallExpression *)expr);
  case STRING_EXPR:
    return string_literal_to_string(buf, (StringLiteral *)expr);
  case ARRAY_EXPR:
    return array_to_string(buf, (ArrayLiteral *)expr);
  case INDEX_EXPR:
    return index_expression_to_string(buf, (IndexExpression *)expr);
  case HASH_EXPR:
    return hash_literal_to_string(buf, (HashLiteral *)expr);
  case WHILE_EXPR:
    return while_to_string(buf, (WhileLoop *)expr);
  case FOR_EXPR:
    return for_to_string(buf, (ForLoop *)expr);
  case REASSIGN_EXPR:
    assert(0);
  }
}

void let_to_string(ResizableBuffer *buf, Statement *stmt) {
  assert(stmt->name != NULL);

  append_to_buf(buf, stmt->token.literal);
  append_to_buf(buf, " ");
  append_to_buf(buf, stmt->name->value);
  append_to_buf(buf, " = ");

  if (stmt->expression != NULL) {
    value_to_string(buf, stmt->expression);
  }
}

void return_to_string(ResizableBuffer *buf, Statement *stmt) {
  append_to_buf(buf, stmt->token.literal);

  if (stmt->expression != NULL) {
    value_to_string(buf, stmt->expression);
  }
}

void expr_to_string(ResizableBuffer *buf, Statement *stmt) {
  if (stmt->expression != NULL) {
    value_to_string(buf, stmt->expression);
  }
}

void int_to_string(ResizableBuffer *buf, NumberLiteral *lit) {
  char formatted[10];
  sprintf(formatted, "%.2f", lit->value);

  append_to_buf(buf, formatted);
}

void prefix_to_string(ResizableBuffer *buf, PrefixExpression *expr) {
  append_to_buf(buf, "(");
  append_to_buf(buf, expr->operator);

  value_to_string(buf, expr->right);

  append_to_buf(buf, ")");
}

void infix_to_string(ResizableBuffer *buf, InfixExpression *expr) {
  append_to_buf(buf, "(");
  value_to_string(buf, expr->left);

  append_to_buf(buf, " ");
  append_to_buf(buf, expr->operator);
  append_to_buf(buf, " ");

  value_to_string(buf, expr->right);
  append_to_buf(buf, ")");
}

void bool_to_string(ResizableBuffer *buf, BooleanLiteral *expr) {
  append_to_buf(buf, expr->token.literal);
}

void continue_to_string(ResizableBuffer *buf) {
  append_to_buf(buf, "continue;");
}

void break_to_string(ResizableBuffer *buf) { append_to_buf(buf, "break;"); }

void stmt_to_string(ResizableBuffer *buf, Statement *stmt) {
  switch (stmt->type) {
  case LET_STATEMENT:
    return let_to_string(buf, stmt);
  case RETURN_STATEMENT:
    return return_to_string(buf, stmt);
  case EXPR_STATEMENT:
    return expr_to_string(buf, stmt);
  case BREAK_STATEMENT:
    return break_to_string(buf);
  case CONTINUE_STATEMENT:
    return continue_to_string(buf);
  }
}

void program_string(ResizableBuffer *buf, Program *p) {
  for (uint32_t i = 0; i < p->statements.len; i++ ) {
    stmt_to_string(buf, p->statements.arr[i]);
  }
  append_to_buf(buf, ";\n");
}
