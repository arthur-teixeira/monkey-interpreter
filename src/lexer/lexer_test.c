#include <stdint.h>
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "lexer.h"
#include <string.h>


void test_next_token(void) {
    char input = *"=+(){},;";

    typedef struct {
        TokenType expected_type;
        char *expected_literal;
    } TestCase;

     TestCase tests[] = {
         { PLUS, "+" },
         { LPAREN, "(" },
         { RPAREN, ")" },
         { LBRACE, "{" },
         { RBRACE, "}" },
         { COMMA, "," },
         { SEMICOLON, ";" },
         { END_OF_FILE, "\0" }
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
