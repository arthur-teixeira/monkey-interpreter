#include "../lexer/lexer.h"
#include <stddef.h>
#include "../linked_list/linked_list.h"

typedef struct {
  Token token;
  char *value;
} Identifier;

Identifier *new_identifier(Token token, char *value);

typedef struct {
  Token token;
  void *value; // TODO: implement as Expression struct?
  Identifier *name;
} Statement;

typedef struct {
  LinkedList *statements;
} Program;

char *program_token_literal(Program *);

Program *new_program(void);
