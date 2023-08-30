#include "../evaluator/evaluator.h"
#include "../parser/parser.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include "compiler.h"
#include "symbol_table.h"
#include <stdint.h>
#include <stdio.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define RUN_COMPILER_TESTS(tests) run_compiler_tests(tests, ARRAY_LEN(tests))

typedef struct {
  char *input;
  size_t expected_constants_len;
  Object *expected_constants[100];
  size_t expected_instructions_len;
  Instruction expected_instructions[100];
} compilerTestCase;

Program *parse(compilerTestCase *test) {
  Lexer *l = new_lexer(test->input);
  Parser *p = new_parser(l);
  Program *program = parse_program(p);
  free_parser(p);
  return program;
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
  printf("ACTUAL:\n%s\n--------------------\n", actual_buf.buf);

  free(expected_buf.buf);
  free(actual_buf.buf);
}

void test_instructions(Instructions *expected, Instructions *actual) {
  for (uint32_t i = 0; i < expected->len; i++) {
    if (expected->arr[i] != actual->arr[i]) {
      print_instructions(expected, actual);
      char msg[100];
      sprintf(msg, "wrong instruction at %d", i);
      TEST_FAIL_MESSAGE(msg);
    }
  }
}
void concat_and_test_instructions(size_t instructions_count,
                                  Instruction expected[], Instructions actual) {
  Instructions concatted = concat_instructions(instructions_count, expected);
  if (concatted.len != actual.len) {
    print_instructions(&concatted, &actual);
    char msg[100];
    sprintf(msg, "expected instructions length %ld, got %ld", concatted.len,
            actual.len);
    TEST_FAIL_MESSAGE(msg);
  }

  test_instructions(&concatted, &actual);
  int_array_free(&concatted);
}

void test_number_object(Object *expected, Object *actual) {
  TEST_ASSERT_EQUAL(NUMBER_OBJ, actual->type);
  TEST_ASSERT_EQUAL(((Number *)expected)->value, ((Number *)actual)->value);
}

void test_string_object(Object *expected, Object *actual) {
  TEST_ASSERT_EQUAL(STRING_OBJ, actual->type);
  TEST_ASSERT_EQUAL_STRING(((String *)expected)->value,
                           ((String *)actual)->value);
}

void test_compiled_function(Object *expected, Object *actual) {
  CompiledFunction *expected_fn = (CompiledFunction *)expected;
  CompiledFunction *actual_fn = (CompiledFunction *)actual;

  test_instructions(&expected_fn->instructions, &actual_fn->instructions);
  int_array_free(&expected_fn->instructions);
}

void test_compiled_loop(Object *expected, Object *actual) {
  CompiledLoop *expected_loop = (CompiledLoop *)expected;
  CompiledLoop *actual_loop = (CompiledLoop *)actual;

  test_instructions(&expected_loop->instructions, &actual_loop->instructions);
  TEST_ASSERT_EQUAL(expected_loop->num_locals, actual_loop->num_locals);
  int_array_free(&expected_loop->instructions);
}

void test_constants(compilerTestCase test, Bytecode code) {
  TEST_ASSERT_EQUAL(test.expected_constants_len, code.constants.len);

  for (size_t i = 0; i < code.constants.len; i++) {
    Object *constant = code.constants.arr[i];
    switch (constant->type) {
    case NUMBER_OBJ:
      test_number_object(test.expected_constants[i], constant);
      break;
    case STRING_OBJ:
      test_string_object(test.expected_constants[i], constant);
      break;
    case COMPILED_FUNCTION_OBJ:
      test_compiled_function(test.expected_constants[i], constant);
      break;
    case COMPILED_LOOP_OBJ:
      test_compiled_loop(test.expected_constants[i], constant);
      break;
    default:
      TEST_FAIL_MESSAGE("unreachable");
    }

    free(test.expected_constants[i]);
  }
}

