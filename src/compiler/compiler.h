#ifndef COMPILER_H
#define COMPILER_H

#include "../ast/ast.h"
#include "../code/code.h"
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
  EmmittedInstruction last_instruction;
  EmmittedInstruction previous_instruction;
} CompilationScope;

typedef struct {
  DynamicArray *constants; // Object*[]
  SymbolTable *symbol_table;
  CompilationScope scopes[256];
  size_t scope_index;
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

void enter_compiler_scope(Compiler *);
Instructions *leave_compiler_scope(Compiler *);

#endif // COMPILER_H
