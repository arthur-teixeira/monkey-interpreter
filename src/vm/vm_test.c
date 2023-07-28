#include "../ast/ast.h"
#include "../compiler/compiler.h"
#include "../lexer/lexer.h"
#include "../object/constants.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "vm.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define VM_RUN_TESTS(tests, type)                                              \
  run_vm_tests(tests, sizeof(tests[0]), ARRAY_LEN(tests), type);

#define EXPECTED_NULL_SENTINEL 98534921

typedef enum {
  VM_TEST_INTEGER,
  VM_TEST_BOOLEAN,
  VM_TEST_NULL,
  VM_TEST_STRING,
  VM_TEST_ARRAY,
} vmTestCaseType;

typedef struct {
  char *input;
  bool expected;
} vmBooleanTestCase;

typedef struct {
  char *input;
  int64_t expected;
} vmIntTestCase;

typedef struct {
  char *input;
} vmNullTestCase;

typedef struct {
  char *input;
  char *expected;
} vmStringTestCase;

typedef struct {
  char *input;
  int64_t expected[100];
  size_t expected_len;
} vmArrayTestCase;

Program *parse(void *test, vmTestCaseType type) {
  char *input;
  switch (type) {
  case VM_TEST_INTEGER:
    input = ((vmIntTestCase *)test)->input;
    break;
  case VM_TEST_BOOLEAN:
    input = ((vmBooleanTestCase *)test)->input;
    break;
  case VM_TEST_NULL:
    input = ((vmNullTestCase *)test)->input;
    break;
  case VM_TEST_STRING:
    input = ((vmStringTestCase *)test)->input;
    break;
  case VM_TEST_ARRAY:
    input = ((vmArrayTestCase *)test)->input;
    break;
  }

  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);
  Program *program = parse_program(p);
  free_parser(p);
  return program;
}

void test_integer_object(double expected, Object *actual) {
  if (expected == EXPECTED_NULL_SENTINEL) {
    TEST_ASSERT_EQUAL_PTR(&obj_null, actual);
    return;
  }

  TEST_ASSERT_EQUAL(NUMBER_OBJ, actual->type);
  TEST_ASSERT_EQUAL_INT64(expected, ((Number *)actual)->value);
}

void test_boolean_object(bool expected, Object *actual) {
  TEST_ASSERT_EQUAL(BOOLEAN_OBJ, actual->type);
  TEST_ASSERT_EQUAL(expected, ((Boolean *)actual)->value);
}

void test_string_object(char *expected, Object *actual) {
  TEST_ASSERT_EQUAL(STRING_OBJ, actual->type);
  TEST_ASSERT_EQUAL_STRING(expected, ((String *)actual)->value);
}

void test_expected_object(void *expected, Object *actual, vmTestCaseType type) {
  switch (type) {
  case VM_TEST_INTEGER:
    test_integer_object(((vmIntTestCase *)expected)->expected, actual);
    break;
  case VM_TEST_BOOLEAN:
    test_boolean_object(((vmBooleanTestCase *)expected)->expected, actual);
    break;
  case VM_TEST_NULL:
    TEST_ASSERT_EQUAL(NULL_OBJ, actual->type);
    break;
  case VM_TEST_STRING:
    test_string_object(((vmStringTestCase *)expected)->expected, actual);
    break;
  case VM_TEST_ARRAY: {
    vmArrayTestCase *test = (vmArrayTestCase *)expected;
    TEST_ASSERT_EQUAL_INT64(test->expected_len,
                            ((Array *)actual)->elements.len);
    for (size_t i = 0; i < test->expected_len; i++) {
      test_integer_object(test->expected[i],
                          ((Array *)actual)->elements.arr[i]);
    }
    break;
  }
  }
}

