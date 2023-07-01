#ifndef COMPILER_H
#define COMPILER_H

#include "../code/code.h"
#include "../ast/ast.h"

typedef enum {
    COMPILER_OK,
    COMPILER_UNKNOWN_OPERATOR,
} CompilerResult;

typedef struct {
    Instructions instructions;
    DynamicArray constants; // Object*[]
} Compiler;

Compiler *new_compiler();

CompilerResult compile_program(Compiler *, Program *);
CompilerResult compile_statement(Compiler *, Statement *);
CompilerResult compile_expression(Compiler *, Expression *);

void free_compiler(Compiler *);


typedef struct {
    Instructions instructions;
    DynamicArray constants;
} Bytecode;

Bytecode bytecode(Compiler *);

void compiler_error(CompilerResult, char *, size_t);

#endif // COMPILER_H
