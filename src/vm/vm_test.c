#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "../ast/ast.h"
#include "../parser/parser.h"
#include "../lexer/lexer.h"
#include "../object/object.h"
#include "../compiler/compiler.h"
#include "vm.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct {
    char *input;
    int expected;
} vmTestCase;

Program *parse(char *input) {
  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);
  Program *program = parse_program(p);
  free_parser(p);
  return program;
}

void test_integer_object(int expected, Object *actual) {
  TEST_ASSERT_EQUAL(NUMBER_OBJ, actual->type);
  TEST_ASSERT_EQUAL(expected, ((Number *)actual)->value);
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
        TEST_ASSERT_TRUE(vm->sp > 0);

        Object *top = stack_top(vm);
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
    };

    run_vm_tests(tests, ARRAY_LEN(tests));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_integer_arithmetic);
    return UNITY_END();
}
