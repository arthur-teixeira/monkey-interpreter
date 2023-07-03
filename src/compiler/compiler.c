#include "compiler.h"
#include "../ast/ast.h"
#include "../object/object.h"
#include <assert.h>
#include <stdlib.h>

Compiler *new_compiler() {
  Compiler *compiler = malloc(sizeof(Compiler));
  assert(compiler != NULL);

  array_init(&compiler->constants, 8);
  int_array_init(&compiler->instructions, 8);
  return compiler;
}

void free_compiler(Compiler *compiler) { free(compiler); }

size_t add_constant(Compiler *compiler, Object *obj) {
  array_append(&compiler->constants, obj);
  return compiler->constants.len - 1;
}

size_t add_instruction(Compiler *compiler, Instruction ins) {
  size_t new_instruction_position = compiler->instructions.len;
  for (size_t i = 0; i < ins.len; i++) {
    int_array_append(&compiler->instructions, ins.arr[i]);
  }
  return new_instruction_position;
}

size_t emit(Compiler *compiler, OpCode op, int *operands,
            size_t operand_count) {
  Instruction ins = make_instruction(op, operands, operand_count);
  size_t position = add_instruction(compiler, ins);

  int_array_free(&ins);
  return position;
}

size_t emit_no_operands(Compiler *compiler, OpCode op) {
  return emit(compiler, op, (int[]){}, 0);
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
  default:
    assert(0 && "not implemented");
  }
  return COMPILER_OK;
}

static CompilerResult compile_operand(Compiler *compiler, char *operand) {
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
  }

  if (strncmp(operand, "<<", 2) == 0) {
    emit(compiler, OP_LSHIFT, (int[]){}, 0);
    return COMPILER_OK;
  }
  if (strncmp(operand, ">>", 2) == 0) {
    emit(compiler, OP_RSHIFT, (int[]){}, 0);
    return COMPILER_OK;
  }

  return COMPILER_UNKNOWN_OPERATOR;
}

CompilerResult compile_infix_expression(Compiler *compiler,
                                        InfixExpression *expr) {

  CompilerResult result;
  result = compile_expression(compiler, ((InfixExpression *)expr)->left);
  if (result != COMPILER_OK) {
    return result;
  }

  result = compile_expression(compiler, ((InfixExpression *)expr)->right);
  if (result != COMPILER_OK) {
    return result;
  }

  return compile_operand(compiler, ((InfixExpression *)expr)->operator);
}

CompilerResult compile_expression(Compiler *compiler, Expression *expr) {
  switch (expr->type) {
  case INFIX_EXPR:
    return compile_infix_expression(compiler, (InfixExpression *)expr);
  case INT_EXPR: {
    Object *num = new_number(((NumberLiteral *)expr)->value);
    size_t new_constant_pos = add_constant(compiler, num);
    emit(compiler, OP_CONSTANT, (int[]){new_constant_pos}, 1);
    break;
  }
  default:
    assert(0 && "not implemented");
  }

  return COMPILER_OK;
}

Bytecode bytecode(Compiler *compiler) {
  Bytecode bytecode = {
      .constants = compiler->constants,
      .instructions = compiler->instructions,
  };

  return bytecode;
}

void compiler_error(CompilerResult error, char *buf, size_t bufsize) {
  switch (error) {
  case COMPILER_UNKNOWN_OPERATOR:
    snprintf(buf, bufsize, "unknown operator\n");
    break;
  case COMPILER_OK:
    break;
  }
}
