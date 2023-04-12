#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "parser.h"
#include <stdbool.h>
#include <stdint.h>

void test_let_statements() {
  char input[] = "let x = 5;"
                 "let y = 10;"
                 "let foobar = 838383;";
  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);

  Program *program = parse_program(p);
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

    Statement *stmt = (Statement *)stmt_node->value;

    TEST_ASSERT_EQUAL_STRING("let", stmt->token.literal);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_identifier, stmt->name->value);
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_identifier,
                             stmt->name->token.literal);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_let_statements);
  UNITY_END();
}
