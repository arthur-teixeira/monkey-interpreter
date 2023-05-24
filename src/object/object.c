#include "./object.h"
#include "../str_utils/str_utils.h"
#include <assert.h>
#include <libiberty/libiberty.h>
#include <stdio.h>

const char *ObjectTypeString[] = {
    "INTEGER_OBJ",  "BOOLEAN_OBJ", "NULL_OBJ",    "RETURN_OBJ", "ERROR_OBJ",
    "FUNCTION_OBJ", "STRING_OBJ",  "BUILTIN_OBJ", "ARRAY_OBJ",
};

void inspect_integer_object(char *buf, Integer *obj) {
  sprintf(buf, "%ld", obj->value);
}

void inspect_boolean_object(char *buf, Boolean *obj) {
  if (obj->value) {
    sprintf(buf, "%s", "true");
  } else {
    sprintf(buf, "%s", "false");
  }
}

void inspect_return_object(char *buf, ReturnValue *obj) {
  return inspect_object(buf, obj->value);
}

void inspect_null_object(char *buf) { sprintf(buf, "null"); }

void inspect_error_object(char *buf, Error *obj) {
  // TODO: Add line and column
  // For that, we have to add the token location to the lexer
  sprintf(buf, "ERROR: %s\n", obj->message);
}

void inspect_function_object(char *buf, Function *obj) {
  buf += sprintf(buf, "fn (");

  Node *cur_node = obj->parameters->tail;
  int i = 0;
  while (cur_node != NULL) {
    Identifier *ident = cur_node->value;

    buf += sprintf(buf, "%s", ident->value);

    if (++i < obj->parameters->size) {
      buf += sprintf(buf, ", ");
    }

    cur_node = cur_node->next;
  }

  buf += sprintf(buf, ")");
}

void inspect_string_object(char *buf, String *str) {
  sprintf(buf, "%s", str->value);
}

void inspect_builtin(char *buf) { sprintf(buf, "builtin function"); }

void inspect_array_object(char *buf, Array *arr) {
  buf += sprintf(buf, "[");

  for (size_t i = 0; i < arr->elements.len; i++) {
    char temp_buf[255];
    inspect_object(temp_buf, arr->elements.arr[i]);
    buf += sprintf(buf, "%s", temp_buf);

    if (i < arr->elements.len - 1) {
      buf += sprintf(buf, ", ");
    }
  }

  buf += sprintf(buf, "]");
}

int hash_inspect_iter(void *buf, hashmap_element_t *pair) {
  HashPair *value = pair->data;

  inspect_object(buf, &value->key);
  buf += sprintf(buf, ": ");
  inspect_object(buf, &value->value);
  buf += sprintf(buf, ", ");

  return 0;
}

void inspect_hash_object(char *buf, Hash *hash) {
  buf += sprintf(buf, "{");
  hashmap_iterate_pairs(&hash->pairs, &hash_inspect_iter, buf);
  buf += sprintf(buf, "}");
}

void inspect_object(char *buf, Object *obj) {
  switch (obj->type) {
  case INTEGER_OBJ:
    return inspect_integer_object(buf, obj->object);
  case BOOLEAN_OBJ:
    return inspect_boolean_object(buf, obj->object);
  case RETURN_OBJ:
    return inspect_return_object(buf, obj->object);
  case NULL_OBJ:
    return inspect_null_object(buf);
  case ERROR_OBJ:
    return inspect_error_object(buf, obj->object);
  case FUNCTION_OBJ:
    return inspect_function_object(buf, obj->object);
  case STRING_OBJ:
    return inspect_string_object(buf, obj->object);
  case BUILTIN_OBJ:
    return inspect_builtin(buf);
  case ARRAY_OBJ:
    return inspect_array_object(buf, obj->object);
  case HASH_OBJ:
    return inspect_hash_object(buf, obj->object);
  }
}

HashKey get_string_hash_key(String *str) {
  HashKey result;
  result.type = STRING_OBJ;
  // TODO: implement own crc32 function or simply inline libiberty impl
  result.value = xcrc32((unsigned char *)str->value, strlen(str->value), 10);

  return result;
}

HashKey get_bool_hash_key(Boolean *boolean) {
  HashKey result;
  result.type = BOOLEAN_OBJ;
  result.value = boolean->value;

  return result;
}

HashKey get_int_hash_key(Integer *integer) {
  HashKey result;
  result.type = INTEGER_OBJ;
  result.value = integer->value;

  return result;
}

HashKey invalid_key() {
  HashKey result;
  result.type = -1;
  result.value = -1;

  return result;
}

HashKey get_hash_key(Object *obj) {
  switch (obj->type) {
  case STRING_OBJ:
    return get_string_hash_key(obj->object);
  case BOOLEAN_OBJ:
    return get_bool_hash_key(obj->object);
  case INTEGER_OBJ:
    return get_int_hash_key(obj->object);
  default:
    return invalid_key();
  }
}
