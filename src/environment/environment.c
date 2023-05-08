#include "environment.h"
#include <assert.h>
#include <string.h>

//TODO: implement own hash map
Environment *new_environment() {
  hashmap_t *hashmap = malloc(sizeof(hashmap_t));
  assert(hashmap != NULL && "Error allocating memory for hashmap");
  hashmap_create(100, hashmap);

  Environment *env = malloc(sizeof(Environment));
  assert(env != NULL && "Error allocating memory for environment");

  env->store = hashmap;

  return env;
}

void *env_get(Environment *env, char *name) {
  return hashmap_get(env->store, name, strlen(name));
}

void *env_set(Environment *env, char *name, void *value) {
  hashmap_put(env->store, name, strlen(name),
              value); // TODO: handle error as runtime error
  return value;
}

void free_environment(Environment *env) {
  hashmap_destroy(env->store);
  free(env);
}
