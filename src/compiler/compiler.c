#include "compiler.h"
#include "../ast/ast.h"
#include "../object/builtins.h"
#include "../object/object.h"
#include "symbol_table.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define JUMP_SENTINEL 9999

CompilationScope new_compilation_scope(void) {
  CompilationScope new_scope = {
      .last_instruction = (EmmittedInstruction){},
      .previous_instruction = (EmmittedInstruction){},
  };

  int_array_init(&new_scope.instructions, 8);

  return new_scope;
}

Compiler *new_compiler() {
  Compiler *compiler = malloc(sizeof(Compiler));
  assert(compiler != NULL);

  compiler->constants = malloc(sizeof(DynamicArray));
  assert(compiler->constants != NULL);
  array_init(compiler->constants, 8);

  SymbolTable *symbol_table = new_symbol_table();
  for (size_t i = 0; i < builtin_definitions_len; i++) {
    symbol_define_builtin(symbol_table, i, builtin_definitions[i].name);
  }

  compiler->symbol_table = symbol_table;
  compiler->scopes[0] = new_compilation_scope();
  compiler->scope_index = 0;
  compiler->is_void_expression = false;

  return compiler;
}

CompilationScope *compiler_current_scope(Compiler *c) {
  return &c->scopes[c->scope_index];
}

Instructions *compiler_current_instructions(Compiler *compiler) {
  return &compiler->scopes[compiler->scope_index].instructions;
}

Compiler *new_compiler_with_state(SymbolTable *symbol_table,
                                  DynamicArray *constants) {
  Compiler *compiler = new_compiler();
  assert(compiler != NULL);

  array_free(compiler->constants);
  free_symbol_table(compiler->symbol_table);

  compiler->constants = constants;
  compiler->symbol_table = symbol_table;

  return compiler;
}

void free_compiler(Compiler *compiler) {
  while (compiler->scope_index > 0) {
    leave_compiler_scope(compiler);
  }

  free_symbol_table(compiler->symbol_table);
  free(compiler);
}

size_t add_constant(Compiler *compiler, Object *obj) {
  array_append(compiler->constants, obj);
  return compiler->constants->len - 1;
}

size_t add_instruction(Compiler *compiler, Instruction ins) {
  size_t new_instruction_position =
      compiler_current_instructions(compiler)->len;
  for (size_t i = 0; i < ins.len; i++) {
    int_array_append(compiler_current_instructions(compiler), ins.arr[i]);
  }
  return new_instruction_position;
}

void set_last_instruction(Compiler *c, OpCode op, size_t pos) {
  EmmittedInstruction previous = compiler_current_scope(c)->last_instruction;
  EmmittedInstruction last = (EmmittedInstruction){op, pos};

  compiler_current_scope(c)->previous_instruction = previous;
  compiler_current_scope(c)->last_instruction = last;
}

size_t emit(Compiler *compiler, OpCode op, int *operands,
            size_t operand_count) {
  Instruction ins = make_instruction(op, operands, operand_count);
  size_t position = add_instruction(compiler, ins);

  set_last_instruction(compiler, op, position);

  int_array_free(&ins);
  return position;
}

size_t emit_no_operands(Compiler *compiler, OpCode op) {
  return emit(compiler, op, (int[]){}, 0);
}

void load_symbol(Compiler *c, Symbol *s) {
  switch (s->scope) {
  case SYMBOL_GLOBAL_SCOPE:
    emit(c, OP_GET_GLOBAL, (int[]){s->index}, 1);
    break;
  case SYMBOL_LOCAL_SCOPE:
    emit(c, OP_GET_LOCAL, (int[]){s->index}, 1);
    break;
  case SYMBOL_BUILTIN_SCOPE:
    emit(c, OP_GET_BUILTIN, (int[]){s->index}, 1);
    break;
  case SYMBOL_FREE_SCOPE:
    emit(c, OP_GET_FREE, (int[]){s->index}, 1);
    break;
  case SYMBOL_FUNCTION_SCOPE:
    emit(c, OP_CURRENT_CLOSURE, (int[]){}, 0);
    break;
  }
}

