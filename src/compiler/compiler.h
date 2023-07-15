#ifndef COMPILER_H
#define COMPILER_H

#include "../code/code.h"
#include "../ast/ast.h"
#include "symbol_table.h"

typedef enum {
    COMPILER_OK,
    COMPILER_UNKNOWN_OPERATOR,
    COMPILER_UNKNOWN_STATEMENT,
    COMPILER_UNKNOWN_IDENTIFIER,
    COMPILER_UNINDEXABLE_TYPE,
} CompilerResult;

typedef struct {
  OpCode op;
  size_t position;
} EmmittedInstruction;

typedef struct {
    Instructions instructions;
    DynamicArray *constants; // Object*[]
    SymbolTable *symbol_table;
    EmmittedInstruction last_instruction;
    EmmittedInstruction previous_instruction;
} Compiler;

Compiler *new_compiler();
Compiler *new_compiler_with_state(SymbolTable *, DynamicArray *);

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
