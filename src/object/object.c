#include "./object.h"
#include <stdio.h>

const char *ObjectTypeString[] = {
  "INTEGER_OBJ",
  "BOOLEAN_OBJ",
  "NULL_OBJ",
  "RETURN_OBJ",
  "ERROR_OBJ",
};

void inspect_integer_object(char *buf, Integer *obj) {
  sprintf(buf, "%ld\n", obj->value);
}

void inspect_boolean_object(char *buf, Boolean *obj) {
  if (obj->value) {
    sprintf(buf, "%s\n", "true");
  } else {
    sprintf(buf, "%s\n", "false");
  }
}

void inspect_return_object(char *buf, ReturnValue *obj) {
  return inspect_object(buf, obj->value);
}

void inspect_null_object(char *buf) {
  sprintf(buf, "null");
}

void inspect_error_object(char *buf, Error *obj) {
  // TODO: Add line and column
  // For that, we have to add the token location to the lexer
  sprintf(buf, "ERROR: %s\n", obj->message);
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
  }
}
