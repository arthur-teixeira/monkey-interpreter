#ifndef VM_H
#define VM_H
#include "../dyn_array/dyn_array.h"
#include "../object/object.h"
#include "../compiler/compiler.h"
#include "../code/code.h"

#define STACK_SIZE 2048

typedef struct {
    DynamicArray constants; //Object*[]
    Instructions instructions;
    Object* stack[STACK_SIZE];
    size_t sp; // points to the next value
} VM;

typedef enum {
    VM_OK,
    VM_STACK_OVERFLOW,
} VMResult;

VM *new_vm(Bytecode);
void free_vm(VM *);
VMResult run_vm(VM *);
void vm_error(VMResult, char *, size_t);

Object *stack_top(VM *);

#endif // VM_H
