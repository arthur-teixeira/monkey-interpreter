#include "parser.h"
#include "../dyn_array/dyn_array.h"
#include "../str_utils/str_utils.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define ARRAY_LEN(arr, type) sizeof(arr) / sizeof(type)

void print_parser_errors(Parser *p) {
  Node *cur_error = p->errors->tail;
  assert(cur_error != NULL);

  while (cur_error != NULL) {
    printf("%s\n", (char *)cur_error->value);
    cur_error = cur_error->next;
  }
}

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
  p->precedences[ASSIGN] = REASSIGN;
  p->precedences[BAND] = BIT_AND;
  p->precedences[BOR] = BIT_OR;
  p->precedences[BXOR] = BIT_XOR;
  p->precedences[LSHIFT] = BITSHIFT;
  p->precedences[RSHIFT] = BITSHIFT;
  p->precedences[MOD] = PRODUCT;
  p->precedences[AND] = PREC_AND;
  p->precedences[OR] = PREC_OR;
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

  stmt->name = new_identifier(p->cur_token, p->cur_token.literal);

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
  return (Expression *)new_identifier(p->cur_token, p->cur_token.literal);
}

Expression *parse_number_literal(Parser *p) {
  NumberLiteral *lit = malloc(sizeof(NumberLiteral));
  assert(lit != NULL);

  lit->type = INT_EXPR;
  lit->value = strtof(p->cur_token.literal, NULL);
  lit->token = p->cur_token;

  return (Expression *)lit;
}

Expression *parse_binary_literal(Parser *p) {
  NumberLiteral *lit = malloc(sizeof(NumberLiteral));
  assert(lit != NULL);

  int result = 0;
  int multiplier = 0;

  for (int i = strlen(p->cur_token.literal) - 1; i >= 0; i--) {
    int cur_val = p->cur_token.literal[i] == '1' ? 1 : 0;
    result += (cur_val * (1 << multiplier++));
  }

  lit->type = INT_EXPR;
  lit->value = result;
  lit->token = p->cur_token;

  return (Expression *)lit;
}

int htoi(char *s) {
  int i = 0;
  int res = 0;

  if (s[0] == '0' && (s[1] == 'X' || s[1] == 'x'))
    i = 2;

  for (; i < strlen(s); ++i) {
    res = res * 16;

    if (isdigit(s[i]))
      res = res + s[i] - '0';
    else if (s[i] >= 'a' && s[i] <= 'f')
      res = res + s[i] - 'a' + 10;
    else if (s[i] >= 'A' && s[i] <= 'F')
      res = res + s[i] - 'A' + 10;
  }
  return res;
}

Expression *parse_hex_literal(Parser *p) {
  NumberLiteral *intt = malloc(sizeof(NumberLiteral));
  assert(intt != NULL);

  intt->type = INT_EXPR;
  intt->token = p->cur_token;
  intt->value = htoi(p->cur_token.literal);

  return (Expression *)intt;
}

Expression *parse_prefix_expression(Parser *p) {
  PrefixExpression *prefix = malloc(sizeof(PrefixExpression));
  assert(prefix != NULL);

  prefix->type = PREFIX_EXPR;
  prefix->operator= strdup(p->cur_token.literal);
  prefix->token = p->cur_token;

  parser_next_token(p);
  prefix->right = parse_expression(p, PREFIX);

  return (Expression *)prefix;
}

Expression *parse_boolean(Parser *p) {
  BooleanLiteral *expr = malloc(sizeof(BooleanLiteral));
  assert(expr != NULL);

  expr->type = BOOL_EXPR;
  expr->token = p->cur_token;
  expr->value = cur_token_is(p, TRUE);

  return (Expression *)expr;
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
  assert(block != NULL);

  block->token = p->cur_token;
  array_init(&block->statements, 10);

  parser_next_token(p);

  while (!cur_token_is(p, RBRACE) && !cur_token_is(p, END_OF_FILE)) {
    Statement *stmt = parse_statement(p);
    if (stmt != NULL) {
      array_append(&block->statements, stmt);
    }
    parser_next_token(p);
  }

  return block;
}

