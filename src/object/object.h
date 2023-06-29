#ifndef OBJECT_H
#define OBJECT_H

#include "../environment/environment.h"
#include "../parser/parser.h"
#include <stdbool.h>

typedef enum {
  NUMBER_OBJ,
  BOOLEAN_OBJ,
  NULL_OBJ,
  RETURN_OBJ,
  ERROR_OBJ,
  FUNCTION_OBJ,
  STRING_OBJ,
  BUILTIN_OBJ,
  ARRAY_OBJ,
  HASH_OBJ,
  CONTINUE_OBJ,
  BREAK_OBJ,
} ObjectType;

extern const char *ObjectTypeString[];

// interface
typedef struct {
  ObjectType type;
} Object;

void inspect_object(ResizableBuffer *, Object *);

typedef struct {
  ObjectType type; // NUMBER_OBJ
  double value;
} Number;

typedef struct {
  ObjectType type; // BOOLEAN_OBJ
  bool value;
} Boolean;

typedef struct {
  ObjectType type; // RETURN_OBJ
  Object *value;
} ReturnValue;

typedef struct {
  ObjectType type; // ERROR_OBJ
  char *message;
} Error;

typedef struct {
  ObjectType type; // FUNCTION_OBJ
  DynamicArray parameters;
  BlockStatement *body;
  Environment *env;
} Function;

typedef struct {
  ObjectType type; // STRING_OBJ
  char *value;
  uint32_t len;
} String;

typedef Object *(*BuiltinFunction)(LinkedList *args); // Takes Object*[]

typedef struct {
  ObjectType type; // BUILTIN_OBJ
  BuiltinFunction fn;
} Builtin;

typedef struct {
  ObjectType type; // ARRAY_OBJ
  DynamicArray elements;
} Array;

typedef struct {
  ObjectType type; // NULL_OBJ
} Null;

int32_t get_hash_key(Object *);

typedef struct {
  Object *key;
  Object *value;
} HashPair;

typedef struct {
  ObjectType type; // HASH_OBJ
  hashmap_t pairs;
} Hash;

size_t sizeof_object(Object *);
#endif
