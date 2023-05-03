#include "./object.h"
#include <stdio.h>

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

void inspect_null_object(char *buf) {
  sprintf(buf, "null");
}

void inspect_object(char *buf, Object *obj) {
  switch (obj->type) {
  case INTEGER_OBJ:
    return inspect_integer_object(buf, obj->object);
  case BOOLEAN_OBJ:
    return inspect_boolean_object(buf, obj->object);
  case NULL_OBJ:
    return inspect_null_object(buf);
  }
}
