#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../hashmap/hashmap.h"

typedef enum {
  SYMBOL_GLOBAL_SCOPE,
  SYMBOL_LOCAL_SCOPE,
  SYMBOL_BUILTIN_SCOPE,
  SYMBOL_FREE_SCOPE,
} SymbolScope;

typedef struct { 
  char *name;
  SymbolScope scope;
  size_t index;
} Symbol;

typedef struct symbol_table_s {
  hashmap_t store;
  size_t num_definitions;
  struct symbol_table_s *outer;
  size_t free_symbols_len;
  Symbol free_symbols[100];
} SymbolTable;

SymbolTable *new_symbol_table(void);
SymbolTable *new_enclosed_symbol_table(SymbolTable *);

const Symbol *symbol_define(SymbolTable *, char *);
const Symbol *symbol_resolve(SymbolTable *, char *);

void free_symbol_table(SymbolTable *);
const Symbol *symbol_define_builtin(SymbolTable *, size_t, char *);
#endif // SYMBOL_TABLE_H
