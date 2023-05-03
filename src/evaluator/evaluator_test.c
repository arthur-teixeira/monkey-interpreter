#include "../evaluator/evaluator.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_eval_integer_expression);
  RUN_TEST(test_eval_boolean_expression);
  RUN_TEST(test_bang_operator);
  UNITY_END();
}
