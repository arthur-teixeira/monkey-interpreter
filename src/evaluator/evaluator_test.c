#include "../evaluator/evaluator.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(arr, type) sizeof(arr) / sizeof(type)

Object *test_eval(char *input) {
  Lexer *lexer = new_lexer(input);
  Parser *parser = new_parser(lexer);
  Program *program = parse_program(parser);

  Environment *env = new_environment();

  Object *result = eval_program(program, env);

  free_parser(parser);
  free_program(program);
  free_environment(env);

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

  for (uint32_t i = 0; i < sizeof(tests) / sizeof(struct testCase); i++) {
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
      },
      {
          "foobar;",
          "undeclared identifier 'foobar'",
      },
      {
          "\"Hello\" - \"world\"",
          "unknown operator: STRING_OBJ - STRING_OBJ",
      },
      {
          "{\"name\": \"Monkey\"}[fn(x) { x }];",
          "unusable as hash key: FUNCTION_OBJ",
      },
      {
          "a = 34",
          "a is not defined",
      },
  };

  for (uint32_t i = 0; i < ARRAY_LEN(tests, struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    TEST_ASSERT_NOT_NULL(evaluated);

    TEST_ASSERT_EQUAL(ERROR_OBJ, evaluated->type);

    Error *error_obj = evaluated->object;
    TEST_ASSERT_EQUAL_STRING(tests[i].expected_message, error_obj->message);
  }
}

void test_let_statements(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
      {"let a = 5; a;", 5},
      {"let a = 5 * 5; a;", 25},
      {"let a = 5; let b = a; b;", 5},
      {"let a = 5; let b = a; let c = a + b + 5; c;", 15},
  };

  for (uint32_t i = 0; i < ARRAY_LEN(tests, struct testCase); i++) {
    test_integer_object(test_eval(tests[i].input), tests[i].expected);
  }
}

void test_function_object(void) {
  char *input = "fn(x) { x + 2; };";

  Object *evaluated = test_eval(input);
  TEST_ASSERT_EQUAL(FUNCTION_OBJ, evaluated->type);

  Function *fn = evaluated->object;

  TEST_ASSERT_EQUAL(1, fn->parameters->size);

  Identifier *param = fn->parameters->tail->value;
  ResizableBuffer buf;
  init_resizable_buffer(&buf, 1000);

  ident_expr_to_string(&buf, param);

  TEST_ASSERT_EQUAL_STRING("x", buf.buf);

  memset(buf.buf, 0, 1000);

  block_to_string(&buf, fn->body);
  TEST_ASSERT_EQUAL_STRING("(x + 2)", buf.buf);

  free(buf.buf);
}