// TODO: this is wrong in the context of a reassignment, since any variable
// can be reassigned, not only globals and locals
void save_symbol(Compiler *c, const Symbol *s) {
  switch (s->scope) {
  case SYMBOL_FUNCTION_SCOPE:
    break;
  case SYMBOL_GLOBAL_SCOPE:
    emit(c, OP_SET_GLOBAL, (int[]){s->index}, 1);
    break;
  case SYMBOL_LOCAL_SCOPE:
    emit(c, OP_SET_LOCAL, (int[]){s->index}, 1);
    break;
  case SYMBOL_BUILTIN_SCOPE:
  case SYMBOL_FREE_SCOPE:
    assert(0 && "unreachable");
    break;
  }
}

bool last_instruction_is(Compiler *c, OpCode op) {
  return compiler_current_scope(c)->last_instruction.op == op;
}

void remove_last_pop(Compiler *c) {
  compiler_current_instructions(c)->len -= 1;
  compiler_current_scope(c)->last_instruction =
      compiler_current_scope(c)->previous_instruction;
}

void replace_instruction(Compiler *compiler, size_t pos, Instruction ins) {
  for (size_t i = 0; i < ins.len; i++) {
    compiler_current_instructions(compiler)->arr[pos + i] = ins.arr[i];
  }
}

void change_operand(Compiler *compiler, size_t opPos, size_t operand) {
  OpCode op = compiler_current_instructions(compiler)->arr[opPos];
  Instruction new_instruction = make_instruction(op, (int[]){operand}, 1);

  replace_instruction(compiler, opPos, new_instruction);
  int_array_free(&new_instruction);
}

CompilerResult compile_program(Compiler *compiler, Program *program) {
  for (size_t i = 0; i < program->statements.len; i++) {
    CompilerResult result =
        compile_statement(compiler, program->statements.arr[i]);
    if (result != COMPILER_OK) {
      return result;
    }
  }

  return COMPILER_OK;
}

CompilerResult compile_statement(Compiler *compiler, Statement *stmt) {
  switch (stmt->type) {
  case EXPR_STATEMENT: {
    CompilerResult result = compile_expression(compiler, stmt->expression);
    if (result != COMPILER_OK) {
      return result;
    }
    if (compiler->is_void_expression) {
      compiler->is_void_expression = false;
    } else {
      emit_no_operands(compiler, OP_POP);
    }

    break;
  }
  case LET_STATEMENT: {
    const Symbol *symbol =
        symbol_define(compiler->symbol_table, stmt->name->value);

    CompilerResult result = compile_expression(compiler, stmt->expression);
    if (result != COMPILER_OK) {
      return result;
    }

    save_symbol(compiler, symbol);

    break;
  }
  case RETURN_STATEMENT: {
    CompilerResult result = compile_expression(compiler, stmt->expression);
    if (result != COMPILER_OK) {
      return result;
    }

    emit_no_operands(compiler, OP_RETURN_VALUE);
    break;
  }
  default:
    assert(0 && "not implemented");
  }

  return COMPILER_OK;
}

static CompilerResult compile_infix_operand(Compiler *compiler, char *operand) {
  if (strncmp(operand, "<<", 2) == 0) {
    emit(compiler, OP_LSHIFT, (int[]){}, 0);
    return COMPILER_OK;
  }
  if (strncmp(operand, ">>", 2) == 0) {
    emit(compiler, OP_RSHIFT, (int[]){}, 0);
    return COMPILER_OK;
  }
  if (strncmp(operand, "==", 2) == 0) {
    emit_no_operands(compiler, OP_EQ);
    return COMPILER_OK;
  }
  if (strncmp(operand, "!=", 2) == 0) {
    emit_no_operands(compiler, OP_NOT_EQ);
    return COMPILER_OK;
  }

  switch (operand[0]) {
  case '+':
    emit_no_operands(compiler, OP_ADD);
    return COMPILER_OK;
  case '-':
    emit_no_operands(compiler, OP_SUB);
    return COMPILER_OK;
  case '*':
    emit_no_operands(compiler, OP_MUL);
    return COMPILER_OK;
  case '/':
    emit_no_operands(compiler, OP_DIV);
    return COMPILER_OK;
  case '&':
    emit_no_operands(compiler, OP_BIT_AND);
    return COMPILER_OK;
  case '|':
    emit_no_operands(compiler, OP_BIT_OR);
    return COMPILER_OK;
  case '^':
    emit_no_operands(compiler, OP_BIT_XOR);
    return COMPILER_OK;
  case '%':
    emit_no_operands(compiler, OP_MOD);
    return COMPILER_OK;
  case '>':
    emit_no_operands(compiler, OP_GREATER);
    return COMPILER_OK;
  }

  return COMPILER_UNKNOWN_OPERATOR;
}

