#include "./code.h"
#include "../big_endian/big_endian.h"
#include "../str_utils/str_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 100

static Definition definitions[OP_COUNT] = {
    {
        .name = "OP_CONSTANT",
        .operand_count = 1,
        .operand_widths = {2},
    },
    {"OP_ADD"},
    {"OP_POP"},
    {"OP_SUB"},
    {"OP_MUL"},
    {"OP_DIV"},
    {"OP_LSHIFT"},
    {"OP_RSHIFT"},
    {"OP_MOD"},
    {"OP_BIT_OR"},
    {"OP_BIT_AND"},
    {"OP_BIT_XOR"},
    {"OP_TRUE"},
    {"OP_FALSE"},
    {"OP_EQ"},
    {"OP_NOT_EQ"},
    {"OP_GREATER"},
    {"OP_MINUS"},
    {"OP_BANG"},
    {
        .name = "OP_JMP_IF_FALSE",
        .operand_count = 1,
        .operand_widths = {2},
    },
    {
        .name = "OP_JMP",
        .operand_count = 1,
        .operand_widths = {2},
    },
    {"OP_NULL"},
    {
        .name = "OP_GET_GLOBAL",
        .operand_count = 1,
        .operand_widths = {2},
    },
    {
        .name = "OP_SET_GLOBAL",
        .operand_count = 1,
        .operand_widths = {2},
    },
    {
        .name = "OP_ARRAY",
        .operand_count = 1,
        .operand_widths = {2},
    },
    {
        .name = "OP_HASH",
        .operand_count = 1,
        .operand_widths = {2},
    },
    {"OP_INDEX"},
    {
        .name = "OP_CALL",
        .operand_count = 1,
        .operand_widths = {1},
    },
    {"OP_RETURN_VALUE"},
    {"OP_RETURN"},
    {
        .name = "OP_GET_LOCAL",
        .operand_count = 1,
        .operand_widths = {1},
    },
    {
        .name = "OP_SET_LOCAL",
        .operand_count = 1,
        .operand_widths = {1},
    },
    {
        .name = "OP_GET_BUILTIN",
        .operand_count = 1,
        .operand_widths = {1},
    },
    {
        .name = "OP_CLOSURE",
        .operand_count = 2,
        .operand_widths = {2, 1},
    },
    {
        .name = "OP_GET_FREE",
        .operand_count = 1,
        .operand_widths = {1},
    },
    {"OP_CURRENT_CLOSURE"},
    {
      .name = "OP_SET_FREE",
      .operand_count = 1,
      .operand_widths = {1},
    },
    {"OP_LOOP"},
};

Definition *lookup(OpCode opcode) {
  if (opcode > OP_COUNT) {
    return NULL;
  }
  return &definitions[opcode];
}

Instruction make_instruction(OpCode op_code, int *operands,
                             size_t operand_count) {
  Definition *def = lookup(op_code);
  assert(def != NULL);

  Instruction instruction;

  int_array_init(&instruction, operand_count + 1);
  int_array_append(&instruction, (uint8_t)op_code);

  for (size_t i = 0; i < operand_count; i++) {
    switch (def->operand_widths[i]) {
    case 1:
      int_array_append(&instruction, (uint8_t)operands[i]);
      break;
    case 2:
      big_endian_push_uint16(&instruction, operands[i]);
      break;
    }
  }

  return instruction;
}

Instructions concat_instructions(size_t count, Instruction *instructions) {
  Instructions out;
  int_array_init(&out, count);

  for (uint32_t i = 0; i < count; i++) {
    Instruction ins = instructions[i];
    for (uint32_t j = 0; j < ins.len; j++) {
      int_array_append(&out, ins.arr[j]);
    }
  }

  return out;
}

void format_instruction(char buf[MAX_LEN], const Definition *def,
                        const IntArray *operands) {
  if (operands->len != def->operand_count) {
    sprintf(buf, "ERROR: operand len %ld does not match defined %d\n",
            operands->len, def->operand_count);
    return;
  }

  switch (def->operand_count) {
  case 0:
    sprintf(buf, "%s", def->name);
    return;
  case 1:
    sprintf(buf, "%s %d", def->name, operands->arr[0]);
    return;
  case 2:
    sprintf(buf, "%s %d %d", def->name, operands->arr[0], operands->arr[1]);
    return;
  default:
    assert(0 && "unhandled operand count");
  };
}

void instructions_to_string(ResizableBuffer *buf,
                            const Instructions *instructions) {
  size_t i = 0;
  while (i < instructions->len) {
    Definition *def = lookup(instructions->arr[i]);
    if (!def) {
      char msg[100];
      sprintf(msg, "ERROR: unknown opcode %d\n", instructions->arr[i]);
      append_to_buf(buf, msg);
      i++;      // skip unknown opcode, but possibly corrupts the rest of the
                // instructions
      continue; // should be break?
    }

    size_t bytes_read;
    IntArray operands = read_operands(def, instructions, i + 1, &bytes_read);

    char instruction_as_string[MAX_LEN];
    format_instruction(instruction_as_string, def, &operands);

    char msg[2 * MAX_LEN];
    sprintf(msg, "%04ld %s\n", i, instruction_as_string);
    append_to_buf(buf, msg);

    i += 1 + bytes_read;
  }
}

IntArray read_operands(Definition *def, const Instruction *instructions,
                       size_t instruction_operands_offset, size_t *bytes_read) {
  IntArray operands;
  int_array_init(&operands, def->operand_count);

  size_t offset = instruction_operands_offset;
  for (size_t i = 0; i < def->operand_count; i++) {
    switch (def->operand_widths[i]) {
    case 2:
      int_array_append(&operands, big_endian_read_uint16(instructions, offset));
      break;
    case 1:
      int_array_append(&operands, instructions->arr[offset]);
      break;
    }

    offset += def->operand_widths[i];
  }

  *bytes_read = offset - instruction_operands_offset;
  return operands;
}
