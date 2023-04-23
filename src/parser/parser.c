#include "parser.h"
#include "../str_utils/str_utils.h"
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

void build_precedence_table(Parser *p) {
  p->precedences[EQ] = EQUALS;
  p->precedences[NOT_EQ] = EQUALS;
  p->precedences[LT] = LESSGREATER;
  p->precedences[GT] = LESSGREATER;
  p->precedences[PLUS] = SUM;
  p->precedences[MINUS] = SUM;
  p->precedences[SLASH] = PRODUCT;
  p->precedences[ASTERISK] = PRODUCT;
}

uint32_t peek_precedence(Parser *p) {
  return p->precedences[p->peek_token.Type];
}

uint32_t cur_precedence(Parser *p) { return p->precedences[p->cur_token.Type]; }

bool cur_token_is(Parser *p, enum TokenType t) {
  return p->cur_token.Type == t;
}

bool peek_token_is(Parser *p, enum TokenType t) {
  return p->peek_token.Type == t;
}

void peek_error(Parser *p, enum TokenType t) {
  char *err_msg = malloc(50);
  sprintf(err_msg, "Expected next token to be %s, got %s.\n", TOKEN_STRING[t],
          TOKEN_STRING[p->peek_token.Type]);

  append(p->errors, err_msg);
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

void no_prefix_parse_fn_error(Parser *p, enum TokenType type) {
  char *msg = malloc(255);
  sprintf(msg, "No prefix parse function for %s found", TOKEN_STRING[type]);
  append(p->errors, msg);
}

Expression *parse_expression(Parser *p, OperatorPrecedenceOrder precedence) {
  prefix_parse_fn prefix = p->prefix_parse_fns[p->cur_token.Type];
  if (prefix == NULL) {
    no_prefix_parse_fn_error(p, p->cur_token.Type);
    return NULL;
  }

  Expression *left_exp = prefix(p);

  while (!peek_token_is(p, SEMICOLON) && precedence < peek_precedence(p)) {
    infix_parse_fn infix = p->infix_parse_fns[p->peek_token.Type];
    if (infix == NULL) {
      return left_exp;
    }

    parser_next_token(p);

    left_exp = infix(p, left_exp);
  }

  return left_exp;
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

PrefixExpression *new_prefix_expression(Parser *p) {
  PrefixExpression *prefix = malloc(sizeof(PrefixExpression));
  if (prefix == NULL) {
    printf("ERROR: Could not create prefix expression: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  prefix->operator= malloc(sizeof(p->cur_token.literal));
  strlcpy(prefix->operator, p->cur_token.literal, sizeof(prefix->operator));
  prefix->token = p->cur_token;

  parser_next_token(p);
  prefix->right = parse_expression(p, PREFIX);

  return prefix;
}

Expression *parse_prefix_expression(Parser *p) {
  Expression *expr = malloc(sizeof(Expression));
  if (expr == NULL) {
    printf("ERROR: Could not create expression: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  expr->type = PREFIX_EXPR;
  expr->value = new_prefix_expression(p);

  return expr;
}

Boolean *new_boolean_expression(Parser *p) {
  Boolean *expr = malloc(sizeof(Boolean));
  if (expr == NULL) {
    printf("ERROR: Could not create boolean expression: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  expr->token = p->cur_token;
  expr->value = cur_token_is(p, TRUE);

  return expr;
}

Expression *parse_boolean(Parser *p) {
  Expression *expr = malloc(sizeof(Expression));
  if (expr == NULL) {
    printf("ERROR: Could not create expression: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  expr->type = BOOL_EXPR;
  expr->value = new_boolean_expression(p);

  return expr;
}

Expression *parse_grouped_expression(Parser *p) {
  parser_next_token(p);

  Expression *expr = parse_expression(p, LOWEST);

  if (!expect_peek(p, RPAREN)) {
    free(expr);
    return NULL;
  }

  return expr;
}

BlockStatement *parse_block_statement(Parser *p) {
  BlockStatement *block = malloc(sizeof(BlockStatement));
  if (block == NULL) {
    printf("ERROR: Could not create block statement: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  block->token = p->cur_token;
  block->statements = new_list();

  parser_next_token(p);

  while(!cur_token_is(p, RBRACE) && !cur_token_is(p, END_OF_FILE)) {
    Statement *stmt = parse_statement(p);
    if (stmt != NULL) {
      append(block->statements, stmt);
    }
    parser_next_token(p);
  }

  return block;
}

IfExpression *new_if_expression(Parser *p) {
  IfExpression *expr = malloc(sizeof(IfExpression));
  if (expr == NULL) {
    printf("ERROR: Could not create if expression: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (!expect_peek(p, LPAREN)) {
    free(expr);
    return NULL;
  }

  parser_next_token(p);
  expr->condition = parse_expression(p, LOWEST);

  if (!expect_peek(p, RPAREN)) {
    free(expr->condition->value);
    free(expr->condition);
    free(expr);
    return NULL;
  }

  if(!expect_peek(p, LBRACE)) {
    free(expr->condition->value);
    free(expr->condition);
    free(expr);
    return NULL;
  }

  expr->consequence = parse_block_statement(p);

  if (!peek_token_is(p, ELSE)) {
    return expr;
  }

  parser_next_token(p);
  if(!expect_peek(p, LBRACE)) {
    free(expr->condition->value);
    free(expr->condition);
    free_list(expr->consequence->statements);
    free(expr);

    return NULL;
  }

  expr->alternative = parse_block_statement(p);

  return expr;
}

Expression *parse_if_expression(Parser *p) {
  Expression *expr = malloc(sizeof(Expression));
  if (expr == NULL) {
    printf("ERROR: Could not create expression: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  expr->type = IF_EXPR;

  IfExpression *iff = new_if_expression(p);

  expr->value = iff;

  return expr;
}

InfixExpression *new_infix_expression(Parser *p) {
  InfixExpression *infix = malloc(sizeof(InfixExpression));
  if (infix == NULL) {
    printf("ERROR: Could not create infix expression: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  infix->operator= malloc(sizeof(p->cur_token.literal));
  strlcpy(infix->operator, p->cur_token.literal, sizeof(infix->operator));

  infix->token = p->cur_token;

  return infix;
}

Expression *parse_infix_expression(Parser *p, Expression *left) {
  Expression *expr = malloc(sizeof(Expression));
  if (expr == NULL) {
    printf("ERROR: Could not create expression: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  expr->type = INFIX_EXPR;
  InfixExpression *infix_expr = new_infix_expression(p);

  uint32_t precedence = cur_precedence(p);
  parser_next_token(p);

  infix_expr->left = left;
  infix_expr->right = parse_expression(p, precedence);

  expr->value = infix_expr;

  return expr;
}

Parser *new_parser(Lexer *l) {
  Parser *p = malloc(sizeof(Parser));
  p->l = l;
  p->errors = new_list();

  for (uint32_t i = 0; i < TOKEN_COUNT; i++) {
    p->precedences[i] = LOWEST;

    p->prefix_parse_fns[i] = NULL;
    p->infix_parse_fns[i] = NULL;
  }

  register_prefix_fn(p, &parse_identifier, IDENT);
  register_prefix_fn(p, &parse_integer_literal, INT);
  register_prefix_fn(p, &parse_prefix_expression, BANG);
  register_prefix_fn(p, &parse_prefix_expression, MINUS);
  register_prefix_fn(p, &parse_boolean, TRUE);
  register_prefix_fn(p, &parse_boolean, FALSE);
  register_prefix_fn(p, &parse_grouped_expression, LPAREN);
  register_prefix_fn(p, &parse_if_expression, IF);

  register_infix_fn(p, &parse_infix_expression, PLUS);
  register_infix_fn(p, &parse_infix_expression, MINUS);
  register_infix_fn(p, &parse_infix_expression, SLASH);
  register_infix_fn(p, &parse_infix_expression, ASTERISK);
  register_infix_fn(p, &parse_infix_expression, EQ);
  register_infix_fn(p, &parse_infix_expression, NOT_EQ);
  register_infix_fn(p, &parse_infix_expression, LT);
  register_infix_fn(p, &parse_infix_expression, GT);

  build_precedence_table(p);

  parser_next_token(p);
  parser_next_token(p);

  return p;
}

void free_parser(Parser *p) {
  free_lexer(p->l);
  free_list(p->errors);
  free(p);
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
