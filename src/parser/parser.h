#include "../ast/ast.h"

typedef struct Parser Parser;
typedef Expression *(*prefix_parse_fn)(struct Parser*);
typedef Expression *(*infix_parse_fn)(struct Parser*, Expression *);
struct Parser {
  Lexer *l;
  Token cur_token;
  Token peek_token;
  LinkedList *errors; // TODO: implement as string buffer
  prefix_parse_fn prefix_parse_fns[TOKEN_COUNT];
  infix_parse_fn infix_parse_fns[TOKEN_COUNT];
};

Parser *new_parser(Lexer *);

void parser_next_token(Parser *);

void free_parser(Parser *);

Program *parse_program(Parser *);

typedef enum {
  _,
  LOWEST,
  EQUALS,
  LESSGREATER,
  SUM,
  PRODUCT,
  PREFIX,
  CALL
} OperatorPrecedenceOrder;

Expression *parse_expression(Parser *p, OperatorPrecedenceOrder precedence);