void run_vm_tests(void *tests, size_t test_size, size_t tests_count,
                  vmTestCaseType type) {
  for (size_t i = 0; i < tests_count; i++) {
    void *test = tests + (i * test_size);
    Program *program = parse(test, type);
    Compiler *compiler = new_compiler();
    CompilerResult result = compile_program(compiler, program);

    if (result != COMPILER_OK) {
      char msg[100];
      compiler_error(result, msg, 100);
      TEST_FAIL_MESSAGE(msg);
    }

    VM *vm = new_vm(bytecode(compiler));
    VMResult vm_result = run_vm(vm);
    if (vm_result != VM_OK) {
      char msg[100];
      vm_error(vm_result, msg, 100);
      TEST_FAIL_MESSAGE(msg);
    }

    Object *top = vm_last_popped_stack_elem(vm);
    test_expected_object(test, top, type);

    free_program(program);
    free_compiler(compiler);
    free_vm(vm);
  }
}

void test_integer_arithmetic(void) {
  vmIntTestCase tests[] = {
      {"1", 1},
      {"2", 2},
      {"1 + 2", 3},
      {"1 - 2", -1},
      {"1 * 2", 2},
      {"4 / 2", 2},
      {"50 / 2 * 2 + 10 - 5", 55},
      {"5 + 5 + 5 + 5 - 10", 10},
      {"2 * 2 * 2 * 2 * 2", 32},
      {"5 * 2 + 10", 20},
      {"5 + 2 * 10", 25},
      {"5 * (2 + 10)", 60},
      {"4 | 1", 5},
      {"2 & 1", 0},
      {"2 ^ 1", 3},
      {"2 << 1", 4},
      {"2 >> 1", 1},
      {"3 % 2", 1},
      {"3 % 3", 0},
      {"-5", -5},
      {"-10", -10},
      {"-50 + 100 + -50", 0},
      {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
  };

  VM_RUN_TESTS(tests, VM_TEST_INTEGER);
}

void test_boolean_expressions(void) {
  vmBooleanTestCase tests[] = {
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
      {"!true", false},
      {"!false", true},
      {"!5", false},
      {"!!true", true},
      {"!!false", false},
      {"!!5", true},
  };

  VM_RUN_TESTS(tests, VM_TEST_BOOLEAN);
}

void test_conditionals(void) {
  vmIntTestCase tests[] = {
      {"if (true) { 10 }", 10},
      {"if (true) { 10 } else { 20 }", 10},
      {"if (false) { 10 } else { 20 } ", 20},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 < 2) { 10 } else { 20 }", 10},
      {"if (1 > 2) { 10 } else { 20 }", 20},
      {"if ((if (false) { 10 })) { 10 } else { 20 }", 20},
  };

  VM_RUN_TESTS(tests, VM_TEST_INTEGER);

  vmNullTestCase null_tests[] = {
      {"if (false) { 10; }"},
      {"if (1 > 2) { 10; }"},
  };

  VM_RUN_TESTS(null_tests, VM_TEST_NULL);

  vmBooleanTestCase bool_tests[] = {
      {"!(if (false) { 10; })", true},
  };

  VM_RUN_TESTS(bool_tests, VM_TEST_BOOLEAN);
}

void test_global_let_statements(void) {
  vmIntTestCase tests[] = {
      {"let one = 1; one", 1},
      {"let one = 1; let two = 2; one + two", 3},
      {"let one = 1; let two = one + one; one + two", 3},
  };

  VM_RUN_TESTS(tests, VM_TEST_INTEGER);
}

void test_string_expressions(void) {
  vmStringTestCase tests[] = {
      {"\"monkey\"", "monkey"},
      {"\"mon\" + \"key\"", "monkey"},
      {"\"mon\" + \"key\" + \"banana\"", "monkeybanana"},
  };

  VM_RUN_TESTS(tests, VM_TEST_STRING);
}

void test_array_literals(void) {
  vmArrayTestCase tests[] = {
      {"[]", {}, 0},
      {"[1, 2, 3]", {1, 2, 3}, 3},
      {"[1 + 2, 3 * 4, 5 + 6]", {3, 12, 11}, 3},
  };

  VM_RUN_TESTS(tests, VM_TEST_ARRAY);
}

