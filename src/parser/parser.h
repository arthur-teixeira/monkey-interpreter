#ifndef PARSER_H
#define PARSER_H
#include "../ast/ast.h"

typedef struct Parser Parser;
typedef Expression *(*prefix_parse_fn)(struct Parser*);
typedef Expression *(*infix_parse_fn)(struct Parser*, Expression *);
struct Parser {
  Lexer *l;
  Token cur_token;
  Token peek_token;
  DynamicArray errors;
  prefix_parse_fn prefix_parse_fns[TOKEN_COUNT];
  infix_parse_fn infix_parse_fns[TOKEN_COUNT];
  uint32_t precedences[TOKEN_COUNT];
};

Parser *new_parser(Lexer *);

void parser_next_token(Parser *);

void free_parser(Parser *);

Program *parse_program(Parser *);

void print_parser_errors(Parser *p);

typedef enum {
  _,
  LOWEST,
  PREC_OR,
  PREC_AND,
  EQUALS,
  BIT_OR,
  BIT_XOR,
  BIT_AND,
  LESSGREATER,
  BITSHIFT,
  SUM,
  PRODUCT,
  PREFIX,
  CALL,
  INDEX,
  REASSIGN,
} OperatorPrecedenceOrder;

Expression *parse_expression(Parser *p, OperatorPrecedenceOrder precedence);
#endif //PARSER_H
