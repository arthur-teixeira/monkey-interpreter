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
                "for;"
                "0b1010;"
                "0xCAFE12;"
                "1 << 2;"
                "1 >> 2;"
                "1 ^ 2;"
                "1 & 2;"
                "1 | 2;"
                "1 % 2;"
                "true && false;"
                "true || false;";

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
      {SEMICOLON, ";"},  {BINARY, "1010"},    {SEMICOLON, ";"},
      {HEX, "CAFE12"},   {SEMICOLON, ";"},    {INT, "1"},
      {LSHIFT, "<<"},    {INT, "2"},          {SEMICOLON, ";"},
      {INT, "1"},        {RSHIFT, ">>"},      {INT, "2"},
      {SEMICOLON, ";"},  {INT, "1"},          {BXOR, "^"},
      {INT, "2"},        {SEMICOLON, ";"},    {INT, "1"},
      {BAND, "&"},       {INT, "2"},          {SEMICOLON, ";"},
      {INT, "1"},        {BOR, "|"},          {INT, "2"},
      {SEMICOLON, ";"},  {INT, "1"},          {MOD, "%"},
      {INT, "2"},        {SEMICOLON, ";"},    {TRUE, "true"},
      {AND, "&&"},       {FALSE, "false"},    {SEMICOLON, ";"},
      {TRUE, "true"},    {OR, "||"},          {FALSE, "false"},
      {SEMICOLON, ";"},  {END_OF_FILE, "\0"},
  };

  Lexer *lexer = new_lexer(input);

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(TestCase); i++) {
    Token tok = next_token(lexer);

    const char *expected_type = TOKEN_STRING[tests[i].expected_type];
    const char *actual_type = TOKEN_STRING[tok.Type];
    TEST_ASSERT_EQUAL_STRING(expected_type, actual_type);
    TEST_ASSERT_EQUAL(tests[i].expected_type, tok.Type);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_literal, tok.literal);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_next_token);
  return UNITY_END();
}
