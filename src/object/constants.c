#include "object.h"

Boolean obj_true = {
    .type = BOOLEAN_OBJ,
    .value = true,
};

Boolean obj_false = {
    .type = BOOLEAN_OBJ,
    .value = false,
};

Null obj_null = {
    .type = NULL_OBJ,
};

Object *native_bool_to_boolean_object(bool condition) {
  if (condition) {
    return (Object *)&obj_true;
  }

  return (Object *)&obj_false;
}
