#include "vm.h"
#include "../big_endian/big_endian.h"
#include "../object/constants.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

VM *new_vm(Bytecode bytecode) {
  VM *vm = malloc(sizeof(VM));
  assert(vm != NULL);

  vm->instructions = bytecode.instructions;
  vm->constants = bytecode.constants;
  memset(vm->stack, 0, sizeof(vm->stack));
  vm->sp = 0;
  memset(vm->globals, 0, sizeof(vm->globals));

  return vm;
}

void free_vm(VM *vm) {
  array_free(&vm->constants);
  int_array_free(&vm->instructions);
  free(vm);
}

Object *stack_top(VM *vm) {
  if (vm->sp == 0) {
    return NULL;
  }

  return vm->stack[vm->sp - 1];
}

VMResult stack_push(VM *vm, Object *value) {
  if (vm->sp >= STACK_SIZE) {
    return VM_STACK_OVERFLOW;
  }

  vm->stack[vm->sp++] = value;
  return VM_OK;
}

VMResult stack_push_constant(VM *vm, uint16_t constant_index) {
  return stack_push(vm, vm->constants.arr[constant_index]);
}

Object *stack_pop(VM *vm) { return vm->stack[--vm->sp]; }

VMResult execute_binary_integer_operation(VM *vm, OpCode op, Number *left,
                                          Number *right) {
  switch (op) {
  case OP_ADD:
    return stack_push(vm, new_number(left->value + right->value));
  case OP_SUB:
    return stack_push(vm, new_number(left->value - right->value));
  case OP_MUL:
    return stack_push(vm, new_number(left->value * right->value));
  case OP_DIV:
    return stack_push(vm, new_number(left->value / right->value));
  case OP_MOD:
    return stack_push(vm, new_number((long)left->value % (long)right->value));
  case OP_RSHIFT:
    return stack_push(vm, new_number((long)left->value >> (long)right->value));
  case OP_LSHIFT:
    return stack_push(vm, new_number((long)left->value << (long)right->value));
  case OP_BIT_AND:
    return stack_push(vm, new_number((long)left->value & (long)right->value));
  case OP_BIT_OR:
    return stack_push(vm, new_number((long)left->value | (long)right->value));
  case OP_BIT_XOR:
    return stack_push(vm, new_number((long)left->value ^ (long)right->value));
  default:
    return VM_UNSUPPORTED_OPERATION;
  }
}

VMResult execute_binary_operation(VM *vm, OpCode op) {
  Object *right = stack_pop(vm);
  Object *left = stack_pop(vm);
  if (right->type == NUMBER_OBJ && left->type == NUMBER_OBJ) {
    return execute_binary_integer_operation(vm, op, (Number *)left,
                                            (Number *)right);
  }
  return VM_UNSUPPORTED_OPERATION;
}

VMResult execute_number_comparison(VM *vm, OpCode op, Number *left,
                                   Number *right) {
  switch (op) {
  case OP_EQ:
    return stack_push(
        vm, native_bool_to_boolean_object(left->value == right->value));
  case OP_NOT_EQ:
    return stack_push(
        vm, native_bool_to_boolean_object(left->value != right->value));
  case OP_GREATER:
    return stack_push(
        vm, native_bool_to_boolean_object(left->value > right->value));
  default:
    return VM_UNSUPPORTED_OPERATION;
  }
}

VMResult execute_comparison(VM *vm, OpCode op) {
  Object *right = stack_pop(vm);
  Object *left = stack_pop(vm);
  if (right->type == NUMBER_OBJ && left->type == NUMBER_OBJ) {
    return execute_number_comparison(vm, op, (Number *)left, (Number *)right);
  }

  switch (op) {
  case OP_EQ:
    return stack_push(vm, native_bool_to_boolean_object(right == left));
  case OP_NOT_EQ:
    return stack_push(vm, native_bool_to_boolean_object(right != left));
  default:
    break;
  }

  return VM_UNSUPPORTED_OPERATION;
}

VMResult execute_bang_operator(VM *vm) {
  Object *operand = stack_pop(vm);

  if (operand == (Object *)&obj_true) {
    return stack_push(vm, (Object *)&obj_false);
  }

  if (operand == (Object *)&obj_false) {
    return stack_push(vm, (Object *)&obj_true);
  }

  if (operand == (Object *)&obj_null) {
    return stack_push(vm, (Object *)&obj_true);
  }

  return stack_push(vm, (Object *)&obj_false);
}