void run_compiler_tests(compilerTestCase *test_cases, size_t test_count) {
  for (uint32_t i = 0; i < test_count; i++) {
    compilerTestCase test = test_cases[i];

    Program *program = parse(&test);
    Compiler *compiler = new_compiler();
    int8_t result = compile_program(compiler, program);
    if (result != COMPILER_OK) {
      char msg[100];
      compiler_error(result, msg, 100);
      TEST_FAIL_MESSAGE(msg);
    }

    Bytecode code = bytecode(compiler);

    concat_and_test_instructions(test.expected_instructions_len,
                                 test.expected_instructions, code.instructions);

    test_constants(test, code);

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
                  .expected_constants =
                      {
                          new_number(1),
                          new_number(2),
                      },
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
                  .expected_constants =
                      {
                          new_number(1),
                          new_number(2),
                      },
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
                  .expected_constants =
                      {
                          new_number(1),
                          new_number(2),
                      },
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
                  .expected_constants =
                      {
                          new_number(2),
                          new_number(1),
                      },
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
                  .expected_constants =
                      {
                          new_number(2),
                          new_number(1),
                      },
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
                  .expected_constants =
                      {
                          new_number(2),
                          new_number(1),
                      },
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
                  .expected_constants =
                      {
                          new_number(2),
                          new_number(1),
                      },
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
                  .expected_constants =
                      {
                          new_number(2),
                          new_number(1),
                      },
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
                  .expected_constants =
                      {
                          new_number(2),
                          new_number(1),
                      },
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
                  .expected_constants =
                      {
                          new_number(2),
                          new_number(1),
                      },
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
                  .expected_constants =
                      {
                          new_number(1),
                      },
                  .expected_instructions_len = 3,
                  .expected_instructions =
                      {
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_MINUS, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                      },
              },
          };

  RUN_COMPILER_TESTS(tests);
}

void test_boolean_expressions(void) {
  compilerTestCase
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
                  .expected_constants =
                      {
                          new_number(1),
                          new_number(2),
                      },
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
                  .expected_constants =
                      {
                          new_number(2),
                          new_number(1),
                      },
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
                  .expected_constants =
                      {
                          new_number(1),
                          new_number(2),
                      },
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
                  .expected_constants =
                      {
                          new_number(1),
                          new_number(2),
                      },
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

  RUN_COMPILER_TESTS(tests);
}

