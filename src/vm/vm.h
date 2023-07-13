#ifndef VM_H
#define VM_H
#include "../code/code.h"
#include "../compiler/compiler.h"
#include "../dyn_array/dyn_array.h"
#include "../object/object.h"

#define GLOBALS_SIZE 65536
#define STACK_SIZE 2048

typedef struct {
  DynamicArray constants; // Object*[]
  Instructions instructions;
  Object *stack[STACK_SIZE];
  size_t sp; // points to the next value
  Object *globals[GLOBALS_SIZE];
} VM;

typedef enum {
  VM_OK,
  VM_STACK_OVERFLOW,
  VM_UNSUPPORTED_OPERATION,
  VM_UNSUPPORTED_TYPE_FOR_OPERATION,
} VMResult;

VM *new_vm(Bytecode);
VM *new_vm_with_state(Bytecode, Object *[GLOBALS_SIZE]);

void free_vm(VM *);
VMResult run_vm(VM *);
void vm_error(VMResult, char *, size_t);
Object *vm_last_popped_stack_elem(VM *);

Object *stack_top(VM *);

#endif // VM_H
