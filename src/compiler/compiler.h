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
  COMPILER_BREAK_OUTSIDE_LOOP,
  COMPILER_CONTINUE_OUTSIDE_LOOP,
} CompilerResult;

typedef struct {
  OpCode op;
  size_t position;
} EmmittedInstruction;

typedef struct {
  bool in_loop;
  size_t break_location_index;
  size_t break_locations[100];
} CurrentLoop;

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
  bool is_void_expression;
  CurrentLoop loop;
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

void save_to_file(Bytecode, const char *);
void enter_loop(Compiler *);
CurrentLoop exit_loop(Compiler *);
void patch_break_statements(Compiler *, size_t, CurrentLoop, Instructions *);
void new_break_statement(Compiler *);

#endif // COMPILER_H
