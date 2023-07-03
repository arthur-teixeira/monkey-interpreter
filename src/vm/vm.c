#include "vm.h"
#include "../big_endian/big_endian.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

VM *new_vm(Bytecode bytecode) {
  VM *vm = malloc(sizeof(VM));
  assert(vm != NULL);

  vm->instructions = bytecode.instructions;
  vm->constants = bytecode.constants;
  memset(vm->stack, 0, sizeof(vm->stack));
  vm->sp = 0;

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

Object *stack_pop(VM *vm) {
  return vm->stack[--vm->sp];
}

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

VMResult run_vm(VM *vm) {
  for (size_t ip = 0; ip < vm->instructions.len; ip++) {
    OpCode op = vm->instructions.arr[ip];

    switch (op) {
    case OP_CONSTANT: {
      uint16_t constant_index =
          big_endian_read_uint16(&vm->instructions, ip + 1);

      VMResult result = stack_push_constant(vm, constant_index);
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
    case OP_ADD: {
      VMResult result = execute_binary_operation(vm, op);
      if (result != VM_OK) {
        return result;
      }
      break;
    }
    case OP_POP:
      stack_pop(vm);
      break;
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
  }
}