void test_conditionals(void) {
  compilerTestCase tests[] = {
      {
          .input = "if (true) { 10 }; 3333;",
          .expected_constants_len = 2,
          .expected_constants =
              {
                  new_number(10),
                  new_number(3333),
              },
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
          .expected_constants =
              {
                  new_number(10),
                  new_number(20),
                  new_number(3333),
              },
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

  RUN_COMPILER_TESTS(tests);
}

void test_global_let_statements(void) {
  compilerTestCase tests[] = {
      {
          .input = "let one = 1; let two = 2;",
          .expected_constants_len = 2,
          .expected_constants =
              {
                  new_number(1),
                  new_number(2),
              },
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
          .expected_constants =
              {
                  new_number(1),
              },
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
          .expected_constants =
              {
                  new_number(1),
              },
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

  RUN_COMPILER_TESTS(tests);
}

void test_string_expressions(void) {
  compilerTestCase tests[] = {
      {
          .input = "\"monkey\"",
          .expected_constants_len = 1,
          .expected_constants =
              {
                  new_string("monkey"),
              },
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
          .expected_constants =
              {
                  new_string("mon"),
                  new_string("key"),
              },
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

  RUN_COMPILER_TESTS(tests);
}

void test_array_literals(void) {
  compilerTestCase tests[] = {
      {
          .input = "[]",
          .expected_constants_len = 0,
          .expected_constants = {},
          .expected_instructions_len = 2,
          .expected_instructions =
              {
                  make_instruction(OP_ARRAY, (int[]){0}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "[1, 2, 3]",
          .expected_constants_len = 3,
          .expected_constants =
              {
                  new_number(1),
                  new_number(2),
                  new_number(3),
              },
          .expected_instructions_len = 5,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_CONSTANT, (int[]){2}, 1),
                  make_instruction(OP_ARRAY, (int[]){3}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "[1 + 2, 3 - 4, 5 * 6]",
          .expected_constants_len = 6,
          .expected_constants =
              {
                  new_number(1),
                  new_number(2),
                  new_number(3),
                  new_number(4),
                  new_number(5),
                  new_number(6),
              },
          .expected_instructions_len = 11,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_ADD, (int[]){}, 0),
                  make_instruction(OP_CONSTANT, (int[]){2}, 1),
                  make_instruction(OP_CONSTANT, (int[]){3}, 1),
                  make_instruction(OP_SUB, (int[]){}, 0),
                  make_instruction(OP_CONSTANT, (int[]){4}, 1),
                  make_instruction(OP_CONSTANT, (int[]){5}, 1),
                  make_instruction(OP_MUL, (int[]){}, 0),
                  make_instruction(OP_ARRAY, (int[]){3}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
  };

  RUN_COMPILER_TESTS(tests);
}

void test_hash_literals(void) {
  compilerTestCase tests[] = {
      {
          .input = "{}",
          .expected_constants_len = 0,
          .expected_instructions_len = 2,
          .expected_instructions =
              {
                  make_instruction(OP_HASH, (int[]){0}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "{1: 2, 3: 4, 5: 6}",
          .expected_constants_len = 6,
          .expected_constants =
              {
                  new_number(1),
                  new_number(2),
                  new_number(3),
                  new_number(4),
                  new_number(5),
                  new_number(6),
              },
          .expected_instructions_len = 8,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_CONSTANT, (int[]){2}, 1),
                  make_instruction(OP_CONSTANT, (int[]){3}, 1),
                  make_instruction(OP_CONSTANT, (int[]){4}, 1),
                  make_instruction(OP_CONSTANT, (int[]){5}, 1),
                  make_instruction(OP_HASH, (int[]){6}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
  };

  RUN_COMPILER_TESTS(tests);
}

void test_index_expressions(void) {
  compilerTestCase tests[] = {
      {
          .input = "[1, 2, 3][1 + 1]",
          .expected_constants_len = 5,
          .expected_constants =
              {
                  new_number(1),
                  new_number(2),
                  new_number(3),
                  new_number(1),
                  new_number(1),
              },
          .expected_instructions_len = 9,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_CONSTANT, (int[]){2}, 1),
                  make_instruction(OP_ARRAY, (int[]){3}, 1),
                  make_instruction(OP_CONSTANT, (int[]){3}, 1),
                  make_instruction(OP_CONSTANT, (int[]){4}, 1),
                  make_instruction(OP_ADD, (int[]){}, 0),
                  make_instruction(OP_INDEX, (int[]){}, 0),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "{1: 2}[2 - 1]",
          .expected_constants_len = 4,
          .expected_constants =
              {
                  new_number(1),
                  new_number(2),
                  new_number(2),
                  new_number(1),
              },
          .expected_instructions_len = 8,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_HASH, (int[]){2}, 1),
                  make_instruction(OP_CONSTANT, (int[]){2}, 1),
                  make_instruction(OP_CONSTANT, (int[]){3}, 1),
                  make_instruction(OP_SUB, (int[]){}, 0),
                  make_instruction(OP_INDEX, (int[]){}, 0),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
  };

  RUN_COMPILER_TESTS(tests);
}

void test_functions(void) {
  compilerTestCase tests[] =
      {
          {
              .input = "fn() { return 5 + 10 }",
              .expected_constants_len = 3,
              .expected_constants =
                  {
                      new_number(5),
                      new_number(10),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){0}, 1),
                              make_instruction(OP_CONSTANT, (int[]){1}, 1),
                              make_instruction(OP_ADD, (int[]){}, 0),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          4),
                  },
              .expected_instructions_len = 2,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){2, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "fn() { 5 + 10 }",
              .expected_constants_len = 3,
              .expected_constants =
                  {
                      new_number(5),
                      new_number(10),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){0}, 1),
                              make_instruction(OP_CONSTANT, (int[]){1}, 1),
                              make_instruction(OP_ADD, (int[]){}, 0),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          4),
                  },
              .expected_instructions_len = 2,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){2, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "fn() { 1; 2 }",
              .expected_constants_len = 3,
              .expected_constants =
                  {
                      new_number(1),
                      new_number(2),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){0}, 1),
                              make_instruction(OP_POP, (int[]){}, 0),
                              make_instruction(OP_CONSTANT, (int[]){1}, 1),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          4),
                  },
              .expected_instructions_len = 2,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){2, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "fn() { }",
              .expected_constants_len = 1,
              .expected_constants =
                  {
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_RETURN, (int[]){}, 0),
                          },
                          1),
                  },
              .expected_instructions_len = 2,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){0, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
      };

  RUN_COMPILER_TESTS(tests);
}

