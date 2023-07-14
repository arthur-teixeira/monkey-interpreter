#include "compiler.h"
#include "../ast/ast.h"
#include "../object/object.h"
#include "symbol_table.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define JUMP_SENTINEL 9999

Compiler *new_compiler() {
  Compiler *compiler = malloc(sizeof(Compiler));
  assert(compiler != NULL);

  compiler->constants = malloc(sizeof(DynamicArray));
  assert(compiler->constants != NULL);
  array_init(compiler->constants, 8);

  int_array_init(&compiler->instructions, 8);
  compiler->last_instruction = (EmmittedInstruction){};
  compiler->previous_instruction = (EmmittedInstruction){};
  compiler->symbol_table = new_symbol_table();

  return compiler;
}

Compiler *new_compiler_with_state(SymbolTable *symbol_table,
                                  DynamicArray *constants) {
  Compiler *compiler = malloc(sizeof(Compiler));
  assert(compiler != NULL);

  int_array_init(&compiler->instructions, 8);
  compiler->last_instruction = (EmmittedInstruction){};
  compiler->previous_instruction = (EmmittedInstruction){};
  compiler->constants = constants;
  compiler->symbol_table = symbol_table;

  return compiler;
}

void free_compiler(Compiler *compiler) {
  free_symbol_table(compiler->symbol_table);
  free(compiler);
}

size_t add_constant(Compiler *compiler, Object *obj) {
  array_append(compiler->constants, obj);
  return compiler->constants->len - 1;
}

size_t add_instruction(Compiler *compiler, Instruction ins) {
  size_t new_instruction_position = compiler->instructions.len;
  for (size_t i = 0; i < ins.len; i++) {
    int_array_append(&compiler->instructions, ins.arr[i]);
  }
  return new_instruction_position;
}

void set_last_instruction(Compiler *compiler, OpCode op, size_t pos) {
  EmmittedInstruction previous = compiler->last_instruction;
  EmmittedInstruction last = (EmmittedInstruction){op, pos};

  compiler->previous_instruction = previous;
  compiler->last_instruction = last;
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

bool last_instruction_is(Compiler *compiler, OpCode op) {
  return compiler->last_instruction.op == op;
}

void remove_last_pop(Compiler *compiler) {
  compiler->instructions.len -= 1;
  compiler->last_instruction = compiler->previous_instruction;
}

void replace_instruction(Compiler *compiler, size_t pos, Instruction ins) {
  for (size_t i = 0; i < ins.len; i++) {
    compiler->instructions.arr[pos + i] = ins.arr[i];
  }
}

void change_operand(Compiler *compiler, size_t opPos, size_t operand) {
  OpCode op = compiler->instructions.arr[opPos];
  Instruction new_instruction = make_instruction(op, (int[]){operand}, 1);

  replace_instruction(compiler, opPos, new_instruction);
  int_array_free(&new_instruction);
}

CompilerResult compile_program(Compiler *compiler, Program *program) {
  for (size_t i = 0; i < program->statements.len; i++) {
    int8_t result = compile_statement(compiler, program->statements.arr[i]);
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
    emit_no_operands(compiler, OP_POP);
    break;
  }
  case LET_STATEMENT: {
    CompilerResult result = compile_expression(compiler, stmt->expression);
    if (result != COMPILER_OK) {
      return result;
    }

    const Symbol *symbol =
        symbol_define(compiler->symbol_table, stmt->name->value);

    emit(compiler, OP_SET_GLOBAL, (int[]){symbol->index}, 1);
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

  size_t after_consequence_pos = compiler->instructions.len;
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

  size_t after_alternative_pos = compiler->instructions.len;
  change_operand(compiler, jmp_pos, after_alternative_pos);

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
    emit(compiler, OP_GET_GLOBAL, (int[]){symbol->index}, 1);
    break;
  }
  case STRING_EXPR: {
    StringLiteral *str_lit = (StringLiteral *)expr;
    Object *str = new_string(str_lit->value, str_lit->len);
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
  default:
    return COMPILER_UNKNOWN_OPERATOR;
  }

  return COMPILER_OK;
}

Bytecode bytecode(Compiler *compiler) {
  Bytecode bytecode = {
      .constants = *compiler->constants,
      .instructions = compiler->instructions,
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
  case COMPILER_OK:
    break;
  }
}
