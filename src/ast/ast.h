#include "../lexer/lexer.h"
#include "../linked_list/linked_list.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
  Token token;
  char *value;
} Identifier;

Identifier *new_identifier(Token, char *);
void free_identifier(Identifier *);

typedef struct {
  Token token;
  int64_t value;
} IntegerLiteral;

void int_to_string(char *, IntegerLiteral *);

typedef enum { LET_STATEMENT, RETURN_STATEMENT, EXPR_STATEMENT } StatementType;

typedef enum {
  IDENT_EXPR,
  INT_EXPR,
  PREFIX_EXPR,
  INFIX_EXPR,
  BOOL_EXPR,
  IF_EXPR,
} ExprType;

typedef struct {
  ExprType type;
  void *value;
} Expression;
void value_to_string(char *, Expression *);

typedef struct {
  Token token;
  char *operator;
  Expression *right;
} PrefixExpression;

void prefix_to_string(char *, PrefixExpression *);

typedef struct {
  Token token;
  Expression *left;
  char *operator;
  Expression *right;
} InfixExpression;

void infix_to_string(char *, InfixExpression *);

typedef struct {
  Token token;
  bool value;
} Boolean;

void bool_to_string(char *, Boolean *);

typedef struct {
  Token token;
  LinkedList *statements;
} BlockStatement;

void block_to_string(char *, BlockStatement *);

typedef struct {
  Token token;
  Expression *condition;
  BlockStatement *consequence;
  BlockStatement *alternative;
} IfExpression;

void if_to_string(char *, IfExpression *);

typedef struct {
  StatementType type;
  Token token;
  Identifier *name;
  Expression *expression;
} Statement;

void stmt_to_string(char *, Statement *);

typedef struct {
  LinkedList *statements;
} Program;

void program_token_literal(char *, Program *);

Program *new_program(void);

void free_program(Program *p);

void program_string(char *, Program *);