size_t emit(Compiler *, OpCode, int *, size_t);

void test_compiler_scopes(void) {
  Compiler *compiler = new_compiler();
  TEST_ASSERT_EQUAL(0, compiler->scope_index);

  SymbolTable *global_symbol_table = compiler->symbol_table;

  emit(compiler, OP_MUL, (int[]){}, 0);

  enter_compiler_scope(compiler);
  TEST_ASSERT_EQUAL(1, compiler->scope_index);

  emit(compiler, OP_SUB, (int[]){}, 0);

  TEST_ASSERT_EQUAL(1,
                    compiler->scopes[compiler->scope_index].instructions.len);

  EmmittedInstruction last_instruction =
      compiler->scopes[compiler->scope_index].last_instruction;

  TEST_ASSERT_EQUAL(OP_SUB, last_instruction.op);

  TEST_ASSERT_EQUAL_PTR(global_symbol_table, compiler->symbol_table->outer);

  leave_compiler_scope(compiler);

  TEST_ASSERT_EQUAL(0, compiler->scope_index);

  TEST_ASSERT_EQUAL_PTR(global_symbol_table, compiler->symbol_table);

  TEST_ASSERT_NULL(compiler->symbol_table->outer);

  emit(compiler, OP_ADD, (int[]){}, 0);

  TEST_ASSERT_EQUAL(2,
                    compiler->scopes[compiler->scope_index].instructions.len);

  last_instruction = compiler->scopes[compiler->scope_index].last_instruction;
  TEST_ASSERT_EQUAL(OP_ADD, last_instruction.op);

  EmmittedInstruction previous_instructions =
      compiler->scopes[compiler->scope_index].previous_instruction;

  TEST_ASSERT_EQUAL(OP_MUL, previous_instructions.op);

  free_compiler(compiler);
}

void test_function_calls(void) {
  compilerTestCase tests[] =
      {
          {
              .input = "fn() { 24 }();",
              .expected_constants_len = 2,
              .expected_constants =
                  {
                      new_number(24),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){0}, 1),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          2),
                  },
              .expected_instructions_len = 4,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){1, 0}, 2),
                      make_instruction(OP_CALL, (int[]){0}, 1),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "let noArg = fn() { 24 }; noArg();",
              .expected_constants_len = 2,
              .expected_constants =
                  {
                      new_number(24),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){0}, 1),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          2),
                  },
              .expected_instructions_len = 5,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){1, 0}, 2),
                      make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                      make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                      make_instruction(OP_CALL, (int[]){0}, 1),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "let oneArg = fn(a) { a }; oneArg(24);",
              .expected_constants_len = 2,
              .expected_constants =
                  {
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          2),
                      new_number(24),
                  },
              .expected_instructions_len = 6,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){0, 0}, 2),
                      make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                      make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                      make_instruction(OP_CONSTANT, (int[]){1}, 1),
                      make_instruction(OP_CALL, (int[]){1}, 1),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "let manyArg = fn(a, b, c) { a; b; c; };"
                       "manyArg(24, 25, 26);",
              .expected_constants_len = 4,
              .expected_constants =
                  {
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_POP, (int[]){}, 0),
                              make_instruction(OP_GET_LOCAL, (int[]){1}, 1),
                              make_instruction(OP_POP, (int[]){}, 0),
                              make_instruction(OP_GET_LOCAL, (int[]){2}, 1),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          6),
                      new_number(24),
                      new_number(25),
                      new_number(26),
                  },
              .expected_instructions_len = 8,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){0, 0}, 2),
                      make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                      make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                      make_instruction(OP_CONSTANT, (int[]){1}, 1),
                      make_instruction(OP_CONSTANT, (int[]){2}, 1),
                      make_instruction(OP_CONSTANT, (int[]){3}, 1),
                      make_instruction(OP_CALL, (int[]){3}, 1),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
      };

  RUN_COMPILER_TESTS(tests);
}

