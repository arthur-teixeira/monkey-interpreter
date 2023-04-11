#include "../lexer/lexer.h"
#include <stddef.h>

typedef struct {
  Token token;
  char *value;
} Identifier;

struct Statement {
  Token token;
  Identifier *name;
  void *value;
};
typedef struct Statement Statement;

char *literal(Statement *);

typedef struct {
  size_t size;
  Statement *statements[];
} Program;

char *program_token_literal(Program *);
