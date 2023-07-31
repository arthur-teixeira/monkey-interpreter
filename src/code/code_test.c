#include "../code/code.h"
#include "../dyn_array/dyn_array.h"
#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

void test_make(void) {
  struct testCase {
    OpCode op;
    int operands[2];
    size_t operand_count;
    Instruction expected;
  };
  struct testCase tests[4];

  Instruction op_constant_instructions;
  int_array_init(&op_constant_instructions, 3);
  int_array_append(&op_constant_instructions, OP_CONSTANT);
  int_array_append(&op_constant_instructions, 0xFF);
  int_array_append(&op_constant_instructions, 0xFE);

  struct testCase op_constant_test = {
      .op = OP_CONSTANT,
      .operands = {0xFFFE},
      .operand_count = 1,
      .expected = op_constant_instructions,
  };

  tests[0] = op_constant_test;

  Instruction op_add_instruction;
  int_array_init(&op_add_instruction, 1);
  int_array_append(&op_add_instruction, OP_ADD);

  struct testCase op_add_test = {
      .op = OP_ADD,
      .operands = {},
      .operand_count = 0,
      .expected = op_add_instruction,
  };

  tests[1] = op_add_test;

  Instruction op_get_local_instruction;
  int_array_init(&op_get_local_instruction, 2);
  int_array_append(&op_get_local_instruction, OP_GET_LOCAL);
  int_array_append(&op_get_local_instruction, 0xFF);

  tests[2] = (struct testCase){
      .op = OP_GET_LOCAL,
      .operands = {0xFF},
      .operand_count = 1,
      .expected = op_get_local_instruction,
  };

  Instruction op_closure;
  int_array_init(&op_closure, 4);
  int_array_append(&op_closure, OP_CLOSURE);
  int_array_append(&op_closure, 0xFF);
  int_array_append(&op_closure, 0xFE);
  int_array_append(&op_closure, 0xFF);

  tests[3] = (struct testCase){
      .op = OP_CLOSURE,
      .operand_count = 2,
      .operands = {0xFFFE, 0xFF},
      .expected = op_closure,
  };

  for (uint32_t i = 0; i < ARRAY_LEN(tests); i++) {
    struct testCase test = tests[i];
    Instruction ins = make_instruction(test.op, test.operands, test.operand_count);

    TEST_ASSERT_EQUAL(test.expected.len, ins.len);

    for (uint32_t j = 0; j < test.expected.len; j++) {
      TEST_ASSERT_EQUAL(test.expected.arr[j], ins.arr[j]);
    }

    int_array_free(&ins);
  }

  int_array_free(&op_constant_instructions);
}

void test_instructions_string(void) {
  Instruction instructions[] = {
      make_instruction(OP_ADD, (int[]){}, 0),
      make_instruction(OP_CONSTANT, (int[]){2}, 1),
      make_instruction(OP_CONSTANT, (int[]){65535}, 1),
      make_instruction(OP_GET_LOCAL, (int[]){1}, 1),
      make_instruction(OP_CLOSURE, (int[]){65535, 255}, 2),
  };

  char *expected = ""
                   "0000 OP_ADD\n"
                   "0001 OP_CONSTANT 2\n"
                   "0004 OP_CONSTANT 65535\n"
                   "0007 OP_GET_LOCAL 1\n"
                   "0009 OP_CLOSURE 65535 255\n";

  Instructions ins = concat_instructions(ARRAY_LEN(instructions), instructions);

  ResizableBuffer buf;
  init_resizable_buffer(&buf, strlen(expected));

  instructions_to_string(&buf, &ins);

  TEST_ASSERT_EQUAL_STRING(expected, buf.buf);
  free(buf.buf);
}

void test_read_operands(void) {
  struct testCase {
    OpCode op;
    int operands[2];
    size_t operand_count;
    int bytes_read;
  };

  struct testCase tests[] = {
      {OP_CONSTANT, {65535}, 1, 2},
      {OP_GET_LOCAL, {255}, 1, 1},
      {OP_CLOSURE, {65535, 255}, 2, 3},
  };

  for (size_t i = 0; i < ARRAY_LEN(tests); i++) {
    struct testCase test = tests[i];
    Instruction ins = make_instruction(test.op, test.operands, test.operand_count);

    Definition *def = lookup(test.op);
    TEST_ASSERT_NOT_NULL(def);

    size_t bytes_read;
    IntArray operands = read_operands(def, &ins, 1, &bytes_read);

    TEST_ASSERT_EQUAL(test.bytes_read, bytes_read);

    for (size_t j = 0; j < def->operand_count; j++) {
      TEST_ASSERT_EQUAL(test.operands[j], operands.arr[j]);
    }
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_make);
  RUN_TEST(test_instructions_string);
  RUN_TEST(test_read_operands);
  return UNITY_END();
}
