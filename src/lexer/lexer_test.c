#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "lexer.h"
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void test_next_token(void) {
  char *input = "let five = 5;"
                "let ten = 10;"
                "let add = fn(x, y) {"
                "  x + y; "
                "};"
                "let result = add(five, ten);"
                "!-/*5;"
                "5 < 10 > 5;";

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
      {BANG, "!"},         {MINUS, "-"},     {SLASH, "/"},   {ASTERISK, "*"},
      {INT, "5"},          {SEMICOLON, ";"}, {INT, "5"},     {LT, "<"},
      {INT, "10"},         {GT, ">"},        {INT, "5"},     {SEMICOLON, ";"},
      {END_OF_FILE, "\0"},
  };

  Lexer *lexer = new_lexer(input);

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(TestCase); i++) {
    Token tok = next_token(lexer);

    //TEST_ASSERT_EQUAL(tests[i].expected_type, tok.Type);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_literal, tok.literal);
    char err_str[300];
    sprintf(err_str, "Happened at i = %d with the literal %s, should be %s", i, tok.literal, tests[i].expected_literal); 
    UNITY_TEST_ASSERT_EQUAL_INT(tests[i].expected_type, tok.Type, i, err_str);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_next_token);
  UNITY_END();
}
