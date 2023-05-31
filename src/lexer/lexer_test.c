#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "lexer.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void test_next_token(void) {
  char *input = "let five = 5;"
                "let ten = 10;"
                "let add = fn(x, y) {"
                "  x + y; "
                "};"
                "let result = add(five, ten);"
                "!-/*5;"
                "5 < 10 > 5;"
                "if (5 < 10) {"
                "  return true;"
                "} else {"
                "  return false;"
                "}"
                "10 == 10;"
                "10 != 9;"
                "\"foobar\""
                "\"foo bar\""
                "[1, 2]"
                "{\"foo\": \"bar\"}"
                "while (true) {"
                "break;"
                "continue;"
                "}"
                "for";

  typedef struct {
    TokenType expected_type;
    char expected_literal[100];
  } TestCase;

  TestCase tests[] = {
      {LET, "let"},      {IDENT, "five"},     {ASSIGN, "="},
      {INT, "5"},        {SEMICOLON, ";"},    {LET, "let"},
      {IDENT, "ten"},    {ASSIGN, "="},       {INT, "10"},
      {SEMICOLON, ";"},  {LET, "let"},        {IDENT, "add"},
      {ASSIGN, "="},     {FUNCTION, "fn"},    {LPAREN, "("},
      {IDENT, "x"},      {COMMA, ","},        {IDENT, "y"},
      {RPAREN, ")"},     {LBRACE, "{"},       {IDENT, "x"},
      {PLUS, "+"},       {IDENT, "y"},        {SEMICOLON, ";"},
      {RBRACE, "}"},     {SEMICOLON, ";"},    {LET, "let"},
      {IDENT, "result"}, {ASSIGN, "="},       {IDENT, "add"},
      {LPAREN, "("},     {IDENT, "five"},     {COMMA, ","},
      {IDENT, "ten"},    {RPAREN, ")"},       {SEMICOLON, ";"},
      {BANG, "!"},       {MINUS, "-"},        {SLASH, "/"},
      {ASTERISK, "*"},   {INT, "5"},          {SEMICOLON, ";"},
      {INT, "5"},        {LT, "<"},           {INT, "10"},
      {GT, ">"},         {INT, "5"},          {SEMICOLON, ";"},
      {IF, "if"},        {LPAREN, "("},       {INT, "5"},
      {LT, "<"},         {INT, "10"},         {RPAREN, ")"},
      {LBRACE, "{"},     {RETURN, "return"},  {TRUE, "true"},
      {SEMICOLON, ";"},  {RBRACE, "}"},       {ELSE, "else"},
      {LBRACE, "{"},     {RETURN, "return"},  {FALSE, "false"},
      {SEMICOLON, ";"},  {RBRACE, "}"},       {INT, "10"},
      {EQ, "=="},        {INT, "10"},         {SEMICOLON, ";"},
      {INT, "10"},       {NOT_EQ, "!="},      {INT, "9"},
      {SEMICOLON, ";"},  {STRING, "foobar"},  {STRING, "foo bar"},
      {LBRACKET, "["},   {INT, "1"},          {COMMA, ","},
      {INT, "2"},        {RBRACKET, "]"},     {LBRACE, "{"},
      {STRING, "foo"},   {COLON, ":"},        {STRING, "bar"},
      {RBRACE, "}"},     {WHILE, "while"},    {LPAREN, "("},
      {TRUE, "true"},    {RPAREN, ")"},       {LBRACE, "{"},
      {BREAK, "break"},  {SEMICOLON, ";"},    {CONTINUE, "continue"},
      {SEMICOLON, ";"},  {RBRACE, "}"},       {FOR, "for"},
      {END_OF_FILE, "\0"},
  };

  Lexer *lexer = new_lexer(input);

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(TestCase); i++) {
    Token tok = next_token(lexer);

    TEST_ASSERT_EQUAL(tests[i].expected_type, tok.Type);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_literal, tok.literal);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_next_token);
  return UNITY_END();
}
