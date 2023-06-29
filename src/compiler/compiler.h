#include "../code/code.h"
#include "../ast/ast.h"

typedef struct {
    Instructions instructions;
    DynamicArray constants; // Object*[]
} Compiler;

Compiler *new_compiler();

int8_t compile(Compiler *, Program *);

void free_compiler(Compiler *);


typedef struct {
    Instructions instructions;
    DynamicArray constants;
} Bytecode;

Bytecode bytecode(Compiler *);