Expression *parse_if_expression(Parser *p){
  if (!expect_peek(p, LPAREN)) {
    return NULL;
  }

  IfExpression *expr = malloc(sizeof(IfExpression));
  assert(expr != NULL);
  expr->type = IF_EXPR;

  parser_next_token(p);
  expr->condition = parse_expression(p, LOWEST);

  if (!expect_peek(p, RPAREN)) {
    free(expr->condition);
    free(expr);
    return NULL;
  }

  if (!expect_peek(p, LBRACE)) {
    free(expr->condition);
    free(expr);
    return NULL;
  }

  expr->consequence = parse_block_statement(p);

  if (!peek_token_is(p, ELSE)) {
    return (Expression *)expr;
  }

  parser_next_token(p);
  if (!expect_peek(p, LBRACE)) {
    free(expr->condition);
    array_free(&expr->consequence->statements);
    free(expr);

    return NULL;
  }

  expr->alternative = parse_block_statement(p);

  return (Expression *)expr;
} 

DynamicArray parse_function_parameters(Parser *p) {
  DynamicArray parameters;
  array_init(&parameters, 10);

  if (peek_token_is(p, RPAREN)) {
    parser_next_token(p);
    return parameters;
  }

  parser_next_token(p);

  Identifier *ident = new_identifier(p->cur_token, p->cur_token.literal);
  array_append(&parameters, ident);

  while (peek_token_is(p, COMMA)) {
    parser_next_token(p);
    parser_next_token(p);
    Identifier *ident = new_identifier(p->cur_token, p->cur_token.literal);
    array_append(&parameters, ident);
  }

  if (!expect_peek(p, RPAREN)) {
    array_free(&parameters);
    parameters.len = -1;
  }

  return parameters;
}

Expression *parse_function_literal(Parser *p) {
  if (!expect_peek(p, LPAREN)) {
    return NULL;
  }

  FunctionLiteral *fn = malloc(sizeof(FunctionLiteral));
  assert(fn != NULL); 

  fn->token = p->cur_token;
  fn->type = FN_EXPR;

  fn->parameters = parse_function_parameters(p);
  if (fn->parameters.len < 0) {
    free(fn);
    return NULL;
  }

  if (!expect_peek(p, LBRACE)) {
    array_free(&fn->parameters);
    free(fn);

    return NULL;
  }

  fn->body = parse_block_statement(p);
  return (Expression *)fn;
} 

Expression *parse_string_literal(Parser *p) {
  StringLiteral *str = malloc(sizeof(StringLiteral));
  assert(str != NULL && "error allocating memory for string literal");

  str->token = p->cur_token;
  str->len = strlen(p->cur_token.literal);
  str->value = strdup(p->cur_token.literal);
  str->type = STRING_EXPR;

  return (Expression *)str;
} 

