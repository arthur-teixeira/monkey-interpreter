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

Object *eval_function_literal(FunctionLiteral *lit, Environment *env) {
  Object *fn_obj = malloc(sizeof(Object));
  fn_obj->type = FUNCTION_OBJ;

  Function *fn = malloc(sizeof(Function));
  fn->env = env;
  
  // should I memcpy this or simply use the same pointers?
  fn->body = lit->body;
  fn->parameters = lit->parameters;

  fn_obj->object = fn;

  return fn_obj;
}

LinkedList *eval_expressions(LinkedList *expressions, Environment *env) {
  LinkedList *evaluated = new_list();

  Node *cur_expression_node = expressions->tail;
  while(cur_expression_node != NULL) {
    Expression *cur_expression = cur_expression_node->value;
    Object *evaluated_expression = eval_expression(cur_expression, env);

    if (is_error(evaluated_expression)) {
      free_list(evaluated);

      LinkedList *list_with_error = new_list();
      append(list_with_error, evaluated);
      return list_with_error;
    }

    append(evaluated, evaluated_expression);

    cur_expression_node = cur_expression_node->next;
  }

  return evaluated;
}

Environment *extend_function_env(Function *fn, LinkedList *args) {
  assert(args->size == fn->parameters->size);

  Environment *env = new_enclosed_environment(fn->env);
  Node *cur_argument_node = args->tail; // Object *
  Node *cur_parameter_node = fn->parameters->tail; //Expression *

  while (cur_parameter_node != NULL  && cur_parameter_node != NULL) {
    Expression *cur_parameter = cur_parameter_node->value;
    printf("type is %d\n", cur_parameter->type);
    assert(cur_parameter->type == IDENT_EXPR);
    Identifier *parameter_ident = cur_parameter->value;

    env_set(env, parameter_ident->value, cur_argument_node->value);

    cur_argument_node = cur_argument_node->next;
    cur_parameter_node = cur_parameter_node->next;
  }

  return env;
}

Object *unwrap_return_value(Object *evaluated) {
  if (evaluated->type == RETURN_OBJ) {
    ReturnValue *ret = evaluated->object;
    return ret->value;
  }

  return evaluated;
}

Object *apply_function(Object *fn_obj, LinkedList *args) {
  if (fn_obj->type != FUNCTION_OBJ) {
    char err_msg[255];
    sprintf(err_msg, "Not a function: %s", ObjectTypeString[FUNCTION_OBJ]);
    return new_error(err_msg);
  }

  Function *fn = fn_obj->object;

  Environment *extended_env = extend_function_env(fn, args);
  Object *evaluated = eval_block_statement(fn->body->statements, extended_env);

  return unwrap_return_value(evaluated);
}

Object *eval_function_call(CallExpression *call, Environment *env) {
  Object *fn = eval_expression(call->function, env);
  if (is_error(fn)) {
    return fn;
  }

  LinkedList *args = eval_expressions(call->arguments, env);
  if (args->size == 1 && is_error(args->tail->value)) {
    free_object(fn);
    Object *ret_value = malloc(sizeof(Object));

    memcpy(args->tail->value, ret_value, sizeof(Object));
    free_list(args);
    return ret_value;
  }

  return apply_function(fn, args);
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
  case FN_EXPR:
    return eval_function_literal(expr->value, env);
  case CALL_EXPR:
    return eval_function_call(expr->value, env);
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