void test_let_statement_scopes(void) {
  compilerTestCase tests[] =
      {
          {
              .input = "let num = 55; fn() { num }",
              .expected_constants_len = 2,
              .expected_constants =
                  {
                      new_number(55),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          2),
                  },
              .expected_instructions_len = 4,
              .expected_instructions =
                  {
                      make_instruction(OP_CONSTANT, (int[]){0}, 1),
                      make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                      make_instruction(OP_CLOSURE, (int[]){1, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "fn() { let num = 55; num }",
              .expected_constants_len = 2,
              .expected_constants =
                  {
                      new_number(55),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){0}, 1),
                              make_instruction(OP_SET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          4),
                  },
              .expected_instructions_len = 2,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){1, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "fn() { let a = 55; let b = 77; a + b }",
              .expected_constants_len = 3,
              .expected_constants =
                  {
                      new_number(55),
                      new_number(77),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){0}, 1),
                              make_instruction(OP_SET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_CONSTANT, (int[]){1}, 1),
                              make_instruction(OP_SET_LOCAL, (int[]){1}, 1),
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_GET_LOCAL, (int[]){1}, 1),
                              make_instruction(OP_ADD, (int[]){}, 0),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          8),
                  },
              .expected_instructions_len = 2,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){2, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
      };

  RUN_COMPILER_TESTS(tests);
}

void test_builtins(void) {
  compilerTestCase tests[] = {
      {
          .input = "len([]); push([], 1);",
          .expected_constants_len = 1,
          .expected_constants =
              {
                  new_number(1),
              },
          .expected_instructions_len = 9,
          .expected_instructions =
              {
                  make_instruction(OP_GET_BUILTIN, (int[]){0}, 1),
                  make_instruction(OP_ARRAY, (int[]){0}, 1),
                  make_instruction(OP_CALL, (int[]){1}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
                  make_instruction(OP_GET_BUILTIN, (int[]){4}, 1),
                  make_instruction(OP_ARRAY, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_CALL, (int[]){2}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "fn() { len([]); }",
          .expected_constants_len = 1,
          .expected_constants =
              {
                  new_concatted_compiled_function(
                      (Instruction[]){
                          make_instruction(OP_GET_BUILTIN, (int[]){0}, 1),
                          make_instruction(OP_ARRAY, (int[]){0}, 1),
                          make_instruction(OP_CALL, (int[]){1}, 1),
                          make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                      },
                      4),
              },
          .expected_instructions_len = 2,
          .expected_instructions =
              {
                  make_instruction(OP_CLOSURE, (int[]){0, 0}, 2),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
  };

  RUN_COMPILER_TESTS(tests);
}

void test_closures(void) {
  compilerTestCase tests[] =
      {
          {
              .input = "fn (a) {"
                       "  fn(b) {"
                       "    a + b;"
                       "  }"
                       "}",
              .expected_constants_len = 2,
              .expected_constants =
                  {
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_GET_FREE, (int[]){0}, 1),
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_ADD, (int[]){}, 0),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          4),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_CLOSURE, (int[]){0, 1}, 2),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          3),
                  },
              .expected_instructions_len = 2,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){1, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "fn(a) {"
                       "  fn(b) {"
                       "    fn(c) {"
                       "      a + b + c;"
                       "    }"
                       "  }"
                       "}",
              .expected_constants_len = 3,
              .expected_constants =
                  {
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_GET_FREE, (int[]){0}, 1),
                              make_instruction(OP_GET_FREE, (int[]){1}, 1),
                              make_instruction(OP_ADD, (int[]){}, 0),
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_ADD, (int[]){}, 0),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          6),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_GET_FREE, (int[]){0}, 1),
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_CLOSURE, (int[]){0, 2}, 2),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          4),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_CLOSURE, (int[]){1, 1}, 2),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          3),
                  },
              .expected_instructions_len = 2,
              .expected_instructions =
                  {
                      make_instruction(OP_CLOSURE, (int[]){2, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
          {
              .input = "let global = 55;"
                       "fn() {"
                       "  let a = 66;"
                       "  fn() {"
                       "    let b = 77;"
                       "    fn() {"
                       "      let c = 88;"
                       "      global + a + b + c;"
                       "    }"
                       "  }"
                       "}",
              .expected_constants_len = 7,
              .expected_constants =
                  {
                      new_number(55),
                      new_number(66),
                      new_number(77),
                      new_number(88),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){3}, 1),
                              make_instruction(OP_SET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                              make_instruction(OP_GET_FREE, (int[]){0}, 1),
                              make_instruction(OP_ADD, (int[]){}, 0),
                              make_instruction(OP_GET_FREE, (int[]){1}, 1),
                              make_instruction(OP_ADD, (int[]){}, 0),
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_ADD, (int[]){}, 0),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          10),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){2}, 1),
                              make_instruction(OP_SET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_GET_FREE, (int[]){0}, 1),
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_CLOSURE, (int[]){4, 2}, 2),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          6),
                      new_concatted_compiled_function(
                          (Instruction[]){
                              make_instruction(OP_CONSTANT, (int[]){1}, 1),
                              make_instruction(OP_SET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                              make_instruction(OP_CLOSURE, (int[]){5, 1}, 2),
                              make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                          },
                          5),
                  },
              .expected_instructions_len = 4,
              .expected_instructions =
                  {
                      make_instruction(OP_CONSTANT, (int[]){0}, 1),
                      make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                      make_instruction(OP_CLOSURE, (int[]){6, 0}, 2),
                      make_instruction(OP_POP, (int[]){}, 0),
                  },
          },
      };

  RUN_COMPILER_TESTS(tests);
}

