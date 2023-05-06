#include "../parser/parser.h"
#include "../environment/environment.h"

Object *eval(Statement *stmt, Environment *env);
Object *eval_program(Program *, Environment *env);
Object *eval_expression(Expression *expr, Environment *env);
void free_environment(Environment *env);

void free_object(Object *);
