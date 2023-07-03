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
  vmTestCaseType type;
  char *input;
  bool expected;
} vmBooleanTestCase;

typedef struct {
  vmTestCaseType type;
  char *input;
  int64_t expected;
} vmIntTestCase;

typedef struct {
  vmTestCaseType type;
} vmTestCase;

Program *parse(vmTestCase *test) {
  char *input;
  switch (test->type) {
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

void test_expected_object(vmTestCase *expected, Object *actual) {
  switch (expected->type) {
  case VM_TEST_INTEGER:
    test_integer_object(((vmIntTestCase *)expected)->expected, actual);
    break;
  case VM_TEST_BOOLEAN:
    test_boolean_object(((vmBooleanTestCase *)expected)->expected, actual);
    break;
  }
}

void run_vm_tests(void *tests, size_t test_size, size_t tests_count) {
  for (size_t i = 0; i < tests_count; i += test_size) {
    vmTestCase *test = tests + (i * test_size);
    Program *program = parse(test);
    Compiler *compiler = new_compiler();
    CompilerResult result = compile_program(compiler, program);
    if (result != COMPILER_OK) {
      TEST_FAIL_MESSAGE("Compiler error");
    }

    VM *vm = new_vm(bytecode(compiler));
    VMResult vm_result = run_vm(vm);
    if (vm_result != VM_OK) {
      TEST_FAIL_MESSAGE("Compiler error");
    }

    Object *top = vm_last_popped_stack_elem(vm);
    test_expected_object(test, top);

    free_program(program);
    free_compiler(compiler);
    free_vm(vm);
  }
}

void test_integer_arithmetic(void) {
  vmIntTestCase tests[] = {
      {VM_TEST_INTEGER, "1", 1},
      {VM_TEST_INTEGER, "2", 2},
      {VM_TEST_INTEGER, "1 + 2", 3},
      {VM_TEST_INTEGER, "1 - 2", -1},
      {VM_TEST_INTEGER, "1 * 2", 2},
      {VM_TEST_INTEGER, "4 / 2", 2},
      {VM_TEST_INTEGER, "50 / 2 * 2 + 10 - 5", 55},
      {VM_TEST_INTEGER, "5 + 5 + 5 + 5 - 10", 10},
      {VM_TEST_INTEGER, "2 * 2 * 2 * 2 * 2", 32},
      {VM_TEST_INTEGER, "5 * 2 + 10", 20},
      {VM_TEST_INTEGER, "5 + 2 * 10", 25},
      {VM_TEST_INTEGER, "5 * (2 + 10)", 60},
      {VM_TEST_INTEGER, "4 | 1", 5},
      {VM_TEST_INTEGER, "2 & 1", 0},
      {VM_TEST_INTEGER, "2 ^ 1", 3},
      {VM_TEST_INTEGER, "2 << 1", 4},
      {VM_TEST_INTEGER, "2 >> 1", 1},
      {VM_TEST_INTEGER, "3 % 2", 1},
      {VM_TEST_INTEGER, "3 % 3", 0},
  };

  run_vm_tests(tests, sizeof(vmIntTestCase), ARRAY_LEN(tests));
}

void test_boolean_expressions(void) {
  vmBooleanTestCase tests[] = {
      {VM_TEST_BOOLEAN, "true", true},
      {VM_TEST_BOOLEAN, "false", false},
      {VM_TEST_BOOLEAN, "1 < 2", true},
      {VM_TEST_BOOLEAN, "1 > 2", false},
      {VM_TEST_BOOLEAN, "1 < 1", false},
      {VM_TEST_BOOLEAN, "1 > 1", false},
      {VM_TEST_BOOLEAN, "1 == 1", true},
      {VM_TEST_BOOLEAN, "1 != 1", false},
      {VM_TEST_BOOLEAN, "1 == 2", false},
      {VM_TEST_BOOLEAN, "1 != 2", true},
      {VM_TEST_BOOLEAN, "true == true", true},
      {VM_TEST_BOOLEAN, "false == false", true},
      {VM_TEST_BOOLEAN, "true == false", false},
      {VM_TEST_BOOLEAN, "true != false", true},
      {VM_TEST_BOOLEAN, "false != true", true},
      {VM_TEST_BOOLEAN, "(1 < 2) == true", true},
      {VM_TEST_BOOLEAN, "(1 < 2) == false", false},
      {VM_TEST_BOOLEAN, "(1 > 2) == true", false},
      {VM_TEST_BOOLEAN, "(1 > 2) == false", true},
  };

  run_vm_tests(tests, sizeof(vmBooleanTestCase), ARRAY_LEN(tests));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_integer_arithmetic);
  RUN_TEST(test_boolean_expressions);
  return UNITY_END();
}
