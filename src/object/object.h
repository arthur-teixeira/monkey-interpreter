#ifndef OBJECT_H
#define OBJECT_H

#include "../code/code.h"
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
  COMPILED_FUNCTION_OBJ,
  CLOSURE_OBJ,
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

typedef Object *(*BuiltinFunction)(DynamicArray args); // Takes Object*[]

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

typedef struct {
  ObjectType type; // COMPILED_FUNCTION_OBJ
  Instructions instructions;
  size_t num_locals;
  size_t num_parameters;
} CompiledFunction;

typedef struct {
  ObjectType type; // CLOSURE_OBJ
  CompiledFunction *fn;
  Object *free_variables[100];
  size_t num_free_variables;
} Closure;

void free_object(Object *);

Object *new_compiled_function(Instructions *, size_t, size_t);
Object *new_concatted_compiled_function(Instructions *, size_t);

size_t sizeof_object(Object *);

Object *new_number(double);
Object *new_string(char *);
Object *new_concatted_string(String *, String *);

Object *new_error(char *);
Object *new_array(Object **, size_t);
Object *new_closure(CompiledFunction *);
#endif
