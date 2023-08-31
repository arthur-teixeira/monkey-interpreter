#include "builtins.h"
#include "object.h"
#include <assert.h>
#include <string.h>

#define ARRAY_LEN(a) sizeof(a) / sizeof(a[0])

Object *check_args_len(const DynamicArray *args, size_t expected) {
  if (args->len != expected) {
    char err_msg[255];
    sprintf(err_msg, "wrong number of arguments: Expected %ld got %ld",
            expected, args->len);

    return new_error(err_msg);
  }

  return NULL;
}

Object *unsupported_arg_error(Object *obj, ObjectType type, char *fn_name) {
  if (type < 0 || obj->type != type) {
    char err_msg[255];
    sprintf(err_msg, "argument to '%s' not supported, got %s", fn_name,
            ObjectTypeString[obj->type]);

    return new_error(err_msg);
  }

  return NULL;
}

Object *len(DynamicArray args) {
  Object *err = check_args_len(&args, 1);
  if (err != NULL) {
    return err;
  }

  Object *obj = args.arr[0];

  switch (obj->type) {
  case STRING_OBJ:
    return new_number(((String *)obj)->len);
  case ARRAY_OBJ:
    return new_number(((Array *)obj)->elements.len);
  default:
    break;
  }

  return unsupported_arg_error(obj, -1, "len");
}

Object *builtin_puts(DynamicArray args) {
  if (args.len == 0) {
    char err_msg[255];
    sprintf(err_msg, "wrong number of arguments: Expected 1 got %ld", args.len);

    return new_error(err_msg);
  }

  for (uint32_t i = 0; i < args.len; i++) {
    Object *value = args.arr[i];
    ResizableBuffer buf;
    init_resizable_buffer(&buf, 100);

    inspect_object(&buf, value);
    printf("%s\n", buf.buf);

    free(buf.buf);
  }

  return new_null();
}

Object *first(DynamicArray args) {
  Object *err = check_args_len(&args, 1);
  if (err != NULL) {
    return err;
  }

  Object *arg = args.arr[0];

  err = unsupported_arg_error(arg, ARRAY_OBJ, "first");
  if (err != NULL) {
    return err;
  }

  Array *arr = (Array *)arg;

  if (arr->elements.len < 1) {
    return new_null();
  }

  return arr->elements.arr[0];
}

Object *last(DynamicArray args) {
  Object *err = check_args_len(&args, 1);
  if (err != NULL) {
    return err;
  }

  Object *arg = args.arr[0];

  err = unsupported_arg_error(arg, ARRAY_OBJ, "last");
  if (err != NULL) {
    return err;
  }

  Array *arr = (Array *)arg;
  if (arr->elements.len < 1) {
    return new_null();
  }

  return arr->elements.arr[arr->elements.len - 1];
}

typedef enum {
  APPEND_PUSH,
  APPEND_SHIFT,
} AppendType;

Object *builtin_append(const DynamicArray *args, AppendType type) {
  Object *err = check_args_len(args, 2);
  if (err != NULL) {
    return err;
  }

  Object *old_arr_obj = args->arr[0];
  size_t new_element_size = sizeof_object(args->arr[1]);
  Object *new_element = malloc(new_element_size);
  memcpy(new_element, args->arr[1], new_element_size);

  err = unsupported_arg_error(old_arr_obj, ARRAY_OBJ, "push");
  if (err != NULL) {
    return err;
  }

  Array *old_arr = (Array *)args->arr[0];

  Array *new_arr = malloc(sizeof(Array));
  assert(new_arr != NULL);

  new_arr->type = ARRAY_OBJ;
  array_init(&new_arr->elements, old_arr->elements.len + 1);

  if (type == APPEND_SHIFT) {
    array_append(&new_arr->elements, args->arr[1]);
  }

  for (size_t i = 0; i < old_arr->elements.len; i++) {
    array_append(&new_arr->elements, old_arr->elements.arr[i]);
  }

  if (type == APPEND_PUSH) {
    array_append(&new_arr->elements, args->arr[1]);
  }

  return (Object *)new_arr;
}

Object *push(DynamicArray args) { return builtin_append(&args, APPEND_PUSH); }

Object *shift(DynamicArray args) { return builtin_append(&args, APPEND_SHIFT); }

Object *rest(DynamicArray args) {
  Object *err = check_args_len(&args, 1);
  if (err != NULL) {
    return err;
  }

  Object *arg = args.arr[0];
  Array *old_arr = (Array *)arg;

  err = unsupported_arg_error(arg, ARRAY_OBJ, "rest");
  if (err != NULL) {
    return err;
  }

  if (old_arr->elements.len < 1) {
    return new_null();
  }

  Array *new_arr = malloc(sizeof(Array));
  assert(new_arr != NULL && "Error allocating memory for new array");

  array_init(&new_arr->elements, old_arr->elements.len - 1);

  for (size_t i = 1; i < old_arr->elements.len; i++) {
    array_append(&new_arr->elements, old_arr->elements.arr[i]);
  }

  new_arr->type = ARRAY_OBJ;

  return (Object *)new_arr;
}

const BuiltinDef builtin_definitions[] = {
    {
        .name = "len",
        .builtin =
            (Builtin){
                .type = BUILTIN_OBJ,
                .fn = len,
            },
    },
    {
        .name = "puts",
        .builtin =
            (Builtin){
                .type = BUILTIN_OBJ,
                .fn = builtin_puts,
            },
    },
    {
        .name = "first",
        .builtin =
            (Builtin){
                .type = BUILTIN_OBJ,
                .fn = first,
            },
    },
    {
        .name = "last",
        .builtin =
            (Builtin){
                .type = BUILTIN_OBJ,
                .fn = last,
            },
    },
    {
        .name = "push",
        .builtin =
            (Builtin){
                .type = BUILTIN_OBJ,
                .fn = push,
            },
    },
    {
        .name = "shift",
        .builtin =
            (Builtin){
                .type = BUILTIN_OBJ,
                .fn = shift,
            },
    },
    {
        .name = "rest",
        .builtin =
            (Builtin){
                .type = BUILTIN_OBJ,
                .fn = rest,
            },
    },
};
const size_t builtin_definitions_len = ARRAY_LEN(builtin_definitions);

const Builtin *get_builtin_by_name(char *name) {
  for (size_t i = 0; i < ARRAY_LEN(builtin_definitions); i++) {
    if (strcmp(name, builtin_definitions[i].name) == 0) {
      return &builtin_definitions[i].builtin;
    }
  }

  return NULL;
}
