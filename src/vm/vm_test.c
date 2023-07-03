#include "../ast/ast.h"
#include "../compiler/compiler.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "vm.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

typedef enum {
  VM_TEST_INTEGER,
  VM_TEST_BOOLEAN,
} vmTestCaseType;

typedef struct {
  char *input;
  bool expected;
} vmBooleanTestCase;

typedef struct {
  char *input;
  int64_t expected;
} vmIntTestCase;

Program *parse(void *test, vmTestCaseType type) {
  char *input;
  switch (type) {
  case VM_TEST_INTEGER:
    input = ((vmIntTestCase *)test)->input;
    break;
  case VM_TEST_BOOLEAN:
    input = ((vmBooleanTestCase *)test)->input;
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

void test_expected_object(void *expected, Object *actual, vmTestCaseType type) {
  switch (type) {
  case VM_TEST_INTEGER:
    test_integer_object(((vmIntTestCase *)expected)->expected, actual);
    break;
  case VM_TEST_BOOLEAN:
    test_boolean_object(((vmBooleanTestCase *)expected)->expected, actual);
    break;
  }
}

void run_vm_tests(void *tests, size_t test_size, size_t tests_count, vmTestCaseType type) {
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

  run_vm_tests(tests, sizeof(vmIntTestCase), ARRAY_LEN(tests), VM_TEST_INTEGER);
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

  run_vm_tests(tests, sizeof(vmBooleanTestCase), ARRAY_LEN(tests), VM_TEST_BOOLEAN);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_integer_arithmetic);
  RUN_TEST(test_boolean_expressions);
  return UNITY_END();
}
