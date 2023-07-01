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

VMError stack_push_constant(VM *vm, uint16_t constant_index) {
  if (vm->sp >= STACK_SIZE) {
    return VM_STACK_OVERFLOW;
  }
  vm->stack[vm->sp++] = vm->constants.arr[constant_index];
  return VM_OK;
}

VMError run_vm(VM *vm) {
  for (size_t ip = 0; ip < vm->instructions.len; ip++) {
    OpCode op = vm->instructions.arr[ip];

    switch (op) {
    case OP_CONSTANT: {
      uint16_t constant_index =
          big_endian_read_uint16(&vm->instructions, ip + 1);

      VMError result = stack_push_constant(vm, constant_index);
      if (result != VM_OK) {
        return result;
      }
      ip += 2;
      break;
    }
    case OP_COUNT:
      assert(0 && "unreachable");
    }
  }

  return VM_OK;
}

void vm_error(VMError error, char *buf, size_t bufsize) {
  switch (error) {
  case VM_OK:
    return;
  case VM_STACK_OVERFLOW:
    snprintf(buf, bufsize, "Stack overflow");
    return;
  }
}
