#include "../dyn_array/dyn_array.h"
#include "../hashmap/hashmap.h"
#include "../lexer/lexer.h"
#include "../linked_list/linked_list.h"
#include "../str_utils/str_utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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
  FOR_EXPR,
  REASSIGN_EXPR,
} ExprType;

// Acts as an "interface". Entry level functions will receive a pointer
// to an Expression struct. Depending on the type, the struct will be cast
// to its correct type. Every type "implements" this interface by having a
// property called "type" as the first member of the struct.
typedef struct {
  ExprType type;
} Expression;

void value_to_string(ResizableBuffer *, Expression *);

typedef struct {
  ExprType type; // IDENT_EXPR
  Token token;
  char *value;
} Identifier;

Identifier *new_identifier(Token, char *);
void free_identifier(Identifier *);

typedef struct {
  ExprType type; // INT_EXPR
  Token token;
  double value;
} NumberLiteral;

void int_to_string(ResizableBuffer *, NumberLiteral *);

typedef struct {
  ExprType type; // PREFIX_EXPR
  Token token;
  char *operator;
  Expression *right;
} PrefixExpression;

void prefix_to_string(ResizableBuffer *, PrefixExpression *);

typedef struct {
  ExprType type; // INFIX_EXPR
  Token token;
  Expression *left;
  char *operator;
  Expression *right;
} InfixExpression;

void infix_to_string(ResizableBuffer *, InfixExpression *);

typedef struct {
  ExprType type; // BOOL_EXPR
  Token token;
  bool value;
} BooleanLiteral;

void bool_to_string(ResizableBuffer *, BooleanLiteral *);

typedef struct {
  Token token;
  DynamicArray statements;
} BlockStatement;

void block_to_string(ResizableBuffer *, BlockStatement *);

typedef struct {
  ExprType type; // IF_EXPR
  Token token;
  Expression *condition;
  BlockStatement *consequence;
  BlockStatement *alternative;
} IfExpression;

void if_to_string(ResizableBuffer *, IfExpression *);

typedef struct {
  ExprType type; // FN_EXPR
  Token token;
  DynamicArray parameters; // Identifier*[];
  BlockStatement *body;
} FunctionLiteral;

void fn_to_string(ResizableBuffer *, FunctionLiteral *);

typedef struct {
  ExprType type; // STRING_EXPR
  Token token;
  char *value;
  uint32_t len;
} StringLiteral;

void string_literal_to_string(ResizableBuffer *, StringLiteral *);

typedef struct {
  ExprType type; // CALL_EXPR
  Token token;
  Expression *function;
  DynamicArray arguments; // TODO: Expression*[];
} CallExpression;

void call_to_string(ResizableBuffer *, CallExpression *);

typedef struct {
  ExprType type; // ARRAY_EXPR
  Token token;
  DynamicArray *elements;
} ArrayLiteral;

void array_to_string(ResizableBuffer *, ArrayLiteral *);

typedef struct {
  ExprType type; // INDEX_EXPR
  Token token;
  Expression *left;
  Expression *index;
} IndexExpression;

void index_expression_to_string(ResizableBuffer *, IndexExpression *);

typedef struct {
  ExprType type; // HASH_EXPR
  Token token;
  hashmap_t pairs;
  size_t len;
} HashLiteral;

void hash_literal_to_string(ResizableBuffer *, HashLiteral *);

typedef struct {
  ExprType type; // REASSIGN_EXPR
  Token token;
  Identifier *name;
  Expression *value;
} Reassignment;

void reassign_to_string(ResizableBuffer *, Reassignment *);

typedef struct {
  ExprType type; // WHILE_EXPR
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
  ExprType type; // FOR_EXPR
  Token token;
  Statement *initialization;
  Expression *condition;
  Statement *update;
  BlockStatement *body;
} ForLoop;

void for_to_string(ResizableBuffer *, ForLoop *);

typedef struct {
  DynamicArray statements; // Statement*[];
} Program;

Program *new_program(void);
void free_program(Program *p);

void program_string(ResizableBuffer *, Program *);

void ident_expr_to_string(ResizableBuffer *buf, Identifier *expr);
void block_to_string(ResizableBuffer *buf, BlockStatement *block);
