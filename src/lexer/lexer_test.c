#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "lexer.h"
#include <stdint.h>
#include <string.h>

void test_next_token(void) {
  char *input = "=+(){},;";

  typedef struct {
    TokenType expected_type;
    char expected_literal[100];
  } TestCase;

  TestCase tests[] = {{ASSIGN, "="}, {PLUS, "+"},      {LPAREN, "("},
                      {RPAREN, ")"}, {LBRACE, "{"},    {RBRACE, "}"},
                      {COMMA, ","},  {SEMICOLON, ";"}, {END_OF_FILE, "\0"}};

  Lexer *lexer = new_lexer(input);

  for (uint32_t i = 0; i < 9; i++) {
    Token tok = next_token(lexer);

    TEST_ASSERT_EQUAL(tests[i].expected_type, tok.Type);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_literal, tok.literal);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_next_token);
  UNITY_END();
}
