#include "parser.h"
#include <stdlib.h>

void parser_next_token(Parser *p) {
    p->cur_token = p->peek_token;
    p->peek_token = next_token(p->l);
}

Parser *new_parser(Lexer *l) {
    Parser *p = malloc(sizeof(Parser));
    p->l = l;
    parser_next_token(p);
    parser_next_token(p);

    return p;
}

void free_parser(Parser *p) {
    free_lexer(p->l);
    free(p);
}

Program *parse_program(Parser *p) {
    return NULL;
}
