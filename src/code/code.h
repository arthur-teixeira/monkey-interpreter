#include <stdint.h>
#include "../dyn_array/dyn_array.h"

#define OPERAND_WIDTHS 4

typedef uint8_t OpCode_t;
typedef IntArray Instruction; // IntArray<Opcode_t>
typedef IntArray Instructions;

typedef enum {
    OP_CONSTANT,
    OP_COUNT,
} OpCode;

typedef struct {
    char *name;
    uint8_t operand_count;
    int operand_widths[OPERAND_WIDTHS];
} Definition;

Instruction make_instruction(OpCode_t, int *, size_t);

char *instructions_to_string(Instructions);
