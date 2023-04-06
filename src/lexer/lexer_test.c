#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "lexer.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

void test_next_token(void) {
  char *input =
                "let five = 5;"
                "let ten = 10;"
                "let add = fn(x, y) {"
                "  x + y; "
                "};"
                "let result = add(five, ten);";

  typedef struct {
    TokenType expected_type;
    char expected_literal[100];
  } TestCase;

  TestCase tests[] = {
      {LET, "let"},        {IDENT, "five"},  {ASSIGN, "="},  {INT, "5"},
      {SEMICOLON, ";"},    {LET, "let"},     {IDENT, "ten"}, {ASSIGN, "="},
      {INT, "10"},         {SEMICOLON, ";"}, {LET, "let"},   {IDENT, "add"},
      {ASSIGN, "="},       {FUNCTION, "fn"}, {LPAREN, "("},  {IDENT, "x"},
      {COMMA, ","},        {IDENT, "y"},     {RPAREN, ")"},  {LBRACE, "{"},
      {IDENT, "x"},        {PLUS, "+"},      {IDENT, "y"},   {SEMICOLON, ";"},
      {RBRACE, "}"},       {SEMICOLON, ";"}, {LET, "let"},   {IDENT, "result"},
      {ASSIGN, "="},       {IDENT, "add"},   {LPAREN, "("},  {IDENT, "five"},
      {COMMA, ","},        {IDENT, "ten"},   {RPAREN, ")"},  {SEMICOLON, ";"},
      {END_OF_FILE, "\0"},
  };

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
