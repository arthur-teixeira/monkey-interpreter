#include "../dyn_array/dyn_array.h"
#include "../hashmap/hashmap.h"
#include "../lexer/lexer.h"
#include "../linked_list/linked_list.h"
#include "../str_utils/str_utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

void int_to_string(ResizableBuffer *, IntegerLiteral *);

typedef enum {
  LET_STATEMENT,
  RETURN_STATEMENT,
  EXPR_STATEMENT,
  BREAK_STATEMENT,
  CONTINUE_STATEMENT
} StatementType;

typedef enum {
  IDENT_EXPR,
  INT_EXPR,
  PREFIX_EXPR,
  INFIX_EXPR,
  BOOL_EXPR,
  IF_EXPR,
  FN_EXPR,
  CALL_EXPR,
  STRING_EXPR,
  ARRAY_EXPR,
  INDEX_EXPR,
  HASH_EXPR,
  WHILE_EXPR,
} ExprType;

typedef struct {
  ExprType type;
  void *value;
} Expression;
void value_to_string(ResizableBuffer *, Expression *);

typedef struct {
  Token token;
  char *operator;
  Expression *right;
} PrefixExpression;

void prefix_to_string(ResizableBuffer *, PrefixExpression *);

typedef struct {
  Token token;
  Expression *left;
  char *operator;
  Expression *right;
} InfixExpression;

void infix_to_string(ResizableBuffer *, InfixExpression *);

typedef struct {
  Token token;
  bool value;
} BooleanLiteral;

void bool_to_string(ResizableBuffer *, BooleanLiteral *);

typedef struct {
  Token token;
  LinkedList *statements;
} BlockStatement;

void block_to_string(ResizableBuffer *, BlockStatement *);

typedef struct {
  Token token;
  Expression *condition;
  BlockStatement *consequence;
  BlockStatement *alternative;
} IfExpression;

void if_to_string(ResizableBuffer *, IfExpression *);

typedef struct {
  Token token;
  LinkedList *parameters; // Identifier*[];
  BlockStatement *body;
} FunctionLiteral;

void fn_to_string(ResizableBuffer *, FunctionLiteral *);

typedef struct {
  Token token;
  char *value;
  uint32_t len;
} StringLiteral;

void string_literal_to_string(ResizableBuffer *, StringLiteral *);

typedef struct {
  Token token;
  Expression *function;
  LinkedList *arguments; // TODO: Expression*[];
} CallExpression;

void call_to_string(ResizableBuffer *, CallExpression *);

typedef struct {
  Token token;
  DynamicArray *elements;
} ArrayLiteral;

void array_to_string(ResizableBuffer *, ArrayLiteral *);

typedef struct {
  Token token;
  Expression *left;
  Expression *index;
} IndexExpression;

void index_expression_to_string(ResizableBuffer *, IndexExpression *);

typedef struct {
  Token token;
  hashmap_t pairs;
  size_t len;
} HashLiteral;

void hash_literal_to_string(ResizableBuffer *, HashLiteral *);

typedef struct {
  Token token;
  Expression *condition;
  BlockStatement *body;
} WhileLoop;

void while_to_string(ResizableBuffer *, WhileLoop *);

typedef struct {
  StatementType type;
  Token token;
  Identifier *name;
  Expression *expression;
} Statement;

void stmt_to_string(ResizableBuffer *, Statement *);

typedef struct {
  LinkedList *statements; // Statement*[];
} Program;

Program *new_program(void);

void free_program(Program *p);

void program_string(ResizableBuffer *, Program *);
void ident_expr_to_string(ResizableBuffer *buf, Identifier *expr);
void block_to_string(ResizableBuffer *buf, BlockStatement *block);
