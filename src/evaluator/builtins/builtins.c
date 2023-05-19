#include "./builtins.h"
#include "../../object/object.h"
#include "../evaluator.h"
#include <assert.h>

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
  if (args->size != 1) {

    char err_msg[255];
    sprintf(err_msg, "wrong number of arguments: Expected 1 got %ld",
            args->size);

    return new_error(err_msg);
  }
  assert(args->size == 1 && "implement arg count error");

  Object *obj = args->tail->value;

  switch (obj->type) {
  case STRING_OBJ:
    return make_result(((String *)obj->object)->len);
  case ARRAY_OBJ:
    return make_result(((Array *)obj->object)->elements.len);
  default:
    break;
  }

  char err_msg[255];
  sprintf(err_msg, "argument to 'len' not supported, got %s",
          ObjectTypeString[obj->type]);

  return new_error(err_msg);
}

void set_len_builtin(hashmap_t *builtins) {
  Object *len_builtin = malloc(sizeof(Object));
  assert(len_builtin != NULL);
  len_builtin->type = BUILTIN_OBJ;

  Builtin *len_obj = malloc(sizeof(Builtin));
  assert(len_obj != NULL);

  len_obj->fn = &len;

  len_builtin->object = len_obj;

  hashmap_put(builtins, "len", strlen("len"), len_builtin);
}

hashmap_t *get_builtins() {
  hashmap_t *builtins = malloc(sizeof(hashmap_t));
  hashmap_create(10, builtins);

  set_len_builtin(builtins);

  return builtins;
}
