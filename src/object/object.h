#include <stdbool.h>

typedef enum {
  INTEGER_OBJ,
  BOOLEAN_OBJ,
  NULL_OBJ,
} ObjectType;

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
} Null;
