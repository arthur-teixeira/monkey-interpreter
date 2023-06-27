#include "./builtins.h"
#include "../../dyn_array/dyn_array.h"
#include "../../object/object.h"
#include "../evaluator.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static Object obj_null = {
    .type = NULL_OBJ,
};

Object *check_args_len(LinkedList *args, size_t expected) {
  if (args->size != expected) {

    char err_msg[255];
    sprintf(err_msg, "wrong number of arguments: Expected 1 got %ld",
            args->size);

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

Object *make_result(double value) {
  Number *result = malloc(sizeof(Number));
  assert(result != NULL);
  result->type = NUMBER_OBJ;
  result->value = value;

  return (Object *)result;
}

Object *len(LinkedList *args) {
  Object *err = check_args_len(args, 1);
  if (err != NULL) {
    return err;
  }

  Object *obj = args->tail->value;

  switch (obj->type) {
  case STRING_OBJ:
    return make_result(((String *)obj)->len);
  case ARRAY_OBJ:
    return make_result(((Array *)obj)->elements.len);
  default:
    break;
  }

  return unsupported_arg_error(obj, -1, "len");
}

Object *first(LinkedList *args) {
  Object *err = check_args_len(args, 1);
  if (err != NULL) {
    return err;
  }

  Object *arg = args->tail->value;

  err = unsupported_arg_error(arg, ARRAY_OBJ, "first");
  if (err != NULL) {
    return err;
  }

  Array *arr = (Array *)arg;

  if (arr->elements.len < 1) {
    return &obj_null;
  }

  return arr->elements.arr[0];
}

Object *last(LinkedList *args) {
  Object *err = check_args_len(args, 1);
  if (err != NULL) {
    return err;
  }

  Object *arg = args->tail->value;

  err = unsupported_arg_error(arg, ARRAY_OBJ, "last");
  if (err != NULL) {
    return err;
  }

  Array *arr = (Array *)arg;
  if (arr->elements.len < 1) {
    return &obj_null;
  }

  return arr->elements.arr[arr->elements.len - 1];
}

Object *rest(LinkedList *args) {
  Object *err = check_args_len(args, 1);
  if (err != NULL) {
    return err;
  }

  Object *arg = args->tail->value;

  err = unsupported_arg_error(arg, ARRAY_OBJ, "rest");
  if (err != NULL) {
    return err;
  }


  Array *new_arr = malloc(sizeof(Array));
  assert(new_arr != NULL && "Error allocating memory for new array");

  Array *old_arr = (Array *)arg;

  array_init(&new_arr->elements, old_arr->elements.len - 1);

  for (size_t i = 1; i < old_arr->elements.len; i++) {
    array_append(&new_arr->elements, old_arr->elements.arr[i]);
  }

  new_arr->type = ARRAY_OBJ;

  return (Object *)new_arr;
}

typedef enum {
  APPEND_PUSH,
  APPEND_SHIFT,
} AppendType;

Object *builtin_append(LinkedList *args, AppendType type) {
  Object *err = check_args_len(args, 2);
  if (err != NULL) {
    return err;
  }

  Object *old_arr_obj = args->tail->value;
  size_t new_element_size = sizeof_object(args->tail->next->value);
  Object *new_element = malloc(new_element_size);
  memcpy(new_element, args->tail->next->value, new_element_size);

  err = unsupported_arg_error(old_arr_obj, ARRAY_OBJ, "push");
  if (err != NULL) {
    return err;
  }

  Array *old_arr = (Array*)args->tail->value;

  Array *new_arr = malloc(sizeof(Array));
  assert(new_arr != NULL);

  new_arr->type = ARRAY_OBJ;
  array_init(&new_arr->elements, old_arr->elements.len + 1);

  if (type == APPEND_SHIFT) {
    array_append(&new_arr->elements, args->tail->next->value);
  }

  for (size_t i = 0; i < old_arr->elements.len; i++) {
    array_append(&new_arr->elements, old_arr->elements.arr[i]);
  }

  if (type == APPEND_PUSH) {
    array_append(&new_arr->elements, args->tail->next->value);
  }

  return (Object *)new_arr;
}

Object *push(LinkedList *args) {
  return builtin_append(args, APPEND_PUSH);
}

Object *shift(LinkedList *args) {
  return builtin_append(args, APPEND_SHIFT);
}

Object *builtin_puts(LinkedList *args) {
  if (args->size == 0) {
    char err_msg[255];
    sprintf(err_msg, "wrong number of arguments: Expected 1 got %ld",
            args->size);

    return new_error(err_msg);
  }

  Node *cur_node = args->tail;
  while (cur_node != NULL) {
    Object *value = (Object *)cur_node->value;
    ResizableBuffer buf;
    init_resizable_buffer(&buf, 100);

    inspect_object(&buf, value);
    printf("%s\n", buf.buf);

    cur_node = cur_node->next;
    free(buf.buf);
  }

  Object *result = malloc(sizeof(Object));
  result->type = NULL_OBJ;
  
  return result;
}

void put_builtin(hashmap_t *builtins, char *fn_name, BuiltinFunction fn) {
  Builtin *builtin = malloc(sizeof(Builtin));
  assert(builtin != NULL);

  builtin->type = BUILTIN_OBJ;
  builtin->fn = fn;

  hashmap_put(builtins, fn_name, strlen(fn_name), builtin);
}

void set_len_builtin(hashmap_t *builtins) {
  put_builtin(builtins, "len", &len);
}

void set_first_builtin(hashmap_t *builtins) {
  put_builtin(builtins, "first", &first);
}

void set_last_builtin(hashmap_t *builtins) {
  put_builtin(builtins, "last", &last);
}

void set_rest_builtin(hashmap_t *builtins) {
  put_builtin(builtins, "rest", &rest);
}

void set_push_builtin(hashmap_t *builtins) {
  put_builtin(builtins, "push", &push);
}

void set_shift_builtin(hashmap_t *builtins) {
  put_builtin(builtins, "shift", &shift);
}

void set_puts_builtin(hashmap_t *builtins) {
  put_builtin(builtins, "puts", &builtin_puts);
}

void get_builtins(hashmap_t *builtins) {
  set_len_builtin(builtins);
  set_first_builtin(builtins);
  set_last_builtin(builtins);
  set_rest_builtin(builtins);
  set_push_builtin(builtins);
  set_puts_builtin(builtins);
  set_shift_builtin(builtins);
}
