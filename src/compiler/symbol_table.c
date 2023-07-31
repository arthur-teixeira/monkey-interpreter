#include "symbol_table.h"
#include <assert.h>
#include <string.h>

SymbolTable *new_symbol_table(void) {
  SymbolTable *table = malloc(sizeof(SymbolTable));
  assert(table != NULL);

  hashmap_create(10, &table->store);
  table->num_definitions = 0;
  table->outer = NULL;
  table->free_symbols_len = 0;
  memset(table->free_symbols, 0, sizeof(table->free_symbols));

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
    symbol->scope = SYMBOL_GLOBAL_SCOPE;
  } else {
    symbol->scope = SYMBOL_LOCAL_SCOPE;
  }

  hashmap_put(&table->store, name, strlen(name), symbol);

  table->num_definitions++;

  return symbol;
}

const Symbol *define_free(SymbolTable *table, Symbol original) {
  table->free_symbols[table->free_symbols_len++] = original;

  Symbol *new_symbol = malloc(sizeof(Symbol));
  assert(new_symbol != NULL);
  *new_symbol = (Symbol){
      .name = original.name,
      .index = table->free_symbols_len - 1,
      .scope = SYMBOL_FREE_SCOPE,
  };

  hashmap_put(&table->store, original.name, strlen(original.name), new_symbol);
  return new_symbol;
}

const Symbol *symbol_resolve(SymbolTable *table, char *name) {
  const Symbol *s = hashmap_get(&table->store, name, strlen(name));
  if (!s && table->outer) {
    s = symbol_resolve(table->outer, name);
    if (!s) {
      return NULL;
    }

    if (s->scope == SYMBOL_GLOBAL_SCOPE || s->scope == SYMBOL_BUILTIN_SCOPE) {
      return s;
    }

    return define_free(table, *s);
  }

  return s;
}

SymbolTable *new_enclosed_symbol_table(SymbolTable *outer) {
  SymbolTable *table = new_symbol_table();
  table->outer = outer;

  return table;
}

const Symbol *symbol_define_builtin(SymbolTable *table, size_t index,
                                    char *name) {
  Symbol *symbol = malloc(sizeof(Symbol));
  assert(symbol != NULL);

  symbol->name = name;
  symbol->index = index;
  symbol->scope = SYMBOL_BUILTIN_SCOPE;

  hashmap_put(&table->store, name, strlen(name), symbol);

  return symbol;
}
