#include "../object/object.h"
#include "../parser/parser.h"

Object *eval(Statement *);
Object *eval_program(Program *);
Object *eval_expression(Expression *);

void free_object(Object *);
