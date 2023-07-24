#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../hashmap/hashmap.h"

typedef char *SymbolScope;

extern SymbolScope GLOBAL_SCOPE; 
extern SymbolScope LOCAL_SCOPE;

typedef struct { 
  char *name;
  SymbolScope scope;
  size_t index;
} Symbol;

typedef struct symbol_table_s {
  hashmap_t store;
  size_t num_definitions;
  struct symbol_table_s *outer;
} SymbolTable;

SymbolTable *new_symbol_table(void);
SymbolTable *new_enclosed_symbol_table(SymbolTable *);

const Symbol *symbol_define(SymbolTable *, char *);
const Symbol *symbol_resolve(SymbolTable *, char *);

void free_symbol_table(SymbolTable *);

#endif // SYMBOL_TABLE_H
