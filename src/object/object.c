#include "./object.h"
#include "../crc/crc.h"
#include "../str_utils/str_utils.h"
#include <assert.h>
#include <stdio.h>

const char *ObjectTypeString[] = {
    "NUMBER_OBJ", "BOOLEAN_OBJ",  "NULL_OBJ",     "RETURN_OBJ",
    "ERROR_OBJ",  "FUNCTION_OBJ", "STRING_OBJ",   "BUILTIN_OBJ",
    "ARRAY_OBJ",  "HASH_OBJ",     "CONTINUE_OBJ", "BREAK_OBJ",
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

  for (uint32_t i = 0; i < obj->parameters.len; i++) {
    Identifier *ident = obj->parameters.arr[i];
    append_to_buf(buf, ident->value);
    if (++i < obj->parameters.len - 1) {
      append_to_buf(buf, ", ");
    }
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

void inspect_compiled_function_object(ResizableBuffer *buf,
                                      CompiledFunction *fn) {
  append_to_buf(buf, "CompiledFunction");
}

void inspect_closure(ResizableBuffer *buf, Closure *closure) {
  append_to_buf(buf, "Closure[");
  char p[16];
  sprintf(p, "%p", closure);
  append_to_buf(buf, p);
  append_to_buf(buf, "]");
}

void inspect_compiled_loop(ResizableBuffer *buf, CompiledLoop *loop) {
  append_to_buf(buf, "CompiledLoop");
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
  case COMPILED_FUNCTION_OBJ:
    return inspect_compiled_function_object(buf, (CompiledFunction *)obj);
  case CLOSURE_OBJ:
    return inspect_closure(buf, (Closure *)obj);
  case COMPILED_LOOP_OBJ:
    return inspect_compiled_loop(buf, (CompiledLoop *)obj);
  case CONTINUE_OBJ:
  case BREAK_OBJ:
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

size_t sizeof_object(Object *obj) {
  switch (obj->type) {
  case STRING_OBJ:
    return sizeof(String);
  case BOOLEAN_OBJ:
    return sizeof(Boolean);
  case NUMBER_OBJ:
    return sizeof(Number);
  case RETURN_OBJ:
    return sizeof(ReturnValue);
  case NULL_OBJ:
    return sizeof(Null);
  case ERROR_OBJ:
    return sizeof(Error);
  case FUNCTION_OBJ:
    return sizeof(Function);
  case BUILTIN_OBJ:
    return sizeof(Builtin);
  case ARRAY_OBJ:
    return sizeof(Array);
  case HASH_OBJ:
    return sizeof(Hash);
  case COMPILED_FUNCTION_OBJ:
    return sizeof(CompiledFunction);
  case CONTINUE_OBJ:
  case BREAK_OBJ:
    return sizeof(Object);
  case CLOSURE_OBJ:
    return sizeof(Closure);
  case COMPILED_LOOP_OBJ:
    return sizeof(CompiledLoop);
  }

  assert(0 && "unknown object type");
}

Object *new_number(double value) {
  Number *int_obj = malloc(sizeof(Number));
  assert(int_obj != NULL && "Error allocating memory for integer");

  int_obj->value = value;
  int_obj->type = NUMBER_OBJ;

  return (Object *)int_obj;
}

Object *new_string(char *value) {
  String *str = malloc(sizeof(String));
  assert(str != NULL && "error allocating memory for string");

  str->type = STRING_OBJ;
  str->len = strlen(value);
  str->value = strdup(value);

  return (Object *)str;
}

Object *new_concatted_string(String *left, String *right) {
  String *new_string = malloc(sizeof(String));
  assert(new_string != NULL);

  new_string->type = STRING_OBJ;
  new_string->len = left->len + right->len;

  new_string->value = malloc(new_string->len + 1);

  strlcpy(new_string->value, left->value, new_string->len + 1);
  strncat(new_string->value, right->value, new_string->len + 1);

  return (Object *)new_string;
}

Object *new_compiled_function(Instructions *instructions, size_t num_locals,
                              size_t num_parameters) {
  CompiledFunction *fn = malloc(sizeof(CompiledFunction));
  assert(fn != NULL);
  fn->type = COMPILED_FUNCTION_OBJ;
  fn->num_locals = num_locals;
  fn->num_parameters = num_parameters;

  fn->instructions = *instructions;

  return (Object *)fn;
}

Object *new_concatted_compiled_function(Instructions *instructions,
                                        size_t instructions_count) {
  CompiledFunction *fn = malloc(sizeof(CompiledFunction));
  assert(fn != NULL);

  fn->instructions = concat_instructions(instructions_count, instructions);

  return (Object *)fn;
}

Object *new_compiled_loop(Instructions *instructions, size_t num_locals) {
  CompiledLoop *loop = malloc(sizeof(CompiledLoop));
  assert(loop != NULL);

  loop->type = COMPILED_LOOP_OBJ;
  loop->num_locals = num_locals;
  loop->instructions = *instructions;

  return (Object *)loop;
}

Object *new_concatted_compiled_loop(Instructions *instructions,
                                    size_t instructions_count,
                                    size_t num_locals) {
  CompiledLoop *loop = malloc(sizeof(CompiledLoop));
  assert(loop != NULL);

  loop->instructions = concat_instructions(instructions_count, instructions);
  loop->num_locals = num_locals;

  return (Object *)loop;
}

void free_object(Object *obj) {
  if (obj->type != BOOLEAN_OBJ && obj->type != BUILTIN_OBJ &&
      obj->type != NULL_OBJ) {
    free(obj);
  }
}

Object *new_error(char *message) {
  Error *err = malloc(sizeof(Error));
  assert(err != NULL);
  err->message = message;
  err->type = ERROR_OBJ;

  return (Object *)err;
}

Object *new_array(Object **arr, size_t len) {
  Array *array = malloc(sizeof(Array));
  assert(array != NULL);

  array->type = ARRAY_OBJ;
  array_init(&array->elements, len);
  for (size_t i = 0; i < len; i++) {
    array_append(&array->elements, arr[i]);
  }

  return (Object *)array;
}

Object *new_closure(Object *fn) {
  Closure *closure = malloc(sizeof(Closure));
  closure->type = CLOSURE_OBJ;

  closure->enclosed = fn;

  return (Object *)closure;
}
