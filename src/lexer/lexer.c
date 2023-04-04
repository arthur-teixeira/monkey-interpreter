#include "lexer.h"
#include <stdlib.h>

Lexer *new_lexer(char *input) {
    Lexer *lexer = malloc(sizeof(Lexer));

    return lexer;
}

Token next_token(Lexer *lexer) {
    Token tok = {
        ASSIGN,
        "=",
    };

    return tok;
}