void test_function_application(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
      {"let identity = fn(x) { x; }; identity(5);", 5},
      {"let identity = fn(x) { return x; }; identity(5);", 5},
      {"let double = fn(x) { 2 * x; }; double(5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
      {"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
      {"fn(x) { x; }(5)", 5},
  };

  for (uint32_t i = 0; i < ARRAY_LEN(tests, struct testCase); i++) {
    test_integer_object(test_eval(tests[i].input), tests[i].expected);
  }
}

void test_closures(void) {
  char *input = "let newAdder = fn(x) {   "
                "  fn(y) { x + y };       "
                "};                       "
                "                         "
                "let addTwo = newAdder(2);"
                "addTwo(2);               ";

  test_integer_object(test_eval(input), 4);
}

void test_string_literal(void) {
  char *input = "\"Hello world\"";

  Object *evaluated = test_eval(input);

  TEST_ASSERT_EQUAL(STRING_OBJ, evaluated->type);

  String *str = evaluated->object;

  TEST_ASSERT_EQUAL_STRING("Hello world", str->value);
}

void test_string_concatenation(void) {
  char *input = "\"Hello\"+ \" \" + \"World!\"";
  Object *evaluated = test_eval(input);
  TEST_ASSERT_EQUAL(STRING_OBJ, evaluated->type);

  String *str = evaluated->object;

  char *expected_string = "Hello World!";

  TEST_ASSERT_EQUAL(strlen(expected_string), str->len);
  TEST_ASSERT_EQUAL_STRING(expected_string, str->value);
}

void test_builtin_len_function(void) {
  struct testCase {
    char *input;
    char *expected_error;
    long expected_value;
  };

  struct testCase tests[] = {
      {"len(\"\")", NULL, 0},
      {"len(\"four\")", NULL, 4},
      {"len(3)", "argument to 'len' not supported, got INTEGER_OBJ", -1},
      {"len(\"one\", \"two\")", "wrong number of arguments: Expected 1 got 2",
       -1},
      {"len([1,2,3])", NULL, 3},
      {"len([])", NULL, 0},
  };

  for (uint32_t i = 0; i < ARRAY_LEN(tests, struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);

    if (tests[i].expected_error != NULL) {
      TEST_ASSERT_EQUAL(ERROR_OBJ, evaluated->type);
      Error *err = evaluated->object;
      TEST_ASSERT_EQUAL_STRING(tests[i].expected_error, err->message);
    } else {
      test_integer_object(evaluated, tests[i].expected_value);
    }
  }
}

void test_array_literals(void) {
  char *input = "[1, 2 * 2, 3 + 3]";

  Object *evaluated = test_eval(input);

  TEST_ASSERT_EQUAL(ARRAY_OBJ, evaluated->type);

  Array *arr = evaluated->object;

  test_integer_object(arr->elements.arr[0], 1);
  test_integer_object(arr->elements.arr[1], 4);
  test_integer_object(arr->elements.arr[2], 6);
}

void test_array_indexing(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
      {
          "[1, 2, 3][0]",
          1,
      },
      {
          "[1, 2, 3][1]",
          2,
      },
      {
          "[1, 2, 3][2]",
          3,
      },
      {
          "let i = 0; [1][i];",
          1,
      },
      {
          "[1, 2, 3][1 + 1];",
          3,
      },
      {
          "let myArray = [1, 2, 3]; myArray[2];",
          3,
      },
      {
          "let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];",
          6,
      },
      {
          "let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]",
          2,
      },
      {
          "[1, 2, 3][3]",
          -1,
      },
      {
          "[1, 2, 3][-1]",
          -1,
      },
  };

  for (size_t i = 0; i < ARRAY_LEN(tests, struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    if (tests[i].expected < 0) {
      TEST_ASSERT_EQUAL(NULL_OBJ, evaluated->type);
    } else {
      test_integer_object(evaluated, tests[i].expected);
    }
  }
}

void test_builtin_array_functions(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
      {"let a = [1, 2, 3]; first(a);", 1},
      {"first([4 + 4, 2, 3]);", 8},
      {"first([])", -1},
      {"let a = [1, 2, 3]; last(a);", 3},
      {"last([4 + 4, 2, 3 * 3]);", 9},
      {"last([])", -1},
  };

  for (size_t i = 0; i < ARRAY_LEN(tests, struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    if (tests[i].expected < 0) {
      TEST_ASSERT_EQUAL(NULL_OBJ, evaluated->type);
    } else {
      test_integer_object(evaluated, tests[i].expected);
    }
  }
}

Object *wrap_object(void *str, ObjectType type) {
  Object *obj = malloc(sizeof(Object));
  obj->type = type;
  obj->object = str;

  return obj;
}

int iter_hash_literal_test(void *generated_map, hashmap_element_t *pair) {
  hashmap_t *map = generated_map;

  const int32_t *key = pair->key;

  HashPair *value = hashmap_get(map, key, sizeof(int32_t));
  TEST_ASSERT_NOT_NULL(value);

  test_integer_object(&value->value, *(long *)pair->data);

  return 0;
}

void test_hash_literals(void) {
  char *input = ""
                "let two = \"two\";           "
                "{                            "
                "    \"one\": 10-9,           "
                "    two: 1 + 1,              "
                "    \"thr\" + \"ee\": 6 / 2, "
                "    4: 4,                    "
                "}                            ";

  Object *evaluated = test_eval(input);
  TEST_ASSERT_EQUAL(HASH_OBJ, evaluated->type);

  Hash *hash = evaluated->object;

  hashmap_t expected_map;
  hashmap_create(4, &expected_map);

  String one = {"one", strlen("one")};
  Object *one_obj = wrap_object(&one, STRING_OBJ);
  int32_t one_key = get_hash_key(one_obj);
  long one_val = 1;

  hashmap_put(&expected_map, &one_key, sizeof(int32_t), &one_val);

  String two = {"two", strlen("two")};
  Object *two_obj = wrap_object(&two, STRING_OBJ);
  int32_t two_key = get_hash_key(two_obj);
  long two_val = 2;

  hashmap_put(&expected_map, &two_key, sizeof(int32_t), &two_val);

  String three = {"three", strlen("three")};
  Object *three_obj = wrap_object(&three, STRING_OBJ);
  int32_t three_key = get_hash_key(three_obj);
  long three_val = 3;

  hashmap_put(&expected_map, &three_key, sizeof(int32_t), &three_val);

  Integer four = {4};
  Object *four_obj = wrap_object(&four, INTEGER_OBJ);
  int32_t four_key = get_hash_key(four_obj);
  long four_val = 4;

  hashmap_put(&expected_map, &four_key, sizeof(int32_t), &four_val);

  hashmap_iterate_pairs(&expected_map, &iter_hash_literal_test, &hash->pairs);

  free(one_obj);
  free(two_obj);
  free(three_obj);
  free(four_obj);
}

void test_hash_index_expressions(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
      {
          "{\"foo\": 5}[\"foo\"]",
          5,
      },
      {
          "{\"foo\": 5}[\"bar\"]",
          -1,
      },
      {
          "let key = \"foo\"; {\"foo\": 5}[key]",
          5,
      },
      {
          "{}[\"foo\"]",
          -1,
      },
      {
          "{5: 5}[5]",
          5,
      },
      {
          "{true: 5}[true]",
          5,
      },
  };

  for (size_t i = 0; i < ARRAY_LEN(tests, struct testCase); i++) {
    Object *evaluated = test_eval(tests[i].input);
    if (tests[i].expected >= 0) {
      test_integer_object(evaluated, tests[i].expected);
    } else {
      TEST_ASSERT_EQUAL(NULL_OBJ, evaluated->type);
    }
  }
}

