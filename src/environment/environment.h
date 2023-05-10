#include "../hashmap/hashmap.h"

struct Environment {
  hashmap_t *store;
  struct Environment *outer;
};

typedef struct Environment Environment;

Environment *new_environment(void);

Environment *new_enclosed_environment(Environment *outer);

void *env_get(Environment *env, char *name);
void *env_set(Environment *env, char *name, void *value);
