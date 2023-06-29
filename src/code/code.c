#include "./code.h"
#include "../big_endian/big_endian.h"
#include <assert.h>
#include <stdlib.h>

static Definition definitions[OP_COUNT] = {
    {
        .name = "OP_CONSTANT",
        .operand_widths = {2},
    },
};

Definition *lookup(OpCode_t opcode) {
  if (opcode > OP_COUNT) {
    return NULL;
  }
  return &definitions[opcode];
}

Instruction make_instruction(OpCode_t op_code, int *operands,
                             size_t operand_count) {
  Definition *def = lookup(op_code);
  assert(def != NULL);

  size_t instruction_len = 1;
  for (size_t i = 0; i < operand_count; i++) {
    instruction_len += def->operand_widths[i];
  }

  Instruction instruction;

  int_array_init(&instruction, operand_count + 1);
  int_array_append(&instruction, op_code);

  for (size_t i = 0; i < operand_count; i++) {
    switch (def->operand_widths[i]) {
    case 2:
      big_endian_push_uint16(&instruction, operands[i]);
      break;
    }
  }

  return instruction;
}

char *instructions_to_string(Instructions instructions) {

}