Expression *parse_array_literal(Parser *p) {
  ArrayLiteral *arr = malloc(sizeof(ArrayLiteral));
  assert(arr != NULL);
  arr->token = p->cur_token;
  arr->elements = malloc(sizeof(DynamicArray));
  arr->type = ARRAY_EXPR;
  assert(arr->elements != NULL);
  array_init(arr->elements, 10);

  if (peek_token_is(p, RBRACKET)) {
    parser_next_token(p);
    return (Expression *)arr;
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

  return (Expression *)arr;
}

Expression *parse_hash_literal(Parser *p) {
  HashLiteral *hash = malloc(sizeof(HashLiteral));
  assert(hash != NULL && "Error allocating memory for hash");

  hash->len = 0;
  hash->token = p->cur_token;
  hash->type = HASH_EXPR;
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

  return (Expression *)hash;
}

Expression *parse_while_loop(Parser *p) {
  bool should_alter_loop_state = !INSIDE_LOOP;

  if (should_alter_loop_state) {
    INSIDE_LOOP = true;
  }
  
  WhileLoop *loop = malloc(sizeof(WhileLoop));
  assert(loop != NULL && "Error allocating memory for while loop");

  loop->type = WHILE_EXPR;
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

  if (should_alter_loop_state) {
    INSIDE_LOOP = false;
  }

  return (Expression *)loop;
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

Expression *parse_for_loop(Parser *p) {
  bool should_alter_loop_state = !INSIDE_LOOP;

  if (should_alter_loop_state) {
    INSIDE_LOOP = true;
  }

  ForLoop *loop = malloc(sizeof(ForLoop));
  assert(loop != NULL);
  loop->type = FOR_EXPR;

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

    if (peek_token_is(p, RPAREN)) {
      parser_next_token(p);
    }
  }

  if (!expect_peek(p, LBRACE)) {
    free(loop->update);
    free(loop->condition);
    free(loop->initialization);
    free(loop);
    return NULL;
  }

  loop->body = parse_block_statement(p);

  if (should_alter_loop_state) {
    INSIDE_LOOP = false;
  }

  return (Expression *)loop;
}

Expression *parse_infix_expression(Parser *p, Expression *left) {
  InfixExpression *infix_expr = malloc(sizeof(InfixExpression));
  assert(infix_expr != NULL); 

  infix_expr->type = INFIX_EXPR;
  infix_expr->operator = strdup(p->cur_token.literal);
  infix_expr->token = p->cur_token;

  uint32_t precedence = cur_precedence(p);
  parser_next_token(p);

  infix_expr->left = left;
  infix_expr->right = parse_expression(p, precedence);

  return (Expression *)infix_expr;
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
  CallExpression *call = malloc(sizeof(CallExpression));
  call->type = CALL_EXPR;
  assert(call != NULL &&
         "ERROR: could not allocate memory for call expression");

  call->token = p->cur_token;
  call->function = function;
  LinkedList *args = parse_call_arguments(p);

  if (args == NULL) {
    free(call);
    return NULL;
  }

  call->arguments = args;

  return (Expression *)call;
}

Expression *parse_index_expression(Parser *p, Expression *left) {
  IndexExpression *index_expr = malloc(sizeof(IndexExpression));
  assert(index_expr != NULL && "Error allocating memory for index expression");
  index_expr->type = INDEX_EXPR;
  index_expr->token = p->cur_token;
  index_expr->left = left;

  parser_next_token(p);

  index_expr->index = parse_expression(p, LOWEST);

  if (!expect_peek(p, RBRACKET)) {
    free(index_expr);
    return NULL;
  }

  return (Expression *)index_expr;
}

Expression *parse_reassignment_expression(Parser *p, Expression *left) {
  assert(left->type == IDENT_EXPR);

  Reassignment *reassignment = malloc(sizeof(Reassignment));
  assert(reassignment != NULL);

  reassignment->type = REASSIGN_EXPR;
  reassignment->name = (Identifier *)left;
  reassignment->token = p->cur_token;

  parser_next_token(p);

  reassignment->value = parse_expression(p, LOWEST);

  return (Expression *)reassignment;
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

  register_prefix_fn(p, parse_identifier, IDENT);
  register_prefix_fn(p, parse_number_literal, NUMBER);
  register_prefix_fn(p, parse_prefix_expression, BANG);
  register_prefix_fn(p, parse_prefix_expression, MINUS);
  register_prefix_fn(p, parse_boolean, TRUE);
  register_prefix_fn(p, parse_boolean, FALSE);
  register_prefix_fn(p, parse_grouped_expression, LPAREN);
  register_prefix_fn(p, parse_if_expression, IF);
  register_prefix_fn(p, parse_function_literal, FUNCTION);
  register_prefix_fn(p, parse_string_literal, STRING);
  register_prefix_fn(p, parse_array_literal, LBRACKET);
  register_prefix_fn(p, parse_hash_literal, LBRACE);
  register_prefix_fn(p, parse_while_loop, WHILE);
  register_prefix_fn(p, parse_for_loop, FOR);
  register_prefix_fn(p, parse_binary_literal, BINARY);
  register_prefix_fn(p, parse_hex_literal, HEX);

  TokenType infix_expressions[] = {
      PLUS,   MINUS,  SLASH, ASTERISK, EQ,   NOT_EQ, LT,  GT,
      LSHIFT, RSHIFT, MOD,   BOR,      BAND, BXOR,   AND, OR,
  };

  for (int i = 0; i < ARRAY_LEN(infix_expressions, TokenType); ++i) {
    register_infix_fn(p, parse_infix_expression, infix_expressions[i]);
  }

  register_infix_fn(p, parse_call_expression, LPAREN);
  register_infix_fn(p, parse_index_expression, LBRACKET);
  register_infix_fn(p, parse_reassignment_expression, ASSIGN);

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