CompilerResult compile_infix_expression(Compiler *compiler,
                                        InfixExpression *expr) {
  CompilerResult result;
  if (strcmp(expr->operator, "<") == 0) {
    result = compile_expression(compiler, expr->right);
    if (result != COMPILER_OK) {
      return result;
    }

    result = compile_expression(compiler, expr->left);
    if (result != COMPILER_OK) {
      return result;
    }

    emit_no_operands(compiler, OP_GREATER);
    return COMPILER_OK;
  }

  result = compile_expression(compiler, ((InfixExpression *)expr)->left);
  if (result != COMPILER_OK) {
    return result;
  }

  result = compile_expression(compiler, ((InfixExpression *)expr)->right);
  if (result != COMPILER_OK) {
    return result;
  }

  return compile_infix_operand(compiler, ((InfixExpression *)expr)->operator);
}

CompilerResult compile_prefix_operand(Compiler *compiler, char *operand) {
  switch (operand[0]) {
  case '!':
    emit_no_operands(compiler, OP_BANG);
    return COMPILER_OK;
  case '-':
    emit_no_operands(compiler, OP_MINUS);
    return COMPILER_OK;
  }

  return COMPILER_UNKNOWN_OPERATOR;
}

CompilerResult compile_prefix_expression(Compiler *compiler,
                                         PrefixExpression *expr) {

  CompilerResult result = compile_expression(compiler, expr->right);
  if (result != COMPILER_OK) {
    return result;
  }

  return compile_prefix_operand(compiler, expr->operator);
}

CompilerResult compile_block_statement(Compiler *compiler,
                                       BlockStatement *stmt) {
  for (size_t i = 0; i < stmt->statements.len; i++) {
    CompilerResult result =
        compile_statement(compiler, stmt->statements.arr[i]);
    if (result != COMPILER_OK) {
      return result;
    }
  }

  return COMPILER_OK;
}

CompilerResult compile_if_expression(Compiler *compiler, IfExpression *expr) {
  CompilerResult result = compile_expression(compiler, expr->condition);
  if (result != COMPILER_OK) {
    return result;
  }

  size_t jmp_if_false_pos =
      emit(compiler, OP_JMP_IF_FALSE, (int[]){JUMP_SENTINEL}, 1);

  result = compile_block_statement(compiler, expr->consequence);
  if (result != COMPILER_OK) {
    return result;
  }

  if (last_instruction_is(compiler, OP_POP)) {
    remove_last_pop(compiler);
  }
  size_t jmp_pos = emit(compiler, OP_JMP, (int[]){JUMP_SENTINEL}, 1);

  size_t after_consequence_pos = compiler_current_instructions(compiler)->len;
  change_operand(compiler, jmp_if_false_pos, after_consequence_pos);

  if (expr->alternative) {
    result = compile_block_statement(compiler, expr->alternative);
    if (result != COMPILER_OK) {
      return result;
    }

    if (last_instruction_is(compiler, OP_POP)) {
      remove_last_pop(compiler);
    }
  } else {
    emit_no_operands(compiler, OP_NULL);
  }

  size_t after_alternative_pos = compiler_current_instructions(compiler)->len;
  change_operand(compiler, jmp_pos, after_alternative_pos);

  return COMPILER_OK;
}

typedef struct {
  Compiler *compiler;
  CompilerResult result;
} HashCompilerContext;

int compile_hash_pair(void *const ctx, struct hashmap_element_s *const pair) {
  HashCompilerContext *const context = ctx;
  Expression *key = (Expression *)pair->key;

  CompilerResult result = compile_expression(context->compiler, key);
  if (result != COMPILER_OK) {
    context->result = result;
    return 1;
  }

  Expression *data = pair->data;
  result = compile_expression(context->compiler, data);
  if (result != COMPILER_OK) {
    context->result = result;
    return 1;
  }

  return 0;
}

CompilerResult compile_hash_expression(Compiler *compiler, HashLiteral *expr) {
  HashLiteral *hash_lit = (HashLiteral *)expr;
  HashCompilerContext context = {
      .compiler = compiler,
      .result = COMPILER_OK,
  };

  hashmap_iterate_pairs(&expr->pairs, &compile_hash_pair, &context);
  if (context.result != COMPILER_OK) {
    return context.result;
  }

  emit(compiler, OP_HASH, (int[]){hash_lit->len * 2}, 1);

  return context.result;
}

