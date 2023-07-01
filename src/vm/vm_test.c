#include "../ast/ast.h"
#include "../compiler/compiler.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "vm.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct {
  char *input;
  signed int expected;
} vmTestCase;

Program *parse(char *input) {
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

void test_expected_object(int expected, Object *actual) {
  test_integer_object(expected, actual);
}

void run_vm_tests(vmTestCase tests[], size_t tests_count) {
  for (size_t i = 0; i < tests_count; i++) {
    vmTestCase test = tests[i];
    Program *program = parse(test.input);
    Compiler *compiler = new_compiler();
    int8_t result = compile_program(compiler, program);
    if (result < 0) {
      TEST_FAIL_MESSAGE("Compiler error");
    }

    VM *vm = new_vm(bytecode(compiler));
    VMResult vm_result = run_vm(vm);
    if (vm_result != VM_OK) {
      TEST_FAIL_MESSAGE("Compiler error");
    }

    Object *top = vm_last_popped_stack_elem(vm);
    test_expected_object(test.expected, top);

    free_program(program);
    free_compiler(compiler);
    free_vm(vm);
  }
}

void test_integer_arithmetic(void) {
  vmTestCase tests[] = {
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
  };

  run_vm_tests(tests, ARRAY_LEN(tests));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_integer_arithmetic);
  return UNITY_END();
}
