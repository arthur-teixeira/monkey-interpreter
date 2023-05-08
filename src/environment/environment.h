#include "../hashmap/hashmap.h"

typedef struct {
  hashmap_t *store;
} Environment;

Environment *new_environment(void);

void *env_get(Environment *env, char *name);
void *env_set(Environment *env, char *name, void *value);