void test_recursive_functions(void) {
  compilerTestCase tests[] = {
      {
          .input = "let countDown = fn(x) { countDown(x - 1); };"
                   "countDown(1);",
          .expected_constants_len = 3,
          .expected_constants =
              {
                  new_number(1),
                  new_concatted_compiled_function(
                      (Instruction[]){
                          make_instruction(OP_CURRENT_CLOSURE, (int[]){}, 0),
                          make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_SUB, (int[]){}, 0),
                          make_instruction(OP_CALL, (int[]){1}, 1),
                          make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                      },
                      6),
                  new_number(1),
              },
          .expected_instructions_len = 6,
          .expected_instructions =
              {
                  make_instruction(OP_CLOSURE, (int[]){1, 0}, 2),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){2}, 1),
                  make_instruction(OP_CALL, (int[]){1}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "let wrapper = fn() {"
                   "  let countDown = fn(x) { countDown(x - 1); };"
                   "  countDown(1);"
                   "};"
                   "wrapper();",
          .expected_constants_len = 4,
          .expected_constants =
              {
                  new_number(1),
                  new_concatted_compiled_function(
                      (Instruction[]){
                          make_instruction(OP_CURRENT_CLOSURE, (int[]){}, 0),
                          make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){0}, 1),
                          make_instruction(OP_SUB, (int[]){}, 0),
                          make_instruction(OP_CALL, (int[]){1}, 1),
                          make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                      },
                      6),
                  new_number(1),
                  new_concatted_compiled_function(
                      (Instruction[]){
                          make_instruction(OP_CLOSURE, (int[]){1, 0}, 2),
                          make_instruction(OP_SET_LOCAL, (int[]){0}, 1),
                          make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){2}, 1),
                          make_instruction(OP_CALL, (int[]){1}, 1),
                          make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                      },
                      6),
              },
          .expected_instructions_len = 5,
          .expected_instructions =
              {
                  make_instruction(OP_CLOSURE, (int[]){3, 0}, 2),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CALL, (int[]){0}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
  };

  RUN_COMPILER_TESTS(tests);
}

