#include "./builtins.h"
#include "../../dyn_array/dyn_array.h"
#include "../../object/object.h"
#include "../evaluator.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static Null null_value = {};

static Object obj_null = {
    NULL_OBJ,
    &null_value,
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

Object *make_result(long value) {
  Object *result = malloc(sizeof(Object));
  assert(result != NULL);

  result->type = INTEGER_OBJ;

  Integer *intt = malloc(sizeof(Integer));
  assert(intt != NULL);
  intt->value = value;

  result->object = intt;

  return result;
}

Object *len(LinkedList *args) {
  Object *err = check_args_len(args, 1);
  if (err != NULL) {
    return err;
  }

  Object *obj = args->tail->value;

  switch (obj->type) {
  case STRING_OBJ:
    return make_result(((String *)obj->object)->len);
  case ARRAY_OBJ:
    return make_result(((Array *)obj->object)->elements.len);
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

  Array *arr = arg->object;

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

  Array *arr = arg->object;
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

  Object *new_arr_obj = malloc(sizeof(Object));
  assert(new_arr_obj != NULL && "Error allocating memory for new array");

  new_arr_obj->type = ARRAY_OBJ;

  Array *new_arr = malloc(sizeof(Array));
  assert(new_arr != NULL && "Error allocating memory for new array");

  Array *old_arr = arg->object;

  array_init(&new_arr->elements, old_arr->elements.len - 1);

  for (size_t i = 1; i < old_arr->elements.len; i++) {
    array_append(&new_arr->elements, old_arr->elements.arr[i]);
  }

  new_arr_obj->object = new_arr;

  return new_arr_obj;
}

Object *push(LinkedList *args) {
  Object *err = check_args_len(args, 2);
  if (err != NULL) {
    return err;
  }

  Object *old_arr_obj = args->tail->value;
  Object *new_element = malloc(sizeof(Object));
  memcpy(new_element, args->tail->next->value, sizeof(Object));

  err = unsupported_arg_error(old_arr_obj, ARRAY_OBJ, "push");
  if (err != NULL) {
    return err;
  }

  Array *old_arr = old_arr_obj->object;

  Object *new_arr_obj = malloc(sizeof(Object));
  assert(new_arr_obj != NULL && "Error allocating memory for new array");

  new_arr_obj->type = ARRAY_OBJ;

  Array *new_arr = malloc(sizeof(Array));
  assert(new_arr != NULL && "Error allocating memory for new array");
  array_init(&new_arr->elements, old_arr->elements.len + 1);

  memcpy(&new_arr->elements, &old_arr->elements, sizeof(old_arr->elements));

  array_append(&new_arr->elements, new_element);

  new_arr_obj->object = new_arr;

  return new_arr_obj;
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
    Object *value = cur_node->value;
    ResizableBuffer buf;
    init_resizable_buffer(&buf, 100);

    inspect_object(&buf, value);
    printf("%s\n", buf.buf);

    cur_node = cur_node->next;
    free(buf.buf);
  }

  Object *result = malloc(sizeof(Object));
  result->type = NULL_OBJ;
  result->object = NULL;
  
  return result;
}

void put_builtin(hashmap_t *builtins, char *fn_name, BuiltinFunction fn) {
  Object *builtin_obj = malloc(sizeof(Object));
  assert(builtin_obj != NULL);
  builtin_obj->type = BUILTIN_OBJ;

  Builtin *builtin = malloc(sizeof(Builtin));
  assert(builtin != NULL);

  builtin->fn = fn;
  builtin_obj->object = builtin;

  hashmap_put(builtins, fn_name, strlen(fn_name), builtin_obj);
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
}
