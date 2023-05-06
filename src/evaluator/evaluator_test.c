#include "../evaluator/evaluator.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_LEN(arr, type) sizeof(arr) / sizeof(type)

Object *test_eval(char *input) {
  Lexer *lexer = new_lexer(input);
  Parser *parser = new_parser(lexer);
  Program *program = parse_program(parser);

  Object *result = eval_program(program);

  free_parser(parser);
  free_program(program);

  return result;
}

void test_integer_object(Object *evaluated, long expected) {
  TEST_ASSERT_EQUAL(INTEGER_OBJ, evaluated->type);
  Integer *intt = evaluated->object;
  TEST_ASSERT_EQUAL_INT64(expected, intt->value);
}

void test_boolean_object(Object *evaluated, bool expected) {
  TEST_ASSERT_EQUAL(BOOLEAN_OBJ, evaluated->type);
  Boolean *bol = evaluated->object;
  TEST_ASSERT_EQUAL(expected, bol->value);
}

void test_eval_integer_expression(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
      {"5", 5},
      {"10", 10},
      {"-5", -5},
      {"-10", -10},
      {"5 + 5 + 5 + 5 - 10", 10},
      {"2 * 2 * 2 * 2 * 2", 32},
      {"-50 + 100 + -50", 0},
      {"5 * 2 + 10", 20},
      {"5 + 2 * 10", 25},
      {"20 + 2 * -10", 0},
      {"50 / 2 * 2 + 10", 60},
      {"2 * (5 + 10)", 30},
      {"3 * 3 * 3 + 10", 37},
      {"3 * (3 * 3) + 10", 37},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
  };

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    test_integer_object(evaluated, tests[i].expected);

    free(evaluated);
  }
}

void test_eval_boolean_expression(void) {
  struct testCase {
    char *input;
    bool expected;
  };

  struct testCase tests[] = {
      {"true", true},
      {"false", false},
      {"1 < 2", true},
      {"1 > 2", false},
      {"1 < 1", false},
      {"1 > 1", false},
      {"1 == 1", true},
      {"1 != 1", false},
      {"1 == 2", false},
      {"1 != 2", true},
      {"true == true", true},
      {"false == false", true},
      {"true == false", false},
      {"true != false", true},
      {"false != true", true},
      {"(1 < 2) == true", true},
      {"(1 < 2) == false", false},
      {"(1 > 2) == true", false},
      {"(1 > 2) == false", true},
  };

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    test_boolean_object(evaluated, tests[i].expected);
  }
}

void test_bang_operator(void) {
  struct testCase {
    char *input;
    bool expected;
  };

  struct testCase tests[] = {
      {"!true", false}, {"!false", true},   {"!5", false},
      {"!!true", true}, {"!!false", false}, {"!!5", true},
  };

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    test_boolean_object(evaluated, tests[i].expected);
  }
}

void test_if_else_expressions(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
      {"if (true) { 10 }", 10},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      {"if (1 < 2) { 10 } else { 20 }", 10},
  };

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    test_integer_object(evaluated, tests[i].expected);
  }
}

void test_null_if_else_expressions(void) {
  char *tests[] = {
    "if (false) { 10 }",
    "if (1 > 2) { 10 }",
  };

  for (uint32_t i = 0; i < 2; i++) {
    Object *evaluated = test_eval(tests[i]);
    TEST_ASSERT_NULL(evaluated);
  }
}

void test_return_statements(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
    {"return 10;", 10},
    {"return 10; 9;", 10},
    {"return 2 * 5; 9;", 10},
    {"9; return 2 * 5; 9", 10},
    {"if (10 > 1) { if (10 > 1) { return 10; } return 1; }", 10},
    {"if (10 > 1) { if (1 > 10) { return 10; } return 1; }", 1},
  };

  for (uint32_t i = 0 ; i < sizeof(tests) / sizeof(struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    test_integer_object(evaluated, tests[i].expected);
  }
}

void test_error_handling(void) {
  struct testCase {
    char *input;
    char *expected_message;
  };

  struct testCase tests[] = {
    {
      "5 + true;",
      "type mismatch: INTEGER_OBJ + BOOLEAN_OBJ",
    },
    {
      "5 + true; 5;",
      "type mismatch: INTEGER_OBJ + BOOLEAN_OBJ",
    },
    {
      "-true;",
      "unknown operator: -BOOLEAN_OBJ",
    },
    {
      "true + false;",
      "unknown operator: BOOLEAN_OBJ + BOOLEAN_OBJ",
    },
    {
      "5; true + false; 5;",
      "unknown operator: BOOLEAN_OBJ + BOOLEAN_OBJ",
    },
    {
      "if (10 > 1) { true + false; }",
      "unknown operator: BOOLEAN_OBJ + BOOLEAN_OBJ",
    },
    {
      "if (10 > 1) { if (10 > 1) { return true + false; } return 1; }",
      "unknown operator: BOOLEAN_OBJ + BOOLEAN_OBJ",
    }
  };

  for (uint32_t i = 0; i < ARRAY_LEN(tests, struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    TEST_ASSERT_NOT_NULL(evaluated);

    TEST_ASSERT_EQUAL(ERROR_OBJ, evaluated->type);

    Error *error_obj = evaluated->object;
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_message, error_obj->message);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_eval_integer_expression);
  RUN_TEST(test_eval_boolean_expression);
  RUN_TEST(test_bang_operator);
  RUN_TEST(test_if_else_expressions);
  RUN_TEST(test_null_if_else_expressions);
  RUN_TEST(test_return_statements);
  RUN_TEST(test_error_handling);
  UNITY_END();
}