void test_while_loops(void) {
  char *input = "let b = 0;     "
                "while (b < 5) {"
                " b = b + 1;    "
                "}"
                "b;";

  Object *evaluated = test_eval(input);
  test_integer_object(evaluated, 5);
}

void test_loop_break(void) {
  char *input = "let b = 0;     "
                "while (b < 5) {"
                " b = 3;        "
                " break;        "
                "}"
                "b;";

  Object *evaluated = test_eval(input);
  test_integer_object(evaluated, 3);
}

void test_reassignment(void) {
  char *input = "let b = 0; "
                "b = 5;     "
                "b;";

  Object *evaluated = test_eval(input);
  test_integer_object(evaluated, 5);
}

void test_bit_shift(void) {
  char *inputs[] = {
    "2 << 1;",
    "8 >> 1;",
  };

  for (int i = 0; i < 2; ++i) {
    Object *evaluated = test_eval(inputs[i]);
    test_integer_object(evaluated, 4);
  }
}

void test_bitwise(void) {
  struct testCase {
    char *input;
    long expected;
  };

  struct testCase tests[] = {
    {"57 | 34", 59},
    {"55 & 123", 51},
    {"120 ^ 392", 496},
  };

  for (size_t i = 0; i < ARRAY_LEN(tests, struct testCase); ++i) {
    Object *evaluated = test_eval(tests[i].input);
    test_integer_object(evaluated, tests[i].expected);
  }
}

void test_and_and_or(void) {
  struct testCase {
    char *input;
    bool expected;
  };

  struct testCase tests[] = {
    {"true && true", true},
    {"true && false", false},
    {"false && true", false},
    {"false && false", false},
    {"true || true", true},
    {"true || false", true},
    {"false || true", true},
    {"false || false", false},
  };

  for (size_t i = 0; i < ARRAY_LEN(tests, struct testCase); ++i) {
    Object *evaluated = test_eval(tests[i].input);
    test_boolean_object(evaluated, tests[i].expected);
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
  RUN_TEST(test_let_statements);
  RUN_TEST(test_function_object);
  RUN_TEST(test_function_application);
  RUN_TEST(test_closures);
  RUN_TEST(test_string_literal);
  RUN_TEST(test_string_concatenation);
  RUN_TEST(test_builtin_len_function);
  RUN_TEST(test_array_literals);
  RUN_TEST(test_array_indexing);
  RUN_TEST(test_builtin_array_functions);
  RUN_TEST(test_hash_index_expressions);
  RUN_TEST(test_hash_literals);
  RUN_TEST(test_while_loops);
  RUN_TEST(test_loop_break);
  RUN_TEST(test_reassignment);
  RUN_TEST(test_bit_shift);
  RUN_TEST(test_bitwise);
  RUN_TEST(test_and_and_or);
  return UNITY_END();
}
