#include "../hashmap/hashmap.h"
#include "../object/object.h"

typedef struct {
  hashmap_t *store;
} Environment;

Environment *new_environment(void);

Object *env_get(Environment *env, char *name);
Object *env_set(Environment *env, char *name, Object *value);
