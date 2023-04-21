#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "parser.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void check_parser_errors(Parser *p) {
  if (p->errors->size == 0) {
    return;
  }

  printf("ERROR: parser has %zu errors\n", p->errors->size);

  Node *n = p->errors->tail;
  while (n != NULL) {
    char *value = n->value;
    printf("Parser error: %s\n", value);

    n = n->next;
  }
  TEST_FAIL();
}

Program *parse_and_check_errors(char *input) {
  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);

  Program *program = parse_program(p);
  check_parser_errors(p);

  return program;
}

void test_let_statements() {
  char input[] = "let x = 5;"
                 "let y = 10;"
                 "let foobar = 838383;";

  Program *program = parse_and_check_errors(input);
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

  Program *program = parse_and_check_errors(input);

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

  Program *program = parse_and_check_errors(input);

  TEST_ASSERT_EQUAL(1, program->statements->size);

  Statement *stmt = program->statements->tail->value;
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(IDENT_EXPR, stmt->expression->type);

  Identifier *ident = stmt->expression->value;
  TEST_ASSERT_EQUAL_STRING("foobar", ident->token.literal);
  TEST_ASSERT_EQUAL_STRING("foobar", ident->value);
}

void test_integer_literal_expression(void) {
  char *input = "5;";
  Program *program = parse_and_check_errors(input);

  TEST_ASSERT_EQUAL(1, program->statements->size);

  Statement *stmt = program->statements->tail->value;
  TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
  TEST_ASSERT_EQUAL(INT_EXPR, stmt->expression->type);

  IntegerLiteral *literal = stmt->expression->value;
  TEST_ASSERT_EQUAL_INT64(5, literal->value);
  TEST_ASSERT_EQUAL_STRING("5", literal->token.literal);
}

void test_integer_literal(Expression *expr, long value) {
  TEST_ASSERT_EQUAL(INT_EXPR, expr->type);
  IntegerLiteral *lit = expr->value;

  TEST_ASSERT_EQUAL_INT64(value, lit->value);

  char as_string[10];
  sprintf(as_string, "%zu", value);
  TEST_ASSERT_EQUAL_STRING(as_string, lit->token.literal);
}

void test_parsing_prefix_expressions(void) {
  struct prefix_test_case {
    char *input;
    char *operator;
    long integer_value;
  };

  struct prefix_test_case prefix_tests[] = {{"!5;", "!", 5}, {"-15;", "-", 15}};

  for (uint32_t i = 0;
       i < sizeof(prefix_tests) / sizeof(struct prefix_test_case); i++) {
    Program *p = parse_and_check_errors(prefix_tests[i].input);

    TEST_ASSERT_EQUAL(1, p->statements->size);

    Statement *stmt = p->statements->tail->value;
    TEST_ASSERT_EQUAL(EXPR_STATEMENT, stmt->type);
    TEST_ASSERT_EQUAL(PREFIX_EXPR, stmt->expression->type);

    PrefixExpression *exp = stmt->expression->value;
    TEST_ASSERT_EQUAL_STRING(prefix_tests[i].operator, exp->operator);

    test_integer_literal(exp->right, prefix_tests[i].integer_value);

    free_program(p);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_let_statements);
  RUN_TEST(test_return_statements);
  RUN_TEST(test_identifier_expression);
  RUN_TEST(test_integer_literal_expression);
  RUN_TEST(test_parsing_prefix_expressions);
  UNITY_END();
}
