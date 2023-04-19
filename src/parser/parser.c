#include "parser.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parser_next_token(Parser *p) {
  p->cur_token = p->peek_token;
  p->peek_token = next_token(p->l);
}

void register_infix_fn(Parser *p, infix_parse_fn fn, enum TokenType token) {
  p->infix_parse_fns[token] = fn;
}

void register_prefix_fn(Parser *p, prefix_parse_fn fn, enum TokenType token) {
  p->prefix_parse_fns[token] = fn;
}

Expression *parse_identifier(Parser *p) {
  Expression *expr = malloc(sizeof(Expression));
  expr->type = IDENT_EXPR;
  Identifier *ident = new_identifier(p->cur_token, p->cur_token.literal);
  expr->value = ident;

  return expr;
}

void int_conversion_error(Parser *p) {
  char *err_msg = malloc(255);
  sprintf(err_msg, "Could not parse %s to an integer", p->cur_token.literal);

  append(p->errors, err_msg);
}

IntegerLiteral *new_int_literal(Parser *p) {
  IntegerLiteral *lit = malloc(sizeof(IntegerLiteral));
  if (lit == NULL) {
    printf("Error allocating identifier value: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  char *err = "";
  int64_t result = strtol(p->cur_token.literal, &err, 10);
  if (strcmp(err, "") != 0) {
    int_conversion_error(p);
    free(lit);
    return NULL;
  }

  lit->value = result;
  lit->token = p->cur_token;

  return lit;
}

Expression *parse_integer_literal(Parser *p) {
  Expression *expr = malloc(sizeof(Expression));
  expr->type = INT_EXPR;
  
  IntegerLiteral *lit = new_int_literal(p);
  expr->value = lit;

  return expr;
}

Parser *new_parser(Lexer *l) {
  Parser *p = malloc(sizeof(Parser));
  p->l = l;
  p->errors = new_list();

  for (uint32_t i = 0; i < TOKEN_COUNT; i++) {
    p->prefix_parse_fns[i] = NULL;
    p->infix_parse_fns[i] = NULL;
  }

  register_prefix_fn(p, &parse_identifier, IDENT);
  register_prefix_fn(p, &parse_integer_literal, INT);

  parser_next_token(p);
  parser_next_token(p);

  return p;
}

void free_parser(Parser *p) {
  free_lexer(p->l);
  free_list(p->errors);
  free(p);
}

void peek_error(Parser *p, enum TokenType t) {
  char *err_msg = malloc(50);
  sprintf(err_msg, "Expected next token to be %s, got %s.\n", TOKEN_STRING[t],
          TOKEN_STRING[p->peek_token.Type]);

  append(p->errors, err_msg);
}

bool cur_token_is(Parser *p, enum TokenType t) {
  return p->cur_token.Type == t;
}

bool peek_token_is(Parser *p, enum TokenType t) {
  return p->peek_token.Type == t;
}

bool expect_peek(Parser *p, enum TokenType t) {
  if (peek_token_is(p, t)) {
    parser_next_token(p);
    return true;
  }

  peek_error(p, t);
  return false;
}

Statement *parse_let_statement(Parser *p) {
  Statement *stmt = malloc(sizeof(Statement));
  stmt->type = LET_STATEMENT;

  if (stmt == NULL) {
    printf("ERROR: Could not create statement: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  stmt->token = p->cur_token;

  if (!expect_peek(p, IDENT)) {
    return NULL;
  }

  Token copied_token;

  memcpy(&copied_token, &p->cur_token, sizeof(Token));
  stmt->name = new_identifier(copied_token, copied_token.literal);

  if (!expect_peek(p, ASSIGN)) {
    return NULL;
  }

  // TODO: Skipping expressions until we find a semicolon
  while (!cur_token_is(p, SEMICOLON)) {
    parser_next_token(p);
  }

  return stmt;
}

Statement *parse_return_statement(Parser *p) {
  Statement *stmt = malloc(sizeof(Statement));
  if (stmt == NULL) {
    printf("ERROR: Could not create statement: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  stmt->type = RETURN_STATEMENT;
  stmt->token = p->cur_token;
  stmt->name = NULL;

  parser_next_token(p);
  // TODO: Skipping expressions until we find a semicolon
  while (!cur_token_is(p, SEMICOLON)) {
    parser_next_token(p);
  }

  return stmt;
}

Expression *parse_expression(Parser *p, OperatorPrecedenceOrder precedence) {
  prefix_parse_fn prefix = p->prefix_parse_fns[p->cur_token.Type];
  if (prefix == NULL) {
    return NULL;
  }

  return prefix(p);
}

Statement *parse_expression_statement(Parser *p) {
  Statement *stmt = malloc(sizeof(Statement));
  if (stmt == NULL) {
    printf("ERROR: Could not create statement: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  stmt->type = EXPR_STATEMENT;
  stmt->token = p->cur_token;
  stmt->expression = parse_expression(p, LOWEST);
  stmt->name = NULL;

  if (peek_token_is(p, SEMICOLON)) {
    parser_next_token(p);
  }

  return stmt;
}

Statement *parse_statement(Parser *p) {
  switch (p->cur_token.Type) {
  case LET:
    return parse_let_statement(p);
  case RETURN:
    return parse_return_statement(p);
  default:
    return parse_expression_statement(p);
  }
}

Program *parse_program(Parser *p) {
  Program *program = new_program();

  for (uint32_t i = 0; !cur_token_is(p, END_OF_FILE); i++) {
    Statement *stmt = parse_statement(p);
    if (stmt != NULL) {
      append(program->statements, stmt);
    }
    parser_next_token(p);
  }

  return program;
}
