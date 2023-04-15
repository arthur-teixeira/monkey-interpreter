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

Parser *new_parser(Lexer *l) {
  Parser *p = malloc(sizeof(Parser));
  p->l = l;
  p->errors = new_list();
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

  // TODO: how to free this afterwards?
  Token *copied_token = malloc(sizeof(Token));
  if (copied_token == NULL) {
    printf("ERROR: Could not copy token to parse: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  memcpy(copied_token, &p->cur_token, sizeof(Token));
  stmt->name = new_identifier(*copied_token, copied_token->literal);

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
  stmt->type = RETURN_STATEMENT;
  if (stmt == NULL) {
    printf("ERROR: Could not create statement: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  stmt->token = p->cur_token;
  stmt->name = NULL;

  parser_next_token(p);
  // TODO: Skipping expressions until we find a semicolon
  while (!cur_token_is(p, SEMICOLON)) {
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
    return NULL;
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
