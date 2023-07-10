#include "symbol_table.h"
#include <assert.h>

SymbolScope GLOBAL_SCOPE = "GLOBAL";


SymbolTable *new_symbol_table(void) {
  SymbolTable *table = malloc(sizeof(SymbolTable));
  assert(table != NULL);

  hashmap_create(10, &table->store);
  table->num_definitions = 0;

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
  symbol->scope = GLOBAL_SCOPE;
  hashmap_put(&table->store, name, strlen(name), symbol);

  table->num_definitions++;

  return symbol;
}

const Symbol *symbol_resolve(SymbolTable *table, char *name) {
  return hashmap_get(&table->store, name, strlen(name));
}