CompilerResult compile_function_literal(Compiler *compiler,
                                        FunctionLiteral *fn) {
  enter_compiler_scope(compiler);

  if (fn->name) {
    symbol_define_function_name(compiler->symbol_table, fn->name);
  }

  for (size_t i = 0; i < fn->parameters.len; i++) {
    Identifier *param = fn->parameters.arr[i];
    assert(param->type == IDENT_EXPR);
    symbol_define(compiler->symbol_table, param->value);
  }

  CompilerResult result = compile_block_statement(compiler, fn->body);
  if (result != COMPILER_OK) {
    return result;
  }

  if (last_instruction_is(compiler, OP_POP)) {
    Instruction new_instruction =
        make_instruction(OP_RETURN_VALUE, (int[]){}, 0);
    replace_instruction(
        compiler, compiler_current_scope(compiler)->last_instruction.position,
        new_instruction);
  }

  if (!last_instruction_is(compiler, OP_RETURN_VALUE)) {
    emit_no_operands(compiler, OP_RETURN);
  }

  Symbol *free_symbols = compiler->symbol_table->free_symbols;
  size_t free_symbols_len = compiler->symbol_table->free_symbols_len;

  size_t num_locals = compiler->symbol_table->num_definitions;

  Instructions *instructions = leave_compiler_scope(compiler);

  for (size_t i = 0; i < free_symbols_len; i++) {
    load_symbol(compiler, &free_symbols[i]);
  }

  Object *compiled_fn =
      new_compiled_function(instructions, num_locals, fn->parameters.len);

  size_t new_constant_pos = add_constant(compiler, compiled_fn);
  emit(compiler, OP_CLOSURE, (int[]){new_constant_pos, free_symbols_len}, 2);

  return COMPILER_OK;
}

CompilerResult compile_while_loop(Compiler *compiler, WhileLoop *loop) {
  size_t loop_condition_pos =
    compiler_current_scope(compiler)->last_instruction.position;

  CompilerResult result = compile_expression(compiler, loop->condition);
  if (result != COMPILER_OK) {
    return result;
  }

  size_t jmp_if_false_pos =
      emit(compiler, OP_JMP_IF_FALSE, (int[]){JUMP_SENTINEL}, 1);

  result = compile_block_statement(compiler, loop->body);
  if (result != COMPILER_OK) {
    return result;
  }

  emit(compiler, OP_JMP, (int[]){loop_condition_pos + 1}, 1);

  size_t after_consequence_pos = compiler_current_instructions(compiler)->len;
  change_operand(compiler, jmp_if_false_pos, after_consequence_pos);

  compiler->is_void_expression = true;

  return COMPILER_OK;
}

CompilerResult compile_reassignment(Compiler *compiler, Reassignment *expr) {
  const Symbol *old_symbol =
      symbol_resolve(compiler->symbol_table, expr->name->value);

  if (!old_symbol) {
    return COMPILER_UNKNOWN_IDENTIFIER;
  }

  CompilerResult result = compile_expression(compiler, expr->value);
  if (result != COMPILER_OK) {
    return result;
  }

  switch (old_symbol->scope) {
  case SYMBOL_GLOBAL_SCOPE:
    emit(compiler, OP_SET_GLOBAL, (int[]){old_symbol->index}, 1);
    emit(compiler, OP_GET_GLOBAL, (int[]){old_symbol->index}, 1);
    break;
  case SYMBOL_LOCAL_SCOPE:
    emit(compiler, OP_SET_LOCAL, (int[]){old_symbol->index}, 1);
    emit(compiler, OP_GET_LOCAL, (int[]){old_symbol->index}, 1);
    break;
  case SYMBOL_FREE_SCOPE:
    emit(compiler, OP_SET_FREE, (int[]){old_symbol->index}, 1);
    emit(compiler, OP_GET_FREE, (int[]){old_symbol->index}, 1);
    break;
  case SYMBOL_FUNCTION_SCOPE:
  case SYMBOL_BUILTIN_SCOPE:
    assert(0 && "unreachable");
    break;
  }

  return COMPILER_OK;
}

