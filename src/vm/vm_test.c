#include "../ast/ast.h"
#include "../compiler/compiler.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "vm.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define VM_RUN_TESTS(tests, type)                                                 \
  run_vm_tests(tests, sizeof(tests[0]), ARRAY_LEN(tests), type);

typedef enum {
  VM_TEST_INTEGER,
  VM_TEST_BOOLEAN,
  VM_TEST_NULL,
  VM_TEST_STRING,
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
  }

  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);
  Program *program = parse_program(p);
  free_parser(p);
  return program;
}

void test_integer_object(double expected, Object *actual) {
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
      {"if ((if (false) { 10 })) { 10 } else { 20 }", 20}
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

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_integer_arithmetic);
  RUN_TEST(test_boolean_expressions);
  RUN_TEST(test_conditionals);
  RUN_TEST(test_global_let_statements);
  RUN_TEST(test_string_expressions);
  return UNITY_END();
}
