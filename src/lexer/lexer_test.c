#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "lexer.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define LEN(arr) sizeof(arr) / sizeof(TestCase)

typedef struct {
  TokenType expected_type;
  char expected_literal[100];
} TestCase;

void test_tokens(Lexer *l, TestCase tests[], size_t len) {
  for (uint32_t i = 0; i < len; i++) {
    Token tok = next_token(l);

    const char *expected_type = TOKEN_STRING[tests[i].expected_type];
    const char *actual_type = TOKEN_STRING[tok.Type];
    TEST_ASSERT_EQUAL_STRING(expected_type, actual_type);
    TEST_ASSERT_EQUAL(tests[i].expected_type, tok.Type);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_literal, tok.literal);
  }
}

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
                "true || false;"
                "1.5";

  TestCase tests[] = {
      {LET, "let"},      {IDENT, "five"},    {ASSIGN, "="},
      {NUMBER, "5"},     {SEMICOLON, ";"},   {LET, "let"},
      {IDENT, "ten"},    {ASSIGN, "="},      {NUMBER, "10"},
      {SEMICOLON, ";"},  {LET, "let"},       {IDENT, "add"},
      {ASSIGN, "="},     {FUNCTION, "fn"},   {LPAREN, "("},
      {IDENT, "x"},      {COMMA, ","},       {IDENT, "y"},
      {RPAREN, ")"},     {LBRACE, "{"},      {IDENT, "x"},
      {PLUS, "+"},       {IDENT, "y"},       {SEMICOLON, ";"},
      {RBRACE, "}"},     {SEMICOLON, ";"},   {LET, "let"},
      {IDENT, "result"}, {ASSIGN, "="},      {IDENT, "add"},
      {LPAREN, "("},     {IDENT, "five"},    {COMMA, ","},
      {IDENT, "ten"},    {RPAREN, ")"},      {SEMICOLON, ";"},
      {BANG, "!"},       {MINUS, "-"},       {SLASH, "/"},
      {ASTERISK, "*"},   {NUMBER, "5"},      {SEMICOLON, ";"},
      {NUMBER, "5"},     {LT, "<"},          {NUMBER, "10"},
      {GT, ">"},         {NUMBER, "5"},      {SEMICOLON, ";"},
      {IF, "if"},        {LPAREN, "("},      {NUMBER, "5"},
      {LT, "<"},         {NUMBER, "10"},     {RPAREN, ")"},
      {LBRACE, "{"},     {RETURN, "return"}, {TRUE, "true"},
      {SEMICOLON, ";"},  {RBRACE, "}"},      {ELSE, "else"},
      {LBRACE, "{"},     {RETURN, "return"}, {FALSE, "false"},
      {SEMICOLON, ";"},  {RBRACE, "}"},      {NUMBER, "10"},
      {EQ, "=="},        {NUMBER, "10"},     {SEMICOLON, ";"},
      {NUMBER, "10"},    {NOT_EQ, "!="},     {NUMBER, "9"},
      {SEMICOLON, ";"},  {STRING, "foobar"}, {STRING, "foo bar"},
      {LBRACKET, "["},   {NUMBER, "1"},      {COMMA, ","},
      {NUMBER, "2"},     {RBRACKET, "]"},    {LBRACE, "{"},
      {STRING, "foo"},   {COLON, ":"},       {STRING, "bar"},
      {RBRACE, "}"},     {WHILE, "while"},   {LPAREN, "("},
      {TRUE, "true"},    {RPAREN, ")"},      {LBRACE, "{"},
      {BREAK, "break"},  {SEMICOLON, ";"},   {CONTINUE, "continue"},
      {SEMICOLON, ";"},  {RBRACE, "}"},      {FOR, "for"},
      {SEMICOLON, ";"},  {BINARY, "1010"},   {SEMICOLON, ";"},
      {HEX, "CAFE12"},   {SEMICOLON, ";"},   {NUMBER, "1"},
      {LSHIFT, "<<"},    {NUMBER, "2"},      {SEMICOLON, ";"},
      {NUMBER, "1"},     {RSHIFT, ">>"},     {NUMBER, "2"},
      {SEMICOLON, ";"},  {NUMBER, "1"},      {BXOR, "^"},
      {NUMBER, "2"},     {SEMICOLON, ";"},   {NUMBER, "1"},
      {BAND, "&"},       {NUMBER, "2"},      {SEMICOLON, ";"},
      {NUMBER, "1"},     {BOR, "|"},         {NUMBER, "2"},
      {SEMICOLON, ";"},  {NUMBER, "1"},      {MOD, "%"},
      {NUMBER, "2"},     {SEMICOLON, ";"},   {TRUE, "true"},
      {AND, "&&"},       {FALSE, "false"},   {SEMICOLON, ";"},
      {TRUE, "true"},    {OR, "||"},         {FALSE, "false"},
      {SEMICOLON, ";"},  {NUMBER, "1.5"},    {END_OF_FILE, "\0"},
  };

  Lexer *lexer = new_lexer(input);

  test_tokens(lexer, tests, LEN(tests));
}

void test_file_lexing(void) {
  Lexer *lexer = new_file_lexer("src/lexer/tests/01.monk");

  TestCase tests[] = {
      {LET, "let"},     {IDENT, "a"},     {ASSIGN, "="},       {NUMBER, "1"}, 
      {SEMICOLON, ";"}, {LET, "let"},     {IDENT, "b"},        {ASSIGN, "="},
      {NUMBER, "5"},    {SEMICOLON, ";"}, {IDENT, "a"},        {PLUS, "+"},
      {IDENT, "b"},     {SEMICOLON, ";"}, {END_OF_FILE, "\0"},
  };

  test_tokens(lexer, tests, LEN(tests));
}

void test_file_larger_than_buffer(void) {
  Lexer *lexer = new_file_lexer("src/lexer/tests/02.monk");

  TestCase tests[] = {
    {LET, "let"}, {IDENT, "first"},   {ASSIGN, "="}, {NUMBER, "10"}, {SEMICOLON, ";"},
    {LET, "let"}, {IDENT, "second"},  {ASSIGN, "="}, {NUMBER, "10"}, {SEMICOLON, ";"},
    {LET, "let"}, {IDENT, "third"},   {ASSIGN, "="}, {NUMBER, "10"}, {SEMICOLON, ";"},
    {LET, "let"}, {IDENT, "fourth"},  {ASSIGN, "="}, {NUMBER, "10"}, {SEMICOLON, ";"},
    {LET, "let"}, {IDENT, "fifth"},   {ASSIGN, "="}, {NUMBER, "10"}, {SEMICOLON, ";"},
    {LET, "let"}, {IDENT, "sixth"},   {ASSIGN, "="}, {NUMBER, "10"}, {SEMICOLON, ";"},
    {LET, "let"}, {IDENT, "seventh"}, {ASSIGN, "="}, {NUMBER, "10"}, {SEMICOLON, ";"},
  };

  test_tokens(lexer, tests, LEN(tests));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_next_token);
  RUN_TEST(test_file_lexing);
  RUN_TEST(test_file_larger_than_buffer);
  return UNITY_END();
}
