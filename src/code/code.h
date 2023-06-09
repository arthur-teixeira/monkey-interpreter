#ifndef CODE_H
#define CODE_H

#include <stdint.h>
#include "../dyn_array/dyn_array.h"
#include "../str_utils/str_utils.h"

#define OPERAND_WIDTHS 4

typedef IntArray Instruction;
typedef IntArray Instructions;

typedef enum {
    OP_CONSTANT,
    OP_ADD,
    OP_POP,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LSHIFT,
    OP_RSHIFT,
    OP_MOD,
    OP_BIT_OR,
    OP_BIT_AND,
    OP_BIT_XOR,
    OP_TRUE,
    OP_FALSE,
    OP_EQ,
    OP_NOT_EQ,
    OP_GREATER,
    OP_MINUS,
    OP_BANG,
    OP_JMP_IF_FALSE,
    OP_JMP,
    OP_NULL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_COUNT,
} OpCode;

typedef struct {
    char *name;
    uint8_t operand_count;
    int operand_widths[OPERAND_WIDTHS];
} Definition;

Instruction make_instruction(OpCode, int *, size_t);

void instructions_to_string(ResizableBuffer *, const Instructions *);

Definition *lookup(OpCode);

IntArray read_operands(Definition *, const Instructions *instructions, size_t, size_t *);

#endif // CODE_H
