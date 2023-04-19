#include "../lexer/lexer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "../linked_list/linked_list.h"

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

typedef enum {
  LET_STATEMENT,
  RETURN_STATEMENT,
  EXPR_STATEMENT
} StatementType;

typedef enum {
  IDENT_EXPR,
  INT_EXPR
} ExprType;

typedef struct {
  ExprType type;
  void *value;
} Expression;
void value_to_string(char *, Expression *);

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

void program_string(char *, Program *);
