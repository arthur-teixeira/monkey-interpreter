#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "parser.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void check_parser_errors(Parser *p) {
  if (p->errors->size == 0) {
    return;
  }

  printf("ERROR: parser has %zu errors\n", p->errors->size);

  Node *n = p->errors->tail;
  while(n != NULL) {
    char *value = n->value;
    printf("Parser error: %s\n", value);

    n = n->next;
  }
  TEST_FAIL();
}

void test_let_statements() {
  char input[] = "let x = 5;"
                 "let y = 10;"
                 "let foobar = 838383;";
  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);

  Program *program = parse_program(p);
  check_parser_errors(p);
  UNITY_TEST_ASSERT_NOT_NULL(program, __LINE__, "parse_program returned null");

  TEST_ASSERT_EQUAL(3, program->statements->size);

  typedef struct {
    char expected_identifier[100];
  } TestCase;

  TestCase tests[] = {{"x"}, {"y"}, {"foobar"}};

  Node *stmt_node = program->statements->tail;
  for (uint32_t i = 0; i < sizeof(tests) / sizeof(TestCase); i++) {
    if (i > 0) {
        stmt_node = stmt_node->next;
    }

    Statement *stmt = stmt_node->value;

    TEST_ASSERT_EQUAL_STRING("let", stmt->token.literal);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_identifier, stmt->name->value);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_identifier,
                             stmt->name->token.literal);
  }
}

void test_return_statements(void) {
  char input[] = "return 5;"
                 "return 10;"
                 "return 993322;";

  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);

  Program *program = parse_program(p);
  check_parser_errors(p);

  UNITY_TEST_ASSERT_NOT_NULL(program, __LINE__, "parse_program returned null");
  TEST_ASSERT_EQUAL(3, program->statements->size);

  Node *stmt_node = program->statements->tail;
  while (stmt_node != NULL) {
    Statement *stmt = stmt_node->value;
    TEST_ASSERT_EQUAL_STRING("return", stmt->token.literal);
    stmt_node = stmt_node->next;
  }
}

void test_identifier_expression(void) {
  char *input = "foobar;";

  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);

  Program *program = parse_program(p);
  check_parser_errors(p);

  TEST_ASSERT_EQUAL(1, program->statements->size);

  Statement *stmt = program->statements->tail->value;
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(IDENT_EXPR, stmt->value->type);

  Identifier *ident = stmt->value->value;
  TEST_ASSERT_EQUAL_STRING("foobar", ident->token.literal);
  TEST_ASSERT_EQUAL_STRING("foobar", ident->value);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_let_statements);
  RUN_TEST(test_return_statements);
  RUN_TEST(test_identifier_expression);
  UNITY_END();
}
