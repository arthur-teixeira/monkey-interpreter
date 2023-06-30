#include "../code/code.h"
#include "../ast/ast.h"

typedef struct {
    Instructions instructions;
    DynamicArray constants; // Object*[]
} Compiler;

Compiler *new_compiler();

int8_t compile_program(Compiler *, Program *);
int8_t compile_statement(Compiler *, Statement *);
int8_t compile_expression(Compiler *, Expression *);

void free_compiler(Compiler *);


typedef struct {
    Instructions instructions;
    DynamicArray constants;
} Bytecode;

Bytecode bytecode(Compiler *);
