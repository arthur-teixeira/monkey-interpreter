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
} Null;
