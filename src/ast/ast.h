#include "../lexer/lexer.h"
#include <stddef.h>
#include <stdio.h>
#include "../linked_list/linked_list.h"

typedef struct {
  Token token;
  char *value;
} Identifier;

Identifier *new_identifier(Token token, char *value);

typedef enum {
  LET_STATEMENT,
  RETURN_STATEMENT,
  EXPR_STATEMENT
} StatementType;

typedef enum {
  IDENT_EXPR,
} ExprType;

typedef struct {
  ExprType type;
  void *value;
} Expression;
char *value_to_string(Expression *);

typedef struct {
  StatementType type;
  Token token;
  Identifier *name;
  Expression *value; 
} Statement;

char *stmt_to_string(Statement *);

typedef struct {
  LinkedList *statements;
} Program;

char *program_token_literal(Program *);

Program *new_program(void);

char *program_string(Program *);
