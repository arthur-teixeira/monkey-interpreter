#include <stdint.h>
#include "../dyn_array/dyn_array.h"
#include "../str_utils/str_utils.h"

#define OPERAND_WIDTHS 4

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

Instruction make_instruction(OpCode, int *, size_t);

void instructions_to_string(ResizableBuffer *, const Instructions *);

Definition *lookup(OpCode);

IntArray read_operands(Definition *, const Instructions *instructions, size_t, size_t *);
