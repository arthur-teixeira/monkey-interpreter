#include "../evaluator/evaluator.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "compiler.h"
#include <stdint.h>

typedef struct {
  char *input;
  size_t expected_constants_len;
  size_t expected_constants[100];
  size_t expected_instructions_len;
  Instruction expected_instructions[100];
} compilerTestCase;

Program *parse(char *input) {
  Lexer *l = new_lexer(input);
  Parser *p = new_parser(l);
  Program *program = parse_program(p);
  free_parser(p);
  return program;
}

Instructions concat_instructions(size_t instructions_count,
                                 Instruction instructions[]) {
  Instructions out;
  int_array_init(&out, instructions_count);

  for (uint32_t i = 0; i < instructions_count; i++) {
    Instruction ins = instructions[i];
    for (uint32_t j = 0; j < ins.len; j++) {
      int_array_append(&out, ins.arr[j]);
    }
  }

  return out;
}

void test_instructions(size_t instructions_count, Instruction expected[],
                       Instructions actual) {
  Instructions concatted = concat_instructions(instructions_count, expected);
  TEST_ASSERT_EQUAL(concatted.len, actual.len);

  for (uint32_t i = 0; i < instructions_count; i++) {
    TEST_ASSERT_EQUAL(concatted.arr[i], actual.arr[i]);
  }

  int_array_free(&concatted);
}

void test_integer_object(int expected, Object *actual) {
  TEST_ASSERT_EQUAL(NUMBER_OBJ, actual->type);
  TEST_ASSERT_EQUAL(expected, ((Number *)actual)->value);
}

void test_constants(size_t constants_count, size_t expected[],
                    DynamicArray actual) {
  TEST_ASSERT_EQUAL(constants_count, actual.len);

  for (uint32_t i = 0; i < constants_count; i++) {
    test_integer_object(expected[i], actual.arr[i]);
  }
}

void run_compiler_tests(compilerTestCase tests[], size_t test_count) {
  for (uint32_t i = 0; i < test_count; i++) {
    compilerTestCase test = tests[i];

    Program *program = parse(test.input);
    Compiler *compiler = new_compiler();
    int8_t result = compile_program(compiler, program);
    if (result < 0) {
      TEST_FAIL_MESSAGE("compiler error");
    }

    Bytecode code = bytecode(compiler);

    test_instructions(test.expected_instructions_len,
                      test.expected_instructions, code.instructions);

    test_constants(test.expected_constants_len, test.expected_constants,
                   code.constants);

    free_program(program);
    free_compiler(compiler);
  }
}

void test_integer_arithmetic(void) {
  int first_args[] = {1};
  Instruction first_instruction = make_instruction(OP_CONSTANT, first_args, 1);

  int second_args[] = {2};
  Instruction second_instruction =
      make_instruction(OP_CONSTANT, second_args, 1);

  compilerTestCase tests[] = {
      {
          .input = "1 + 2",
          .expected_constants_len = 2,
          .expected_constants = {1, 2},
          .expected_instructions_len = 2,
          .expected_instructions =
              {
                  first_instruction,
                  second_instruction,
              },
      },
  };

  run_compiler_tests(tests, 1);

  int_array_free(&first_instruction);
  int_array_free(&second_instruction);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_integer_arithmetic);
  return UNITY_END();
}
