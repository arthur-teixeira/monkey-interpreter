#include "parser.h"
#include "../dyn_array/dyn_array.h"
#include "../str_utils/str_utils.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void parser_next_token(Parser *p) {
  p->cur_token = p->peek_token;
  p->peek_token = next_token(p->l);
}

void register_infix_fn(Parser *p, infix_parse_fn fn, TokenType token) {
  p->infix_parse_fns[token] = fn;
}

void register_prefix_fn(Parser *p, prefix_parse_fn fn, TokenType token) {
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
  p->precedences[LPAREN] = CALL;
  p->precedences[LBRACKET] = INDEX;
}

uint32_t peek_precedence(Parser *p) {
  return p->precedences[p->peek_token.Type];
}

uint32_t cur_precedence(Parser *p) { return p->precedences[p->cur_token.Type]; }

bool cur_token_is(Parser *p, TokenType t) { return p->cur_token.Type == t; }

bool peek_token_is(Parser *p, TokenType t) { return p->peek_token.Type == t; }

void peek_error(Parser *p, TokenType t) {
  char *err_msg = malloc(50);
  sprintf(err_msg, "Expected next token to be %s, got %s.\n", TOKEN_STRING[t],
          TOKEN_STRING[p->peek_token.Type]);

  append(p->errors, err_msg);
}

