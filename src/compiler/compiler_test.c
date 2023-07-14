#include "../evaluator/evaluator.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "compiler.h"
#include <stdio.h>
#include <stdint.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define RUN_COMPILER_TESTS(tests, type)                                        \
  run_compiler_tests(tests, sizeof(tests[0]), ARRAY_LEN(tests), type)

typedef enum {
  COMPILER_TEST_INT,
  COMPILER_TEST_STRING,
} CompilerTestType;

typedef struct {
  char *input;
  size_t expected_constants_len;
  size_t expected_constants[100];
  size_t expected_instructions_len;
  Instruction expected_instructions[100];
} compilerIntTestCase;

typedef struct {
  char *input;
  size_t expected_constants_len;
  char *expected_constants[100];
  size_t expected_instructions_len;
  Instruction expected_instructions[100];
} compilerStringTestCase;

Program *parse(void *test, CompilerTestType type) {
  char *input;
  switch (type) {
  case COMPILER_TEST_INT:
    input = ((compilerIntTestCase *)test)->input;
    break;
  case COMPILER_TEST_STRING:
    input = ((compilerStringTestCase *)test)->input;
    break;
  }

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

void print_instructions(const Instructions *concatted,
                        const Instructions *actual) {
  ResizableBuffer expected_buf;
  init_resizable_buffer(&expected_buf, 50);
  instructions_to_string(&expected_buf, concatted);
  printf("EXPECTED:\n%s\n--------------------\n", expected_buf.buf);
  ResizableBuffer actual_buf;
  init_resizable_buffer(&actual_buf, 50);
  instructions_to_string(&actual_buf, actual);
  printf("ACTUAL:\n%s\n----------------------\n", actual_buf.buf);

  free(expected_buf.buf);
  free(actual_buf.buf);
}

void test_instructions(size_t instructions_count, Instruction expected[],
                       Instructions actual) {
  Instructions concatted = concat_instructions(instructions_count, expected);
  if (concatted.len != actual.len) {
    print_instructions(&concatted, &actual);
    char msg[100];
    sprintf(msg, "expected instructions length %ld, got %ld", concatted.len,
            actual.len);
    TEST_FAIL_MESSAGE(msg);
  }

  for (uint32_t i = 0; i < instructions_count; i++) {
    if (concatted.arr[i] != actual.arr[i]) {
      print_instructions(&concatted, &actual);
      char msg[100];
      sprintf(msg, "wrong instruction at %d", i);
      TEST_FAIL_MESSAGE(msg);
    }

    int_array_free(&expected[i]);
  }

  int_array_free(&concatted);
}

void test_integer_object(int expected, Object *actual) {
  TEST_ASSERT_EQUAL(NUMBER_OBJ, actual->type);
  TEST_ASSERT_EQUAL(expected, ((Number *)actual)->value);
}

void test_integer_constants(size_t constants_count, size_t expected[],
                            DynamicArray actual) {
  TEST_ASSERT_EQUAL(constants_count, actual.len);

  for (uint32_t i = 0; i < constants_count; i++) {
    test_integer_object(expected[i], actual.arr[i]);
  }
}

void test_string_constants(size_t constants_count, char *expected[],
                           DynamicArray actual) {
  TEST_ASSERT_EQUAL(constants_count, actual.len);

  for (uint32_t i = 0; i < constants_count; i++) {
    TEST_ASSERT_EQUAL(STRING_OBJ, ((String *)actual.arr[i])->type);
    TEST_ASSERT_EQUAL_STRING(expected[i], ((String *)actual.arr[i])->value);
  }
}

void run_compiler_tests(void *tests, size_t test_size, size_t test_count,
                        CompilerTestType type) {
  for (uint32_t i = 0; i < test_count; i++) {
    void *test = tests + (i * test_size);

    Program *program = parse(test, type);
    Compiler *compiler = new_compiler();
    int8_t result = compile_program(compiler, program);
    if (result != COMPILER_OK) {
      char msg[100];
      compiler_error(result, msg, 100);
      TEST_FAIL_MESSAGE(msg);
    }

    Bytecode code = bytecode(compiler);

    switch (type) {
    case COMPILER_TEST_INT: {
      compilerIntTestCase *int_test = (compilerIntTestCase *)test;
      test_instructions(int_test->expected_instructions_len,
                        int_test->expected_instructions, code.instructions);

      test_integer_constants(int_test->expected_constants_len,
                             int_test->expected_constants, code.constants);
      break;
    }
    case COMPILER_TEST_STRING: {
      compilerStringTestCase *str_test = (compilerStringTestCase *)test;
      test_instructions(str_test->expected_instructions_len,
                        str_test->expected_instructions, code.instructions);

      test_string_constants(str_test->expected_constants_len,
                            str_test->expected_constants, code.constants);
      break;
    }
    }

    free_program(program);
    free_compiler(compiler);
  }
}

void test_integer_arithmetic(void) {
  compilerIntTestCase
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
              {
                  .input = "-1",
                  .expected_constants_len = 1,
                  .expected_constants = {1},
                  .expected_instructions_len = 3,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_MINUS, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
          };

  RUN_COMPILER_TESTS(tests, COMPILER_TEST_INT);
}

void test_boolean_expressions(void) {
  compilerIntTestCase
      tests[] =
          {
              {
                  .input = "true",
                  .expected_constants_len = 0,
                  .expected_instructions_len = 2,
                  .expected_instructions =
                      {
                          make_instruction(OP_TRUE, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "false",
                  .expected_constants_len = 0,
                  .expected_instructions_len = 2,
                  .expected_instructions =
                      {
                          make_instruction(OP_FALSE, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "1 > 2",
                  .expected_constants_len = 2,
                  .expected_constants = {1, 2},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_GREATER, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "1 < 2",
                  .expected_constants_len = 2,
                  .expected_constants = {2, 1},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_GREATER, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "1 == 2",
                  .expected_constants_len = 2,
                  .expected_constants = {1, 2},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_EQ, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "1 != 2",
                  .expected_constants_len = 2,
                  .expected_constants = {1, 2},
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){1}, 1),
                          make_instruction(OP_NOT_EQ, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "true == false",
                  .expected_constants_len = 0,
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_TRUE, (int[]){}, 0),
                          make_instruction(OP_FALSE, (int[]){}, 0),
                          make_instruction(OP_EQ, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "true != false",
                  .expected_constants_len = 0,
                  .expected_instructions_len = 4,
                  .expected_instructions =
                      {
                          make_instruction(OP_TRUE, (int[]){}, 0),
                          make_instruction(OP_FALSE, (int[]){}, 0),
                          make_instruction(OP_NOT_EQ, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
              {
                  .input = "!true",
                  .expected_constants_len = 0,
                  .expected_instructions_len = 3,
                  .expected_instructions =
                      {
                          make_instruction(OP_TRUE, (int[]){}, 0),
                          make_instruction(OP_BANG, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
          };

  RUN_COMPILER_TESTS(tests, COMPILER_TEST_INT);
}

void test_conditionals(void) {
  compilerIntTestCase tests[] = {
      {
          .input = "if (true) { 10 }; 3333;",
          .expected_constants_len = 2,
          .expected_constants = {10, 3333},
          .expected_instructions_len = 8,
          .expected_instructions =
              {
                  // 0000
                  make_instruction(OP_TRUE, (int[]){}, 0),
                  // 0001
                  make_instruction(OP_JMP_IF_FALSE, (int[]){10}, 1),
                  // 0004
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  // 0007
                  make_instruction(OP_JMP, (int[]){11}, 1),
                  // 0010
                  make_instruction(OP_NULL, (int[]){}, 0),
                  // 0011
                  make_instruction(OP_POP, (int[]){}, 0),
                  // 0012
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  // 0015
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "if (true) { 10 } else { 20 }; 3333;",
          .expected_constants_len = 3,
          .expected_constants = {10, 20, 3333},
          .expected_instructions_len = 8,
          .expected_instructions =
              {
                  // 0000
                  make_instruction(OP_TRUE, (int[]){}, 0),
                  // 0001
                  make_instruction(OP_JMP_IF_FALSE, (int[]){10}, 1),
                  // 0004
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  // 0007
                  make_instruction(OP_JMP, (int[]){13}, 1),
                  // 0010
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  // 0013
                  make_instruction(OP_POP, (int[]){}, 0),
                  // 0014
                  make_instruction(OP_CONSTANT, (int[]){2}, 1),
                  // 0017
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
  };

  RUN_COMPILER_TESTS(tests, COMPILER_TEST_INT);
}

void test_global_let_statements(void) {
  compilerIntTestCase tests[] = {
      {
          .input = "let one = 1; let two = 2;",
          .expected_constants_len = 2,
          .expected_constants = {1, 2},
          .expected_instructions_len = 4,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){1}, 1),
              },
      },
      {
          .input = "let one = 1; one;",
          .expected_constants_len = 1,
          .expected_constants = {1},
          .expected_instructions_len = 4,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "let one = 1; let two = one; two;",
          .expected_constants_len = 1,
          .expected_constants = {1},
          .expected_instructions_len = 6,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){1}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){1}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
  };

  RUN_COMPILER_TESTS(tests, COMPILER_TEST_INT);
}

void test_string_expressions(void) {
  compilerStringTestCase tests[] = {
      {
          .input = "\"monkey\"",
          .expected_constants_len = 1,
          .expected_constants = {"monkey"},
          .expected_instructions_len = 2,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
        .input = "\"mon\" + \"key\"",
        .expected_constants_len = 2,
        .expected_constants = {"mon", "key"},
        .expected_instructions_len = 4,
        .expected_instructions =
            {
                make_instruction(OP_CONSTANT, (int[]){0}, 1),
                make_instruction(OP_CONSTANT, (int[]){1}, 1),
                make_instruction(OP_ADD, (int[]){}, 0),
                make_instruction(OP_POP, (int[]){}, 0),
            },
      },
  };

  RUN_COMPILER_TESTS(tests, COMPILER_TEST_STRING);
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
