#include "symbol_table.h"
#include <assert.h>

SymbolScope GLOBAL_SCOPE = "GLOBAL";
SymbolScope LOCAL_SCOPE = "LOCAL";

SymbolTable *new_symbol_table(void) {
  SymbolTable *table = malloc(sizeof(SymbolTable));
  assert(table != NULL);

  hashmap_create(10, &table->store);
  table->num_definitions = 0;
  table->outer = NULL;

  return table;
}

void free_symbol_table(SymbolTable *table) {
  hashmap_destroy(&table->store);
  free(table);
}

const Symbol *symbol_define(SymbolTable *table, char *name) {
  Symbol *symbol = malloc(sizeof(Symbol));
  assert(symbol != NULL);
  symbol->name = name;
  symbol->index = table->num_definitions;
  if (!table->outer) {
    symbol->scope = GLOBAL_SCOPE;
  } else {
    symbol->scope = LOCAL_SCOPE;
  }

  hashmap_put(&table->store, name, strlen(name), symbol);

  table->num_definitions++;

  return symbol;
}

const Symbol *symbol_resolve(SymbolTable *table, char *name) {
  const Symbol *s = NULL;
  SymbolTable *t = table;

  while (!s && t) {
    s = hashmap_get(&t->store, name, strlen(name));
    t = t->outer;
  }

  return s;
}

SymbolTable *new_enclosed_symbol_table(SymbolTable *outer) {
  SymbolTable *table = new_symbol_table();
  table->outer = outer;

  return table;
}
