#include "../token/token.h"
#include <stdint.h>

typedef struct {
    char *input;
    uint32_t position;
    uint32_t read_position;
    char ch;
} Lexer;

Lexer* new_lexer(char *input);

Token next_token(Lexer*);
