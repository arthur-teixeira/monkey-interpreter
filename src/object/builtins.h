#ifndef OBJECT_BUILTINS_H
#define OBJECT_BUILTINS_H

#include "object.h"

typedef struct {
  char *name;
  Builtin builtin;
} BuiltinDef;

const Builtin *get_builtin_by_name(char *);

// TODO: remove
Object *unsupported_arg_error(Object *, ObjectType, char *);
Object *check_args_len(const DynamicArray *, size_t);

#endif // OBJECT_BUILTINS_H
