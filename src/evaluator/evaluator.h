#ifndef EVALUATOR_H
#define EVALUATOR_H
#include "../object/object.h"

Object *eval(Statement *stmt, Environment *env);
Object *eval_program(Program *, Environment *env);
Object *eval_expression(Expression *expr, Environment *env);

Object *new_error(char *);

void free_environment(Environment *env);
#endif
