#include "./evaluator.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void free_object(Object *obj) {
  free(obj->object);
  free(obj);
}

Object *eval_program(Program *program) {
  Object *result;

  Node *cur_node = program->statements->tail;
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

static Boolean bool_true = {
    true,
};

static Boolean bool_false = {
    false,
};

static Object obj_true = {
    BOOLEAN_OBJ,
    &bool_true,
};

static Object obj_false = {
    BOOLEAN_OBJ,
    &bool_false,
};

Object *new_boolean(BooleanLiteral *bol) {
  if (bol->value) {
    return &obj_true;
  }

  return &obj_false;
}

Object *native_bool_to_boolean_object(bool condition) {
  if (condition) {
    return &obj_true;
  }

  return &obj_false;
}

Object *inverted_boolean(Boolean *bol) {
  if (bol->value) {
    return &obj_false;
  }

  return &obj_true;
}

Object *cast_int_to_boolean(Integer *intt) {
  if (intt->value) {
    return &obj_false;
  }

  return &obj_true;
}

Object *eval_bang_operator_expression(Object *right) {
  switch (right->type) {
  case BOOLEAN_OBJ:
    return inverted_boolean(right->object);
  case NULL_OBJ:
    return &obj_true;
  case INTEGER_OBJ:
    return cast_int_to_boolean(right->object);
  default:
    return &obj_false;
  }
}

Object *eval_minus_operator_expression(Object *right) {
  assert(right->type == INTEGER_OBJ &&
         "should be an integer"); // TODO: should be a parser error as well

  Integer *intt = right->object;

  Object *new_int_obj = malloc(sizeof(Object));
  assert(new_int_obj != NULL && "error allocating memory for new integer obj");
  new_int_obj->type = INTEGER_OBJ;

  Integer *new_int = malloc(sizeof(Integer));
  assert(new_int != NULL && "error allocating memory for new integer");

  new_int->value = -intt->value;
  new_int_obj->object = new_int;

  return new_int_obj;
}

Object *eval_integer_boolean_operation(Integer *left, char *operator,
                                       Integer * right) {
  if (strcmp(operator, ">") == 0) {
    return native_bool_to_boolean_object(left->value > right->value);
  } else if (strcmp(operator, "<") == 0) {
    return native_bool_to_boolean_object(left->value < right->value);
  } else if (strcmp(operator, "!=") == 0) {
    return native_bool_to_boolean_object(left->value != right->value);
  } else if (strcmp(operator, "==") == 0) {
    return native_bool_to_boolean_object(left->value == right->value);
  } else {
    return NULL;
  }
}

Object *eval_integer_infix_expression(Integer *left, char *operator,
                                      Integer * right) {
  Object *obj = malloc(sizeof(Object));
  assert(obj != NULL && "error allocating memory for infix integer");
  obj->type = INTEGER_OBJ;

  Integer *evaluated = malloc(sizeof(Integer));
  assert(evaluated != NULL && "error allocating memory for evaluated integer");

  // TODO: add %, >=, <=
  if (strcmp(operator, "+") == 0) {
    evaluated->value = left->value + right->value;
  } else if (strcmp(operator, "-") == 0) {
    evaluated->value = left->value - right->value;
  } else if (strcmp(operator, "*") == 0) {
    evaluated->value = left->value * right->value;
  } else if (strcmp(operator, "/") == 0) {
    evaluated->value = left->value / right->value; // TODO: floating point
  } else {
    free(obj);
    free(evaluated);
    return eval_integer_boolean_operation(left, operator, right);
  }

  obj->object = evaluated;
  return obj;
}

Object *eval_prefix_expression(PrefixExpression *expr) {
  Object *right = eval_expression(expr->right);

  if (strcmp(expr->operator, "!") == 0) {
    return eval_bang_operator_expression(right);
  }

  if (strcmp(expr->operator, "-") == 0) {
    return eval_minus_operator_expression(right);
  }

  assert(0 && "unreachable");
}

Object *eval_infix_expression(InfixExpression *expr) {
  Object *right = eval_expression(expr->right);
  Object *left = eval_expression(expr->left);

  if (right->type == INTEGER_OBJ && left->type == INTEGER_OBJ) {
    return eval_integer_infix_expression(left->object, expr->operator,
                                         right->object);
  }

  // These work because we only have a single instance for the true and false
  // objects, so we can simply do a pointer comparison.
  if (strcmp(expr->operator, "==") == 0) { 
    return native_bool_to_boolean_object(right == left);
  } else if (strcmp(expr->operator, "!=") == 0) {
    return native_bool_to_boolean_object(right != left);
  }

  return NULL;
}

Object *eval_expression(Expression *expr) {
  switch (expr->type) {
  case INT_EXPR:
    return new_integer(expr->value);
  case BOOL_EXPR:
    return new_boolean(expr->value);
  case PREFIX_EXPR:
    return eval_prefix_expression(expr->value);
  case INFIX_EXPR:
    return eval_infix_expression(expr->value);
  default:
    assert(0 && "not implemented");
  }
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
