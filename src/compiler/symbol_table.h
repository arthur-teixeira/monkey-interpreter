#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../hashmap/hashmap.h"

typedef char *SymbolScope;

extern SymbolScope GLOBAL_SCOPE; 

typedef struct { 
  char *name;
  SymbolScope scope;
  size_t index;
} Symbol;

typedef struct {
  hashmap_t store;
  size_t num_definitions;
} SymbolTable;

SymbolTable *new_symbol_table(void);

const Symbol *symbol_define(SymbolTable *, char *);
const Symbol *symbol_resolve(SymbolTable *, char *);

void free_symbol_table(SymbolTable *);

#endif // SYMBOL_TABLE_H
