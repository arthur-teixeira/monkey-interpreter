#include "../lexer/lexer.h"
#include <stddef.h>
#include "../linked_list/linked_list.h"

typedef struct {
  Token token;
  char *value;
} Identifier;

Identifier *new_identifier(Token token, char *value);

struct Statement {
  Token token;
  Identifier *name;
  void *value;
};
typedef struct Statement Statement;

char *literal(Statement *);

typedef struct {
  LinkedList *statements;
} Program;

char *program_token_literal(Program *);

Program *new_program(void);
