#include "../object/object.h"
#include "../parser/parser.h"

Object *eval(Statement *);
Object *eval_program(Program *);

void free_object(Object *);
