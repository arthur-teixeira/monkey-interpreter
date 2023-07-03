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
