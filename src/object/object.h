#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>
#include "../environment/environment.h"
#include "../parser/parser.h"

typedef enum {
  INTEGER_OBJ,
  BOOLEAN_OBJ,
  NULL_OBJ,
  RETURN_OBJ,
  ERROR_OBJ,
  FUNCTION_OBJ,
  STRING_OBJ,
  BUILTIN_OBJ,
  ARRAY_OBJ,
  HASH_OBJ,
} ObjectType;

extern const char *ObjectTypeString[];

typedef struct {
 ObjectType type; 
 void *object;
} Object;

void inspect_object(char *, Object *);

typedef struct {
  long value;  
} Integer;

typedef struct {
  bool value;
} Boolean;

typedef struct {
  Object *value;
} ReturnValue;

typedef struct {
  char *message;
} Error;

typedef struct {
  LinkedList *parameters;
  BlockStatement *body;
  Environment *env;
} Function;

typedef struct {
  char *value;
  uint32_t len;
} String;

typedef Object *(*BuiltinFunction)(LinkedList *args); //Takes Object*[]

typedef struct {
  BuiltinFunction fn;
} Builtin;

typedef struct {
  DynamicArray elements;
} Array;

typedef struct {
} Null;

typedef struct {
  ObjectType type;
  uint64_t value;
} HashKey;

HashKey get_hash_key(Object *);

typedef struct {
  Object key;
  Object value;
} HashPair;

typedef struct {
  hashmap_t pairs;
} Hash;

#endif
