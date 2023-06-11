#ifndef LEXER_H
#define LEXER_H
#include "../token/token.h"
#include <stdint.h>
#include <stdio.h>

typedef struct {
    FILE *file;
    char *input;
    uint32_t position;
    uint32_t read_position;
    char ch;
} Lexer;

Lexer* new_lexer(char *input);

Lexer *new_file_lexer(const char *filename);

void free_lexer(Lexer *l);

Token next_token(Lexer*);
#endif //LEXER_H