void test_reassignment(void) {
  compilerTestCase tests[] = {
      {
          .input = "let a = 1; a = 2;",
          .expected_constants_len = 2,
          .expected_constants =
              {
                  new_number(1),
                  new_number(2),
              },
          .expected_instructions_len = 6,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "let a = 1;"
                   "let b = fn(x) { a = x; };"
                   "b(2);",
          .expected_constants_len = 3,
          .expected_constants =
              {
                  new_number(1),
                  new_concatted_compiled_function(
                      (Instruction[]){
                          make_instruction(OP_GET_LOCAL, (int[]){0}, 1),
                          make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                          make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                          make_instruction(OP_RETURN_VALUE, (int[]){}, 0),
                      },
                      4),
                  new_number(2),
              },
          .expected_instructions_len = 8,
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CLOSURE, (int[]){1, 0}, 2),
                  make_instruction(OP_SET_GLOBAL, (int[]){1}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){1}, 1),
                  make_instruction(OP_CONSTANT, (int[]){2}, 1),
                  make_instruction(OP_CALL, (int[]){1}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
      },
      {
          .input = "let a = 1; a = a + 1;",
          .expected_constants_len = 2,
          .expected_constants =
              {
                  new_number(1),
                  new_number(1),
              },
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_ADD, (int[]){}, 0),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
          .expected_instructions_len = 8,
      },
  };

  RUN_COMPILER_TESTS(tests);
}

void test_while_loops(void) {
  compilerTestCase tests[] = {
      {
          .input = "let a = 0; while (a < 10) { a = a + 1; }; a;",
          .expected_constants_len = 4,
          .expected_constants =
              {
                  new_number(0),
                  new_number(10),
                  new_number(1),
                  new_concatted_compiled_loop(
                      (Instruction[]){
                          make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){2}, 1),
                          make_instruction(OP_ADD, (int[]){}, 0),
                          make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                          make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                          make_instruction(OP_POP, (int[]){}, 0),
                          make_instruction(OP_CONTINUE, (int[]){}, 0),
                      },
                      7, 0),
              },
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GREATER, (int[]){}, 0),
                  make_instruction(OP_JMP_IF_FALSE, (int[]){24}, 1),
                  make_instruction(OP_CLOSURE, (int[]){3, 0}, 2),
                  make_instruction(OP_LOOP, (int[]){}, 0),
                  make_instruction(OP_JMP, (int[]){6}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
              },
          .expected_instructions_len = 15,
      },
  };

  RUN_COMPILER_TESTS(tests);
}

void test_loop_control_statements(void) {
  compilerTestCase tests[] = {
      {
          .input = "let a = 0; while (a < 10) { continue; };",
          .expected_constants_len = 3,
          .expected_constants =
              {
                  new_number(0),
                  new_number(10),
                  new_concatted_compiled_loop(
                      (Instruction[]){
                          make_instruction(OP_CONTINUE, (int[]){}, 0),
                      },
                      1, 0),
              },
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GREATER, (int[]){}, 0),
                  make_instruction(OP_JMP_IF_FALSE, (int[]){24}, 1),
                  make_instruction(OP_CLOSURE, (int[]){2, 0}, 2),
                  make_instruction(OP_LOOP, (int[]){}, 0),
                  make_instruction(OP_JMP, (int[]){6}, 1),
              },
          .expected_instructions_len = 13,
      },
      {
          .input = "let a = 0; while (a < 10) { if (a == 5) { continue; }; };",
          .expected_constants_len = 4,
          .expected_constants =
              {
                  new_number(0),
                  new_number(10),
                  new_number(5),
                  new_concatted_compiled_loop(
                      (Instruction[]){
                          make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                          make_instruction(OP_CONSTANT, (int[]){2}, 1),
                          make_instruction(OP_EQ, (int[]){}, 0),
                          make_instruction(OP_JMP_IF_FALSE, (int[]){14}, 1),
                          make_instruction(OP_CONTINUE, (int[]){}, 0),
                          make_instruction(OP_JMP, (int[]){15}, 1),
                          make_instruction(OP_NULL, (int[]){}, 0),
                          make_instruction(OP_POP, (int[]){}, 0),
                          make_instruction(OP_CONTINUE, (int[]){}, 0),
                      },
                      9, 0),
              },
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GREATER, (int[]){}, 0),
                  make_instruction(OP_JMP_IF_FALSE, (int[]){24}, 1),
                  make_instruction(OP_CLOSURE, (int[]){3, 0}, 2),
                  make_instruction(OP_LOOP, (int[]){}, 0),
                  make_instruction(OP_JMP, (int[]){6}, 1),
              },
          .expected_instructions_len = 13,
      },
      {
          .input = "let a = 0; while (a < 10) { break; };",
          .expected_constants_len = 3,
          .expected_constants =
              {
                  new_number(0),
                  new_number(10),
                  new_concatted_compiled_loop(
                      (Instruction[]){
                          make_instruction(OP_BREAK, (int[]){24}, 1),
                          make_instruction(OP_CONTINUE, (int[]){}, 0),
                      },
                      2, 0),
              },
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_GREATER, (int[]){}, 0),
                  make_instruction(OP_JMP_IF_FALSE, (int[]){24}, 1),
                  make_instruction(OP_CLOSURE, (int[]){2, 0}, 2),
                  make_instruction(OP_LOOP, (int[]){}, 0),
                  make_instruction(OP_JMP, (int[]){6}, 1),
              },
          .expected_instructions_len = 13,
      },

  };

  RUN_COMPILER_TESTS(tests);
}

