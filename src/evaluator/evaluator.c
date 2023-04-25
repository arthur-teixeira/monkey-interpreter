#include "./evaluator.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

void free_object(Object *obj) {
  free(obj->object);
  free(obj);
}

Object *eval_statements(LinkedList *statements) {
  Object *result;

  Node *cur_node = statements->tail;
  while (cur_node != NULL) {
    result = eval(cur_node->value);
    cur_node = cur_node->next;
  }

  return result;
}

Object *new_integer(IntegerLiteral *lit) {
  Object *obj = malloc(sizeof(Object));
  assert(obj != NULL && "Error allocating memory for integer object");

  obj->type = INTEGER_OBJ;

  Integer *int_obj = malloc(sizeof(Integer));
  assert(int_obj != NULL && "Error allocating memory for integer");
  
  int_obj->value = lit->value;
  obj->object = int_obj;

  return obj;
}

Object *eval_expression(Expression *expr) {
  switch (expr->type) {
    case INT_EXPR:
      return new_integer(expr->value);
    default:
      assert(0 && "not implemented");
  }
}

Object *eval_program(Program *p) {
  return eval_statements(p->statements);
}

Object *eval(Statement *stmt) {
  switch (stmt->type) {
    case EXPR_STATEMENT:
      return eval_expression(stmt->expression);
    case RETURN_STATEMENT:
      assert(0 && "not implemented");
      return NULL;
    case LET_STATEMENT:
      assert(0 && "not implemented");
      return NULL;
    default:
      assert(0 && "unreachable");
      return NULL;
  }
}