CompilerResult compile_expression(Compiler *compiler, Expression *expr) {
  switch (expr->type) {
  case INFIX_EXPR:
    return compile_infix_expression(compiler, (InfixExpression *)expr);
  case PREFIX_EXPR:
    return compile_prefix_expression(compiler, (PrefixExpression *)expr);
  case INT_EXPR: {
    Object *num = new_number(((NumberLiteral *)expr)->value);
    size_t new_constant_pos = add_constant(compiler, num);
    emit(compiler, OP_CONSTANT, (int[]){new_constant_pos}, 1);
    break;
  }
  case BOOL_EXPR: {
    if (((BooleanLiteral *)expr)->value) {
      emit_no_operands(compiler, OP_TRUE);
    } else {
      emit_no_operands(compiler, OP_FALSE);
    }
    break;
  }
  case IF_EXPR:
    return compile_if_expression(compiler, (IfExpression *)expr);
  case IDENT_EXPR: {
    const Symbol *symbol =
        symbol_resolve(compiler->symbol_table, ((Identifier *)expr)->value);
    if (!symbol) {
      return COMPILER_UNKNOWN_IDENTIFIER;
    }

    load_symbol(compiler, (Symbol *)symbol);

    break;
  }
  case STRING_EXPR: {
    StringLiteral *str_lit = (StringLiteral *)expr;
    Object *str = new_string(str_lit->value);
    size_t new_constant_pos = add_constant(compiler, str);
    emit(compiler, OP_CONSTANT, (int[]){new_constant_pos}, 1);
    break;
  }
  case ARRAY_EXPR: {
    ArrayLiteral *arr_lit = (ArrayLiteral *)expr;
    for (size_t i = 0; i < arr_lit->elements->len; i++) {
      CompilerResult result =
          compile_expression(compiler, arr_lit->elements->arr[i]);
      if (result != COMPILER_OK) {
        return result;
      }
    }
    emit(compiler, OP_ARRAY, (int[]){arr_lit->elements->len}, 1);
    break;
  }
  case HASH_EXPR:
    return compile_hash_expression(compiler, (HashLiteral *)expr);
  case INDEX_EXPR: {
    IndexExpression *index_expr = (IndexExpression *)expr;
    CompilerResult result = compile_expression(compiler, index_expr->left);
    if (result != COMPILER_OK) {
      return result;
    }
    result = compile_expression(compiler, index_expr->index);
    if (result != COMPILER_OK) {
      return result;
    }

    emit_no_operands(compiler, OP_INDEX);
    break;
  }
  case FN_EXPR:
    return compile_function_literal(compiler, (FunctionLiteral *)expr);
  case CALL_EXPR: {
    CallExpression *call = (CallExpression *)expr;
    CompilerResult result = compile_expression(compiler, call->function);
    if (result != COMPILER_OK) {
      return result;
    }

    for (size_t i = 0; i < call->arguments.len; i++) {
      CompilerResult result =
          compile_expression(compiler, call->arguments.arr[i]);
      if (result != COMPILER_OK) {
        return result;
      }
    }

    emit(compiler, OP_CALL, (int[]){call->arguments.len}, 1);
    break;
  }
  case WHILE_EXPR:
    return compile_while_loop(compiler, (WhileLoop *)expr);
  case REASSIGN_EXPR:
    return compile_reassignment(compiler, (Reassignment *)expr);
  default:
    return COMPILER_UNKNOWN_OPERATOR;
  }

  return COMPILER_OK;
}

Bytecode bytecode(Compiler *compiler) {
  Bytecode bytecode = {
      .constants = *compiler->constants,
      .instructions = *compiler_current_instructions(compiler),
  };

  return bytecode;
}

void compiler_error(CompilerResult error, char *buf, size_t bufsize) {
  // TODO: save error message in compiler with more info
  switch (error) {
  case COMPILER_UNKNOWN_OPERATOR:
    snprintf(buf, bufsize, "unknown operator");
    break;
  case COMPILER_UNKNOWN_STATEMENT:
    snprintf(buf, bufsize, "unknown statement");
    break;
  case COMPILER_UNKNOWN_IDENTIFIER:
    snprintf(buf, bufsize, "unknown identifier");
    break;
  case COMPILER_UNINDEXABLE_TYPE:
    snprintf(buf, bufsize, "unknown type");
    break;
  case COMPILER_OK:
    break;
  }
}

void enter_compiler_scope(Compiler *c) {
  c->scopes[++c->scope_index] = new_compilation_scope();
  c->symbol_table = new_enclosed_symbol_table(c->symbol_table);
}

Instructions *leave_compiler_scope(Compiler *c) {
  Instructions *instructions = compiler_current_instructions(c);
  c->scope_index--;

  SymbolTable *temp = c->symbol_table;
  c->symbol_table = c->symbol_table->outer;
  free_symbol_table(temp);

  return instructions;
}