bool expect_peek(Parser *p, TokenType t) {
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

  parser_next_token(p);

  stmt->expression = parse_expression(p, LOWEST);
  parser_next_token(p);

  if (peek_token_is(p, SEMICOLON)) {
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

  stmt->expression = parse_expression(p, LOWEST);
  parser_next_token(p);

  if (peek_token_is(p, SEMICOLON)) {
    parser_next_token(p);
  }

  return stmt;
}

void no_prefix_parse_fn_error(Parser *p, TokenType type) {
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

void illegal_statement_error(Parser *p, char *stmt_type) {
  char *err_msg = malloc(MAX_LEN);
  snprintf(err_msg, MAX_LEN, "Illegal %s statement", stmt_type);

  append(p->errors, err_msg);
}

static bool INSIDE_LOOP = false;

Statement *parse_continue_statement(Parser *p) {
  if (!INSIDE_LOOP) {
    illegal_statement_error(p, "continue");
    return NULL;
  }

  Statement *stmt = malloc(sizeof(Statement));
  assert(stmt != NULL);

  stmt->type = CONTINUE_STATEMENT;
  stmt->token = p->cur_token;
  stmt->expression = NULL;
  stmt->name = NULL;

  parser_next_token(p);
  if (peek_token_is(p, SEMICOLON)) {
    parser_next_token(p);
  }

  return stmt;
}

Statement *parse_break_statement(Parser *p) {
  if (!INSIDE_LOOP) {
    illegal_statement_error(p, "break");
    return NULL;
  }

  Statement *stmt = malloc(sizeof(Statement));
  assert(stmt != NULL);

  stmt->type = BREAK_STATEMENT;
  stmt->token = p->cur_token;
  stmt->expression = NULL;
  stmt->name = NULL;

  parser_next_token(p);
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
  case BREAK:
    return parse_break_statement(p);
  case CONTINUE:
    return parse_continue_statement(p);
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
  char *err_msg = malloc(MAX_LEN + 50);
  snprintf(err_msg, MAX_LEN + 50, "Could not parse %s to an integer",
           p->cur_token.literal);

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

BooleanLiteral *new_boolean_expression(Parser *p) {
  BooleanLiteral *expr = malloc(sizeof(BooleanLiteral));
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

  while (!cur_token_is(p, RBRACE) && !cur_token_is(p, END_OF_FILE)) {
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

  if (!expect_peek(p, LBRACE)) {
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
  if (!expect_peek(p, LBRACE)) {
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
  if (iff == NULL) {
    free(expr);
    return NULL;
  }
  expr->value = iff;

  return expr;
}

LinkedList *parse_function_parameters(Parser *p) {
  LinkedList *parameters = new_list();

  if (peek_token_is(p, RPAREN)) {
    parser_next_token(p);
    return parameters;
  }

  parser_next_token(p);

  Identifier *ident = new_identifier(p->cur_token, p->cur_token.literal);
  append(parameters, ident);

  while (peek_token_is(p, COMMA)) {
    parser_next_token(p);
    parser_next_token(p);
    Identifier *ident = new_identifier(p->cur_token, p->cur_token.literal);
    append(parameters, ident);
  }

  if (!expect_peek(p, RPAREN)) {
    free_list(parameters);
    return NULL;
  }

  return parameters;
}

FunctionLiteral *new_function_literal(Parser *p) {
  FunctionLiteral *fn = malloc(sizeof(FunctionLiteral));
  if (fn == NULL) {
    printf("ERROR: Could not create function literal: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  fn->token = p->cur_token;

  if (!expect_peek(p, LPAREN)) {
    free(fn);
    return NULL;
  }

  LinkedList *parameters = parse_function_parameters(p);
  if (parameters == NULL) {
    free(fn);
    return NULL;
  }

  fn->parameters = parameters;

  if (!expect_peek(p, LBRACE)) {
    free_list(fn->parameters);
    free(fn);

    return NULL;
  }

  fn->body = parse_block_statement(p);
  return fn;
}

Expression *parse_function_literal(Parser *p) {
  Expression *expr = malloc(sizeof(Expression));
  if (expr == NULL) {
    printf("ERROR: Could not create expression: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  expr->type = FN_EXPR;

  FunctionLiteral *value = new_function_literal(p);

  if (value == NULL) {
    free(expr);
    return NULL;
  }

  expr->value = value;

  return expr;
}

StringLiteral *new_string_literal(Parser *p) {
  StringLiteral *str = malloc(sizeof(StringLiteral));
  assert(str != NULL && "error allocating memory for string literal");

  str->token = p->cur_token;
  str->len = strlen(p->cur_token.literal);
  str->value = malloc(str->len + 1); // adding one to count for null terminator
  assert(str->value != NULL &&
         "error allocating memory for string literal value");
  strlcpy(str->value, p->cur_token.literal, str->len + 1);

  return str;
}

Expression *parse_string_literal(Parser *p) {
  Expression *expr = malloc(sizeof(Expression));
  assert(expr != NULL && "Error allocating memory for string expression");
  expr->type = STRING_EXPR;

  StringLiteral *str = new_string_literal(p);
  expr->value = str;

  return expr;
}

ArrayLiteral *new_array_literal(Parser *p) {
  ArrayLiteral *arr = malloc(sizeof(ArrayLiteral));
  arr->token = p->cur_token;
  arr->elements = malloc(sizeof(DynamicArray));
  array_init(arr->elements, 10);

  if (peek_token_is(p, RBRACKET)) {
    parser_next_token(p);
    return arr;
  }

  parser_next_token(p);
  array_append(arr->elements, parse_expression(p, LOWEST));

  while (peek_token_is(p, COMMA)) {
    parser_next_token(p);
    parser_next_token(p);
    array_append(arr->elements, parse_expression(p, LOWEST));
  };

  if (!expect_peek(p, RBRACKET)) {
    array_free(arr->elements);
    return NULL;
  }

  return arr;
}

Expression *parse_array_literal(Parser *p) {
  Expression *expr = malloc(sizeof(Expression));
  assert(expr != NULL && "Error allocating memory for array expression");
  expr->type = ARRAY_EXPR;

  ArrayLiteral *arr = new_array_literal(p);
  expr->value = arr;

  return expr;
}

HashLiteral *new_hash_literal(Parser *p) {
  HashLiteral *hash = malloc(sizeof(HashLiteral));
  assert(hash != NULL && "Error allocating memory for hash");

  hash->len = 0;
  hash->token = p->cur_token;
  hashmap_create(5, &hash->pairs);

  while (!peek_token_is(p, RBRACE)) {
    parser_next_token(p);
    Expression *key = parse_expression(p, LOWEST);
    if (!expect_peek(p, COLON)) {
      hashmap_destroy(&hash->pairs);
      free(hash);
      return NULL;
    }

    parser_next_token(p);
    Expression *value = parse_expression(p, LOWEST);

    hashmap_put(&hash->pairs, key, sizeof(Expression), value);
    hash->len += 1;

    if (!peek_token_is(p, RBRACE) && !expect_peek(p, COMMA)) {
      hashmap_destroy(&hash->pairs);
      free(hash);
      return NULL;
    }
  }

  if (!expect_peek(p, RBRACE)) {
    hashmap_destroy(&hash->pairs);
    free(hash);
    return NULL;
  }

  return hash;
}

Expression *parse_hash_literal(Parser *p) {
  Expression *expr = malloc(sizeof(Expression));
  assert(expr != NULL && "Error allocating memory for hash expression");

  expr->type = HASH_EXPR;
  HashLiteral *hash = new_hash_literal(p);

  expr->value = hash;
  return expr;
}

WhileLoop *new_while_loop(Parser *p) {
  WhileLoop *loop = malloc(sizeof(WhileLoop));
  assert(loop != NULL && "Error allocating memory for while loop");

  loop->token = p->cur_token;

  // TODO: improve error messages
  if (!expect_peek(p, LPAREN)) {
    free(loop);
    return NULL;
  }

  parser_next_token(p);
  loop->condition = parse_expression(p, LOWEST);

  if (!expect_peek(p, RPAREN)) {
    free(loop->condition);
    free(loop);

    return NULL;
  }

  parser_next_token(p);

  loop->body = parse_block_statement(p);

  return loop;
}

Expression *parse_while_loop(Parser *p) {
  bool should_alter_loop_state = !INSIDE_LOOP;
  Expression *expr = malloc(sizeof(Expression));
  assert(expr != NULL && "Error allocating memory for while expression");
  expr->type = WHILE_EXPR;

  if (should_alter_loop_state) {
    INSIDE_LOOP = true;
  }
  WhileLoop *loop = new_while_loop(p);

  if (should_alter_loop_state) {
    INSIDE_LOOP = false;
  }

  expr->value = loop;

  return expr;
}

Statement *parse_if_exists(Parser *p, bool prev_exists) {
  if (peek_token_is(p, SEMICOLON)) {
    parser_next_token(p);
    return NULL;
  }

  if (cur_token_is(p, SEMICOLON) && peek_token_is(p, RPAREN)) {
    return NULL;
  }

  parser_next_token(p);
  return parse_statement(p);
}

ForLoop *new_for_loop(Parser *p) {
  ForLoop *loop = malloc(sizeof(ForLoop));
  assert(loop != NULL);

  if (!expect_peek(p, LPAREN)) {
    free(loop);
    return NULL;
  }

  loop->initialization = parse_if_exists(p, false);

  Statement *condition = parse_if_exists(p, loop->initialization != NULL);
  if (condition != NULL) {
    loop->condition = condition->expression;
    free(condition);
  } else {
    loop->condition = NULL;
  }

  if (peek_token_is(p, RPAREN)) {
    parser_next_token(p);
    loop->update = NULL;
  } else {
    parser_next_token(p);
    loop->update = parse_statement(p);
  }


  if (!expect_peek(p, LBRACE)) {
    free(loop->update);
    free(loop->condition);
    free(loop->initialization);
    free(loop);
    return NULL;
  }

  loop->body = parse_block_statement(p);

  return loop;
}

Expression *parse_for_loop(Parser *p) {
  bool should_alter_loop_state = !INSIDE_LOOP;
  Expression *expr = malloc(sizeof(Expression));
  assert(expr != NULL && "Error allocating memory for for expression");
  expr->type = FOR_EXPR;

  if (should_alter_loop_state) {
    INSIDE_LOOP = true;
  }

  ForLoop *loop = new_for_loop(p);

  if (should_alter_loop_state) {
    INSIDE_LOOP = false;
  }

  expr->value = loop;

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

LinkedList *parse_call_arguments(Parser *p) {
  LinkedList *arguments = new_list();

  if (peek_token_is(p, RPAREN)) {
    parser_next_token(p);
    return arguments;
  }

  parser_next_token(p);
  append(arguments, parse_expression(p, LOWEST));

  while (peek_token_is(p, COMMA)) {
    parser_next_token(p);
    parser_next_token(p);
    append(arguments, parse_expression(p, LOWEST));
  }

  if (!expect_peek(p, RPAREN)) {
    free_list(arguments); // TODO: Need to free each expression
                          // based on its type
    return NULL;
  }

  return arguments;
}

Expression *parse_call_expression(Parser *p, Expression *function) {
  Expression *expr = malloc(sizeof(Expression));
  assert(expr != NULL && "ERROR: could not allocate memory for expression");

  expr->type = CALL_EXPR;

  CallExpression *call = malloc(sizeof(CallExpression));
  assert(call != NULL &&
         "ERROR: could not allocate memory for call expression");

  call->token = p->cur_token;
  call->function = function;
  LinkedList *args = parse_call_arguments(p);

  if (args == NULL) {
    free(call);
    free(expr);
    return NULL;
  }

  call->arguments = args;

  expr->value = call;
  return expr;
}

Expression *parse_index_expression(Parser *p, Expression *left) {
  Expression *exp = malloc(sizeof(Expression));
  assert(exp != NULL && "Error allocating memory for index expression");
  exp->type = INDEX_EXPR;

  IndexExpression *index_expr = malloc(sizeof(IndexExpression));
  assert(index_expr != NULL && "Error allocating memory for index expression");
  index_expr->token = p->cur_token;
  index_expr->left = left;

  parser_next_token(p);

  index_expr->index = parse_expression(p, LOWEST);

  if (!expect_peek(p, RBRACKET)) {
    free(index_expr);
    free(exp);
    return NULL;
  }

  exp->value = index_expr;

  return exp;
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
  register_prefix_fn(p, &parse_function_literal, FUNCTION);
  register_prefix_fn(p, &parse_string_literal, STRING);
  register_prefix_fn(p, &parse_array_literal, LBRACKET);
  register_prefix_fn(p, &parse_hash_literal, LBRACE);
  register_prefix_fn(p, &parse_while_loop, WHILE);
  register_prefix_fn(p, &parse_for_loop, FOR);

  register_infix_fn(p, &parse_infix_expression, PLUS);
  register_infix_fn(p, &parse_infix_expression, MINUS);
  register_infix_fn(p, &parse_infix_expression, SLASH);
  register_infix_fn(p, &parse_infix_expression, ASTERISK);
  register_infix_fn(p, &parse_infix_expression, EQ);
  register_infix_fn(p, &parse_infix_expression, NOT_EQ);
  register_infix_fn(p, &parse_infix_expression, LT);
  register_infix_fn(p, &parse_infix_expression, GT);
  register_infix_fn(p, &parse_call_expression, LPAREN);
  register_infix_fn(p, &parse_index_expression, LBRACKET);

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
