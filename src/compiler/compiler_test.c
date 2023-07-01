#include "../evaluator/evaluator.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "compiler.h"
#include <stdint.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

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
    int_array_free(&expected[i]);
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
  compilerTestCase
      tests[] =
          {
              {
                  .input = "1 + 2",
                  .expected_constants_len = 2,
                  .expected_constants = {1, 2},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_ADD, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "1 - 2",
                  .expected_constants_len = 2,
                  .expected_constants = {1, 2},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_SUB, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "1 * 2",
                  .expected_constants_len = 2,
                  .expected_constants = {1, 2},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_MUL, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "2 / 1",
                  .expected_constants_len = 2,
                  .expected_constants = {2, 1},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_DIV, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "2 << 1",
                  .expected_constants_len = 2,
                  .expected_constants = {2, 1},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_LSHIFT, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "2 >> 1",
                  .expected_constants_len = 2,
                  .expected_constants = {2, 1},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_RSHIFT, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "2 % 1",
                  .expected_constants_len = 2,
                  .expected_constants = {2, 1},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_MOD, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "2 | 1",
                  .expected_constants_len = 2,
                  .expected_constants = {2, 1},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_BIT_OR, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "2 & 1",
                  .expected_constants_len = 2,
                  .expected_constants = {2, 1},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_BIT_AND, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "2 ^ 1",
                  .expected_constants_len = 2,
                  .expected_constants = {2, 1},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_BIT_XOR, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
          };

  run_compiler_tests(tests, ARRAY_LEN(tests));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_integer_arithmetic);
  return UNITY_END();
}
