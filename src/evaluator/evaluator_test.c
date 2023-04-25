#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "../evaluator/evaluator.h"
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
  printf("deez nuts %ld\n", intt->value);
  TEST_ASSERT_EQUAL_INT64(expected, intt->value);
}

void test_eval_integer_expression(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
      {"5", 5},
      {"10", 10},
  };

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    test_integer_object(evaluated, tests[i].expected);

    free(evaluated);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_eval_integer_expression);
  UNITY_END();
}
