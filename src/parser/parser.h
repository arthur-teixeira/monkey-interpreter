#include "../ast/ast.h"

typedef struct {
  Lexer *l;
  Token cur_token;
  Token peek_token;
  LinkedList *errors; // TODO: implement as string buffer
  Expression *(*prefixParseFn[TOKEN_COUNTS])(void);
  Expression *(*infixParseFns[TOKEN_COUNTS])(Expression *);
} Parser;

Parser *new_parser(Lexer *);

void parser_next_token(Parser *);

void free_parser(Parser *);

Program *parse_program(Parser *);
