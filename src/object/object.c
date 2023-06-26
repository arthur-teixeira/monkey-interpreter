#include "./object.h"
#include "../crc/crc.h"
#include "../str_utils/str_utils.h"
#include <assert.h>
#include <stdio.h>

const char *ObjectTypeString[] = {
    "INTEGER_OBJ", "BOOLEAN_OBJ",  "NULL_OBJ",     "RETURN_OBJ",
    "ERROR_OBJ",   "FUNCTION_OBJ", "STRING_OBJ",   "BUILTIN_OBJ",
    "ARRAY_OBJ",   "HASH_OBJ",     "CONTINUE_OBJ", "BREAK_OBJ",
};

void inspect_number_object(ResizableBuffer *buf, Number *obj) {
  char temp_buf[100];
  sprintf(temp_buf, "%d", (int)obj->value);

  append_to_buf(buf, temp_buf);
}

void inspect_boolean_object(ResizableBuffer *buf, Boolean *obj) {
  if (obj->value) {
    append_to_buf(buf, "true");
  } else {
    append_to_buf(buf, "false");
  }
}

void inspect_return_object(ResizableBuffer *buf, ReturnValue *obj) {
  return inspect_object(buf, obj->value);
}

void inspect_null_object(ResizableBuffer *buf) { append_to_buf(buf, "null"); }

void inspect_error_object(ResizableBuffer *buf, Error *obj) {
  // TODO: Add line and column
  // For that, we have to add the token location to the lexer
  sprintf(buf->buf, "ERROR: %s\n", obj->message);
}

void inspect_function_object(ResizableBuffer *buf, Function *obj) {
  append_to_buf(buf, "fn (");

  Node *cur_node = obj->parameters->tail;
  int i = 0;
  while (cur_node != NULL) {
    Identifier *ident = cur_node->value;

    append_to_buf(buf, ident->value);

    if (++i < obj->parameters->size) {
      append_to_buf(buf, ", ");
    }

    cur_node = cur_node->next;
  }

  append_to_buf(buf, ")");
}

void inspect_string_object(ResizableBuffer *buf, String *str) {
  append_to_buf(buf, str->value);
}

void inspect_builtin(ResizableBuffer *buf) {
  append_to_buf(buf, "builtin function");
}

void inspect_array_object(ResizableBuffer *buf, Array *arr) {
  append_to_buf(buf, "[");

  for (size_t i = 0; i < arr->elements.len; i++) {
    inspect_object(buf, arr->elements.arr[i]);

    if (i < arr->elements.len - 1) {
      append_to_buf(buf, ", ");
    }
  }
  append_to_buf(buf, "]");
}

int hash_inspect_iter(void *buf, hashmap_element_t *pair) {
  HashPair *value = pair->data;

  ResizableBuffer *char_buf = buf;

  inspect_object(char_buf, value->key);
  append_to_buf(char_buf, ": ");
  inspect_object(char_buf, value->value);
  append_to_buf(char_buf, ", ");

  return 0;
}

void inspect_hash_object(ResizableBuffer *buf, Hash *hash) {
  append_to_buf(buf, "{");
  hashmap_iterate_pairs(&hash->pairs, &hash_inspect_iter, buf);
  append_to_buf(buf, "}");
}

void inspect_object(ResizableBuffer *buf, Object *obj) {
  switch (obj->type) {
  case NUMBER_OBJ:
    return inspect_number_object(buf, (Number *)obj);
  case BOOLEAN_OBJ:
    return inspect_boolean_object(buf, (Boolean *)obj);
  case RETURN_OBJ:
    return inspect_return_object(buf, (ReturnValue *)obj);
  case NULL_OBJ:
    return inspect_null_object(buf);
  case ERROR_OBJ:
    return inspect_error_object(buf, (Error *)obj);
  case FUNCTION_OBJ:
    return inspect_function_object(buf, (Function *)obj);
  case STRING_OBJ:
    return inspect_string_object(buf, (String *)obj);
  case BUILTIN_OBJ:
    return inspect_builtin(buf);
  case ARRAY_OBJ:
    return inspect_array_object(buf, (Array *)obj);
  case HASH_OBJ:
    return inspect_hash_object(buf, (Hash *)obj);
  default:
    return; // break and continue object are sentinel values
  }
}

int32_t get_string_hash_key(String *str) {
  return crc32((unsigned char *)str->value, strlen(str->value), 10);
}

int32_t get_bool_hash_key(Boolean *boolean) {
  return boolean->value << BOOLEAN_OBJ;
}

int32_t get_int_hash_key(Number *integer) {
  return (long)integer->value << NUMBER_OBJ;
}

int32_t get_hash_key(Object *obj) {
  switch (obj->type) {
  case STRING_OBJ:
    return get_string_hash_key((String *)obj);
  case BOOLEAN_OBJ:
    return get_bool_hash_key((Boolean *)obj);
  case NUMBER_OBJ:
    return get_int_hash_key((Number *)obj);
  default:
    return -1;
  }
}
