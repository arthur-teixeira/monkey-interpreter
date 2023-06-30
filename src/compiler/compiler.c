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

void free_compiler(Compiler *compiler) {
  array_free(&compiler->constants);
  free(compiler);
}

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

int8_t compile_program(Compiler *compiler, Program *program) {
  for (size_t i = 0; i < program->statements.len; i++) {
    int8_t result = compile_statement(compiler, program->statements.arr[i]);
    if (result < 0) {
      return result;
    }
  }

  return 0;
}

int8_t compile_statement(Compiler *compiler, Statement *stmt) {
  switch (stmt->type) {
  case EXPR_STATEMENT:
    return compile_expression(compiler, stmt->expression);
  default:
    assert(0 && "not implemented");
  }
  return 0;
}

int8_t compile_infix_expression(Compiler *compiler, InfixExpression *expr) {
  size_t result = compile_expression(compiler, ((InfixExpression *)expr)->left);
  if (result < 0) {
    return result;
  }

  result = compile_expression(compiler, ((InfixExpression *)expr)->right);
  if (result < 0) {
    return result;
  }

  return 0;
}

int8_t compile_expression(Compiler *compiler, Expression *expr) {
  switch (expr->type) {
  case INFIX_EXPR:
    return compile_infix_expression(compiler, (InfixExpression *)expr);
  case INT_EXPR: {
    Number *num = malloc(sizeof(Number));
    assert(num != NULL);
    num->type = NUMBER_OBJ;
    num->value = ((NumberLiteral *)expr)->value;
    size_t new_constant_pos = add_constant(compiler, (Object *)num);
    int operands[] = {new_constant_pos};
    emit(compiler, OP_CONSTANT, operands, 1);
    break;
  }
  default:
    assert(0 && "not implemented");
  }
  return 0;
}

Bytecode bytecode(Compiler *compiler) {
  Bytecode bytecode = {
      .constants = compiler->constants,
      .instructions = compiler->instructions,
  };

  return bytecode;
}
