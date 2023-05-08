#include "./evaluator.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_object(Object *obj) {
  if (obj->type != BOOLEAN_OBJ) {
    free(obj->object);
    free(obj);
  }
}

Object *new_error(char *message) {
  Object *error_obj = malloc(sizeof(Object));
  assert(error_obj != NULL && "Error allocating memory for error object");
  error_obj->type = ERROR_OBJ;

  Error *err = malloc(sizeof(Error));
  assert(err != NULL && "Error allocating memory for error");
  err->message = strdup(message);
  error_obj->object = err;

  return error_obj;
}

bool is_error(Object *obj) {
  if (obj != NULL) {
    return obj->type == ERROR_OBJ;
  }

  return false;
}

Object *eval_block_statement(LinkedList *statements, Environment *env) {
  Object *result;

  Node *cur_node = statements->tail;
  while (cur_node != NULL) {
    result = eval(cur_node->value, env);

    if (result != NULL 
        && (result->type == RETURN_OBJ
        || result->type == ERROR_OBJ)
    ) {
      return result;
    }
    cur_node = cur_node->next;
  }

  return result;
}

Object *eval_program(Program *program, Environment *env) {
  Object *result;

  Node *cur_node = program->statements->tail;
  while (cur_node != NULL) {
    result = eval(cur_node->value, env);

    if (result != NULL && result->type == RETURN_OBJ) {
      ReturnValue *ret = result->object;
      Object *ret_value = ret->value;
      free_object(result);
      return ret_value;
    }

    if (result != NULL && result->type == ERROR_OBJ) {
      return result;
    }

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
  if (right->type != INTEGER_OBJ) {
    char error_message[255];
    sprintf(error_message, "unknown operator: -%s",
            ObjectTypeString[right->type]);

    return new_error(error_message);
  };

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

Object *eval_integer_boolean_operation(Object *left_obj, char *operator,
                                       Object * right_obj) {

  Integer *left = left_obj->object;
  Integer *right = right_obj->object;

  if (strcmp(operator, ">") == 0) {
    return native_bool_to_boolean_object(left->value > right->value);
  } else if (strcmp(operator, "<") == 0) {
    return native_bool_to_boolean_object(left->value < right->value);
  } else if (strcmp(operator, "!=") == 0) {
    return native_bool_to_boolean_object(left->value != right->value);
  } else if (strcmp(operator, "==") == 0) {
    return native_bool_to_boolean_object(left->value == right->value);
  }

  char error_message[255];

  sprintf(error_message, "unknown operator: %s %s %s",
          ObjectTypeString[left_obj->type], operator,
          ObjectTypeString[right_obj->type]);

  return new_error(error_message);
}

Object *eval_integer_infix_expression(Object *left_obj, char *operator,
                                      Object * right_obj) {
  Object *obj = malloc(sizeof(Object));
  assert(obj != NULL && "error allocating memory for infix integer");
  obj->type = INTEGER_OBJ;

  Integer *evaluated = malloc(sizeof(Integer));
  assert(evaluated != NULL && "error allocating memory for evaluated integer");

  Integer *left = left_obj->object;
  Integer *right = right_obj->object;

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
    return eval_integer_boolean_operation(left_obj, operator, right_obj);
  }

  obj->object = evaluated;
  return obj;
}

Object *eval_prefix_expression(PrefixExpression *expr, Environment *env) {
  Object *right = eval_expression(expr->right, env);
  if (is_error(right)) {
    return right;
  }

  if (strcmp(expr->operator, "!") == 0) {
    return eval_bang_operator_expression(right);
  }

  if (strcmp(expr->operator, "-") == 0) {
    return eval_minus_operator_expression(right);
  }

  char error_message[255];
  sprintf(error_message, "unknown operator: %s%s", expr->operator,
          ObjectTypeString[right->type]);

  return new_error(error_message);
}

Object *eval_infix_expression(InfixExpression *expr, Environment *env) {
  Object *right = eval_expression(expr->right, env);
  Object *left = eval_expression(expr->left, env);

  if (is_error(left)) {
    return left;
  }

  if (is_error(right)) {
    return right;
  }

  if (right->type == INTEGER_OBJ && left->type == INTEGER_OBJ) {
    return eval_integer_infix_expression(left, expr->operator, right);
  }

  // These work because we only have a single instance for the true and false
  // objects, so we can simply do a pointer comparison.
  if (strcmp(expr->operator, "==") == 0) {
    assert(right->type == BOOLEAN_OBJ && left->type == BOOLEAN_OBJ);
    return native_bool_to_boolean_object(right == left);
  } else if (strcmp(expr->operator, "!=") == 0) {
    assert(right->type == BOOLEAN_OBJ && left->type == BOOLEAN_OBJ);
    return native_bool_to_boolean_object(right != left);
  }

  char error_message[255];
  if (right->type != left->type) {
    sprintf(error_message, "type mismatch: %s %s %s",
            ObjectTypeString[left->type], expr->operator,
            ObjectTypeString[right->type]);

  } else {
    sprintf(error_message, "unknown operator: %s %s %s",
            ObjectTypeString[left->type], expr->operator,
            ObjectTypeString[right->type]);
  }

  return new_error(error_message);
}

bool is_truthy(Object *obj) {
  if (obj == NULL || obj == &obj_false) {
    return false;
  }

  return true;
}

Object *eval_if_expression(IfExpression *expr, Environment *env) {
  Object *condition = eval_expression(expr->condition, env);

  if (is_error(condition)) {
    return condition;
  }

  if (is_truthy(condition)) {
    return eval_block_statement(expr->consequence->statements, env);
  } else if (expr->alternative != NULL) {
    return eval_block_statement(expr->alternative->statements, env);
  }

  return NULL;
}

Object *eval_identifier(Identifier *ident, Environment *env) {
  Object *val = env_get(env, ident->value);
  if (val == NULL) {
    char error_message[255];
    sprintf(error_message, "undeclared identifier '%s'", ident->value);
    return new_error(error_message);
  }

  return val;
}

Object *eval_expression(Expression *expr, Environment *env) {
  switch (expr->type) {
  case INT_EXPR:
    return new_integer(expr->value);
  case BOOL_EXPR:
    return new_boolean(expr->value);
  case PREFIX_EXPR:
    return eval_prefix_expression(expr->value, env);
  case INFIX_EXPR:
    return eval_infix_expression(expr->value, env);
  case IF_EXPR:
    return eval_if_expression(expr->value, env);
  case IDENT_EXPR: 
    return eval_identifier(expr->value, env);
  default:
    assert(0 && "not implemented");
  }
}

Object *eval_return_statement(Expression *expr, Environment *env) {
  Object *ret_obj = malloc(sizeof(Object));
  assert(ret_obj != NULL && "error allocating memory for return object");
  ret_obj->type = RETURN_OBJ;

  ReturnValue *ret = malloc(sizeof(ReturnValue));
  assert(ret != NULL && "error allocating memory for return value");

  ret->value = eval_expression(expr, env);

  ret_obj->object = ret;

  if (is_error(ret->value)) {
    Object *err = ret->value;
    free(ret_obj);

    return err;
  }
  return ret_obj;
}

Object *eval_let_statement(Statement *stmt, Environment *env) {
  Object *val = eval_expression(stmt->expression, env);
  if (is_error(val)) {
    return val;
  }
  env_set(env, stmt->name->value, val);

  return NULL;
}

Object *eval(Statement *stmt, Environment *env) {
  switch (stmt->type) {
  case EXPR_STATEMENT:
    return eval_expression(stmt->expression, env);
  case RETURN_STATEMENT:
    return eval_return_statement(stmt->expression, env);
  case LET_STATEMENT:
    return eval_let_statement(stmt, env);
  default:
    assert(0 && "unreachable");
    return NULL;
  }
}