VMResult execute_minus_operator(VM *vm) {
  Object *operand = stack_pop(vm);

  if (operand->type != NUMBER_OBJ) {
    return VM_UNSUPPORTED_TYPE_FOR_OPERATION;
  }

  double value = -((Number *)operand)->value;
  return stack_push(vm, new_number(value));
}

static bool is_truthy(Object *obj) {
  if (obj->type == BOOLEAN_OBJ) {
    return ((Boolean *)obj)->value;
  }

  if (obj->type == NULL_OBJ) {
    return false;
  }

  return true;
}

VMResult run_vm(VM *vm) {
  for (size_t ip = 0; ip < vm->instructions.len; ip++) {
    VMResult result;
    OpCode op = vm->instructions.arr[ip];

    switch (op) {
    case OP_CONSTANT: {
      uint16_t constant_index =
          big_endian_read_uint16(&vm->instructions, ip + 1);

      result = stack_push_constant(vm, constant_index);
      if (result != VM_OK) {
        return result;
      }
      ip += 2;
      break;
    }
    case OP_MUL:
    case OP_SUB:
    case OP_DIV:
    case OP_MOD:
    case OP_BIT_OR:
    case OP_BIT_AND:
    case OP_BIT_XOR:
    case OP_RSHIFT:
    case OP_LSHIFT:
    case OP_ADD:
      result = execute_binary_operation(vm, op);
      if (result != VM_OK) {
        return result;
      }
      break;
    case OP_POP:
      stack_pop(vm);
      break;
    case OP_TRUE:
      result = stack_push(vm, (Object *)&obj_true);
      if (result != VM_OK) {
        return result;
      }
      break;
    case OP_FALSE:
      result = stack_push(vm, (Object *)&obj_false);
      if (result != VM_OK) {
        return result;
      }
      break;
    case OP_GREATER:
    case OP_EQ:
    case OP_NOT_EQ:
      result = execute_comparison(vm, op);
      if (result != VM_OK) {
        return result;
      }
      break;
    case OP_BANG:
      result = execute_bang_operator(vm);
      if (result != VM_OK) {
        return result;
      }
      break;
    case OP_MINUS:
      result = execute_minus_operator(vm);
      if (result != VM_OK) {
        return result;
      }
      break;
    case OP_JMP: {
      uint16_t pos = big_endian_read_uint16(&vm->instructions, ip + 1);
      ip = pos - 1;
      break;
    }
    case OP_JMP_IF_FALSE: {
      uint16_t pos = big_endian_read_uint16(&vm->instructions, ip + 1);
      ip += 2; // skip the operand

      Object *condition = stack_pop(vm);
      if (!is_truthy(condition)) {
        ip = pos - 1;
      }
      break;
    }
    case OP_NULL:
      result = stack_push(vm, (Object *)&obj_null);
      if (result != VM_OK) {
        return result;
      }
      break;
    case OP_SET_GLOBAL: {
      uint16_t global_index = big_endian_read_uint16(&vm->instructions, ip + 1);
      ip += 2;
      vm->globals[global_index] = stack_pop(vm);
      break;
    }
    case OP_GET_GLOBAL: {
      uint16_t global_index = big_endian_read_uint16(&vm->instructions, ip + 1);
      ip += 2;
      VMResult result = stack_push(vm, vm->globals[global_index]);
      if (result != VM_OK) {
        return result;
      }
      break;
    }
    case OP_COUNT:
      assert(0 && "unreachable");
    }
  }

  return VM_OK;
}

Object *vm_last_popped_stack_elem(VM *vm) { return vm->stack[vm->sp]; }

void vm_error(VMResult error, char *buf, size_t bufsize) {
  switch (error) {
  case VM_OK:
    return;
  case VM_STACK_OVERFLOW:
    snprintf(buf, bufsize, "Stack overflow");
    return;
  case VM_UNSUPPORTED_OPERATION:
    snprintf(buf, bufsize, "Unsupported operation");
    return;
  case VM_UNSUPPORTED_TYPE_FOR_OPERATION:
    snprintf(buf, bufsize, "Unsupported type for operation");
    return;
  }
}