void test_for_loops(void) {
  compilerTestCase tests[] = {
      {
          .input = "let a = 0;"
                   "for (let b = 0; b < 10; b = b + 1) {"
                   "  a = a + b;                        "
                   "}                                   ",
          .expected_constants_len = 5,
          .expected_constants =
              {
                  new_number(0),
                  new_number(0),
                  new_number(10),
                  new_concatted_compiled_loop(
                      (Instruction[]){
                          make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                          make_instruction(OP_GET_GLOBAL, (int[]){1}, 1),
                          make_instruction(OP_ADD, (int[]){}, 0),
                          make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                          make_instruction(OP_GET_GLOBAL, (int[]){0}, 1),
                          make_instruction(OP_POP, (int[]){}, 0),
                          make_instruction(OP_CONTINUE, (int[]){}, 0),
                      },
                      2, 0),
                  new_number(1),
              },
          .expected_instructions =
              {
                  make_instruction(OP_CONSTANT, (int[]){0}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){0}, 1),
                  make_instruction(OP_CONSTANT, (int[]){1}, 1),
                  make_instruction(OP_SET_GLOBAL, (int[]){1}, 1),
                  make_instruction(OP_CONSTANT, (int[]){2}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){1}, 1),
                  make_instruction(OP_GREATER, (int[]){}, 0),
                  make_instruction(OP_JMP_IF_FALSE, (int[]){44}, 1),
                  make_instruction(OP_CLOSURE, (int[]){3, 0}, 2),
                  make_instruction(OP_LOOP, (int[]){}, 0),
                  make_instruction(OP_GET_GLOBAL, (int[]){1}, 1),
                  make_instruction(OP_CONSTANT, (int[]){4}, 1),
                  make_instruction(OP_ADD, (int[]){}, 0),
                  make_instruction(OP_SET_GLOBAL, (int[]){1}, 1),
                  make_instruction(OP_GET_GLOBAL, (int[]){1}, 1),
                  make_instruction(OP_POP, (int[]){}, 0),
                  make_instruction(OP_JMP, (int[]){12}, 1),
              },
          .expected_instructions_len = 17,
      },
  };

  RUN_COMPILER_TESTS(tests);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_compiler_scopes);
  RUN_TEST(test_function_calls);
  RUN_TEST(test_integer_arithmetic);
  RUN_TEST(test_boolean_expressions);
  RUN_TEST(test_conditionals);
  RUN_TEST(test_global_let_statements);
  RUN_TEST(test_string_expressions);
  RUN_TEST(test_array_literals);
  RUN_TEST(test_hash_literals);
  RUN_TEST(test_index_expressions);
  RUN_TEST(test_functions);
  RUN_TEST(test_let_statement_scopes);
  RUN_TEST(test_builtins);
  RUN_TEST(test_closures);
  RUN_TEST(test_recursive_functions);
  RUN_TEST(test_reassignment);
  RUN_TEST(test_while_loops);
  RUN_TEST(test_loop_control_statements);
  RUN_TEST(test_for_loops);
  return UNITY_END();
}
