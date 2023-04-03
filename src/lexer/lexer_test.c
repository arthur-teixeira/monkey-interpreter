#include <stdint.h>
#include <unity/unity.h>
#include <unity/unity_internals.h>
#include "lexer.h"
#include "../token/token.h"

void setUp(void) {
}

void test_next_token(void) {
    char input = *"=+(){},;";

    struct {
      TokenType expected_type;
      char expected_literal[MAX_LEN];
    } tests[] = {
        { ASSIGN, '=' },
        { PLUS, '+' },
        { LPAREN, '(' },
        { RPAREN, ')' },
        { LBRACE, '{' },
        { RBRACE, '}' },
        { COMMA, ',' },
        { SEMICOLON, ';' },
        { END_OF_FILE, '\0' },
    };

    Lexer *lexer = new_lexer(&input);

    for (uint32_t i = 0; i < 9; i++) {
        Token tok = next_token(lexer);

        TEST_ASSERT_EQUAL_STRING(tok.Type, tests[i].expected_type);
        TEST_ASSERT_EQUAL_STRING(tok.literal, tests[i].expected_literal);
    }
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_next_token);
    UNITY_END();
}