void test_index_expressions(void) {
  vmIntTestCase tests[] = {
      {"[1, 2, 3][1]", 2},
      {"[1, 2, 3][0 + 2]", 3},
      {"[[1, 1, 1]][0][0]", 1},
      {"[][0]", -1},
      {"[1, 2, 3][99]", EXPECTED_NULL_SENTINEL},
      {"[1][-1]", EXPECTED_NULL_SENTINEL},
      {"{1: 1, 2: 2}[1]", 1},
      {"{1: 1, 2: 2}[2]", 2},
      {"{1: 1}[0]", EXPECTED_NULL_SENTINEL},
      {"{}[0]", EXPECTED_NULL_SENTINEL},
  };

  VM_RUN_TESTS(tests, VM_TEST_INTEGER);
}

void test_calling_functions(void) {
  vmIntTestCase tests[] = {
      {
          .input = "let fivePlusTen = fn() { 5 + 10; }; fivePlusTen();",
          .expected = 15,
      },
      {
          .input = "let one = fn() { 1; };"
                   "let two = fn() { 2; };"
                   "one() + two()",
          .expected = 3,
      },
      {
          .input = "let a = fn() { 1 };"
                   "let b = fn() { a() + 1 };"
                   "let c = fn() { b() + 1 };"
                   "c(); ",
          .expected = 3,
      },
  };

  VM_RUN_TESTS(tests, VM_TEST_INTEGER)
}

void test_functions_with_return_statement(void) {
  vmIntTestCase tests[] = {
      {
          .input = "let earlyExit = fn() { return 99; 100; };"
                   "earlyExit();",
          .expected = 99,
      },
      {
          .input = "let earlyExit = fn() { return 99; return 100; };"
                   "earlyExit();",
          .expected = 99,
      },
  };

  VM_RUN_TESTS(tests, VM_TEST_INTEGER);
}

void test_functions_without_return_value(void) {
  vmNullTestCase tests[] = {
      {
          .input = "let noReturn = fn() { }; noReturn();",
      },
      {
          .input = "let noReturn = fn() { };"
                   "let noReturnTwo = fn() { noReturn(); };"
                   "noReturn(); noReturnTwo();",
      },
  };

  VM_RUN_TESTS(tests, VM_TEST_NULL);
}

void test_calling_functions_with_bindings(void) {
  vmIntTestCase tests[] = {
    {
      .input = "let one = fn() { let one = 1; one; }; one();",
      .expected = 1,
    },
    {
      .input = "let oneAndTwo = fn() { let one = 1; let two = 2; one + two; };"
               "oneAndTwo();",
      .expected = 3,
    },
    {
      .input = "let oneAndTwo = fn() { let one = 1; let two = 2; one + two; };"
        "let threeAndFour = fn() { let three = 3; let four = 4; three + four; };"
        "oneAndTwo() + threeAndFour();",
      .expected = 10,
    },
    {
      .input = "let firstFoobar = fn() { let foobar = 50; foobar; };"
        "let secondFoobar = fn() { let foobar = 100; foobar; };"
        "firstFoobar() + secondFoobar();",
      .expected = 150,
    },
    {
      .input = "let globalSeed = 50;"
        "let minusOne = fn() {"
        "  let num = 1;"
        "  globalSeed - num;"
        "};"
        "let minusTwo = fn() {"
        "  let num = 2;"
        "  globalSeed - num;"
        "};"
        "minusOne() + minusTwo();",
      .expected = 97,
    },
  };

  VM_RUN_TESTS(tests, VM_TEST_INTEGER);
}

void test_functions_with_arguments_and_bindings(void) {
  vmIntTestCase tests[] = {
    {
      .input = "let identity = fn(a) { a; };"
        "identity(4);",
      .expected = 4,
    },
    {
      .input = "let sum = fn(a, b) { a + b; };"
        "sum(1, 2);",
      .expected = 3,
    },
  };

  VM_RUN_TESTS(tests, VM_TEST_INTEGER);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_integer_arithmetic);
  RUN_TEST(test_boolean_expressions);
  RUN_TEST(test_conditionals);
  RUN_TEST(test_global_let_statements);
  RUN_TEST(test_string_expressions);
  RUN_TEST(test_array_literals);
  RUN_TEST(test_calling_functions);
  RUN_TEST(test_functions_with_return_statement);
  RUN_TEST(test_functions_without_return_value);
  RUN_TEST(test_calling_functions_with_bindings);
  RUN_TEST(test_functions_with_arguments_and_bindings);
  return UNITY_END();
}
