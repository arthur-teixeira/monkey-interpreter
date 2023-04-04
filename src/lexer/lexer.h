#include "../token/token.h"

typedef struct {
} Lexer;

Token next_token(Lexer*);

Lexer* new_lexer(char *input);
