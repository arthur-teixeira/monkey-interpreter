#include "./evaluator.h"
#include "../str_utils/str_utils.h"
#include "./builtins/builtins.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
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

    if (result != NULL &&
        (result->type == RETURN_OBJ || result->type == ERROR_OBJ ||
         result->type == CONTINUE_OBJ || result->type == BREAK_OBJ)) {
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

static Null null_value = {};

static Object obj_null = {
    NULL_OBJ,
    &null_value,
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

Object *eval_string_expression(StringLiteral *expr) {
  Object *str_obj = malloc(sizeof(Object));
  assert(str_obj != NULL && "Error allocating memory for string object");
  str_obj->type = STRING_OBJ;

  String *str = malloc(sizeof(String));
  assert(str != NULL && "error allocating memory for string");

  str->len = expr->len;
  str->value = strdup(expr->value);

  str_obj->object = str;

  return str_obj;
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
    evaluated->value = left->value / right->value;
  } else if (strcmp(operator, "<<") == 0) {
    evaluated->value = left->value << right->value;
  } else if (strcmp(operator, ">>") == 0) {
    evaluated->value = left->value >> right->value;
  } else if (strcmp(operator, "|") == 0) {
    evaluated->value = left->value | right->value;
  } else if (strcmp(operator, "&") == 0) {
    evaluated->value = left->value & right->value;
  } else if (strcmp(operator, "^") == 0) {
    evaluated->value = left->value ^ right->value;
  } else if (strcmp(operator, "%") == 0) {
    evaluated->value = left->value % right->value;
  } else {
    free(obj);
    free(evaluated);
    return eval_integer_boolean_operation(left_obj, operator, right_obj);
  }

  obj->object = evaluated;
  return obj;
}

Object *eval_string_infix_expression(Object *left, char *operator,
                                     Object * right) {
  assert(left->type == STRING_OBJ && "left object should be string");
  assert(right->type == STRING_OBJ && "right object should be string");

  if (strcmp(operator, "+") != 0) {
    char error_message[255];
    sprintf(error_message, "unknown operator: %s %s %s",
            ObjectTypeString[left->type], operator,
            ObjectTypeString[right->type]);

    return new_error(error_message);
  }

  String *left_str = left->object;
  String *right_str = right->object;

  Object *new_str_obj = malloc(sizeof(Object));
  new_str_obj->type = STRING_OBJ;

  String *new_string = malloc(sizeof(String));
  new_string->len = left_str->len + right_str->len;

  new_string->value = malloc(new_string->len + 1);

  strlcpy(new_string->value, left_str->value, new_string->len + 1);
  strncat(new_string->value, right_str->value, new_string->len + 1);

  new_str_obj->object = new_string;
  return new_str_obj;
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

  if (right->type == STRING_OBJ && left->type == STRING_OBJ) {
    return eval_string_infix_expression(left, expr->operator, right);
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
  hashmap_t builtins;
  hashmap_create(10, &builtins);
  get_builtins(&builtins);
  Object *builtin = hashmap_get(&builtins, ident->value, strlen(ident->value));

  hashmap_destroy(&builtins);
  if (builtin != NULL) {
    return builtin;
  }

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

  fn->body = lit->body;
  fn->parameters = lit->parameters;

  fn_obj->object = fn;

  return fn_obj;
}

LinkedList *eval_expressions(LinkedList *expressions, Environment *env) {
  LinkedList *evaluated = new_list();

  Node *cur_expression_node = expressions->tail;
  while (cur_expression_node != NULL) {
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
  Node *cur_argument_node = args->tail;            // Object *
  Node *cur_parameter_node = fn->parameters->tail; // Identifier *

  while (cur_parameter_node != NULL && cur_parameter_node != NULL) {
    Identifier *parameter_ident = cur_parameter_node->value;

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
  if (fn_obj->type == BUILTIN_OBJ) {
    Builtin *builtin = fn_obj->object;
    Object *result = builtin->fn(args);

    return result;
  }

  if (fn_obj->type != FUNCTION_OBJ) {
    char err_msg[255];
    sprintf(err_msg, "Not a function: %s", ObjectTypeString[FUNCTION_OBJ]);
    return new_error(err_msg);
  }
  Function *fn = fn_obj->object;

  if (fn->parameters->size != args->size) {
    char err_msg[255];
    sprintf(err_msg, "Wrong parameter count: Expected %ld got %ld",
            fn->parameters->size, args->size);
    return new_error(err_msg);
  }

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

Object *eval_array_literal(ArrayLiteral *literal, Environment *env) {
  Object *array_obj = malloc(sizeof(Object));
  assert(array_obj != NULL && "Error allocating memory for array object");

  Array *array = malloc(sizeof(Array));
  assert(array != NULL && "Error allocating memory for array");
  array_init(&array->elements, literal->elements->len);

  for (size_t i = 0; i < literal->elements->len; i++) {
    Object *evaluated = eval_expression(literal->elements->arr[i], env);
    if (is_error(evaluated)) {
      array_free(&array->elements);
      return evaluated;
    }

    array_append(&array->elements, evaluated);
  }

  array_obj->type = ARRAY_OBJ;
  array_obj->object = array;

  return array_obj;
}

Object *eval_array_indexing(Object *left, IndexExpression *idx,
                            Environment *env) {
  Object *evaluated_index = eval_expression(idx->index, env);

  if (evaluated_index->type != INTEGER_OBJ) {
    char error_msg[255];
    sprintf(error_msg,
            "attempting to index array with non-integer index: got %s",
            ObjectTypeString[evaluated_index->type]);

    return new_error(error_msg);
  }

  assert(left->type == ARRAY_OBJ);
  Array *evaluated_array = left->object;
  Integer *index = evaluated_index->object;

  if (index->value > evaluated_array->elements.len - 1 || index->value < 0) {
    return &obj_null;
  }

  return evaluated_array->elements.arr[index->value];
}

Object *eval_hash_indexing(Object *left, IndexExpression *idx,
                           Environment *env) {
  Object *evaluated_index = eval_expression(idx->index, env);

  assert(left->type == HASH_OBJ);
  Hash *hash_object = left->object;

  int32_t key = get_hash_key(evaluated_index);
  if (key == -1) {
    char error_msg[255];
    sprintf(error_msg, "unusable as hash key: %s",
            ObjectTypeString[evaluated_index->type]);

    return new_error(error_msg);
  }

  HashPair *pair = hashmap_get(&hash_object->pairs, &key, sizeof(int32_t));
  if (pair == NULL) {
    return &obj_null;
  }

  return &pair->value;
}

Object *eval_index_expression(IndexExpression *idx, Environment *env) {
  Object *left = eval_expression(idx->left, env);

  switch (left->type) {
  case ARRAY_OBJ:
    return eval_array_indexing(left, idx, env);
  case HASH_OBJ:
    return eval_hash_indexing(left, idx, env);
  default:
    break;
  }

  char error_msg[255];
  sprintf(error_msg, "attempting to index %s", ObjectTypeString[left->type]);
  return new_error(error_msg);
}

typedef struct {
  Object *error;
  Environment *env;
  hashmap_t *evaluated_hash;
} hashEvaluationContext;

int iter_eval_hash_literal(void *context, hashmap_element_t *pair) {
  hashEvaluationContext *eval_context = context;

  Expression *key_node = (Expression *)pair->key;
  Expression *value_node = pair->data;

  Object *key = eval_expression(key_node, eval_context->env);
  if (is_error(key)) {
    eval_context->error = key;
    return 1;
  }

  int32_t hash_key = get_hash_key(key);
  if (hash_key == -1) {
    char err_msg[255];
    sprintf(err_msg, "Unusable as hash key: %s", ObjectTypeString[key->type]);
    eval_context->error = new_error(err_msg);
    return 1;
  }
  int32_t *hash_key_in_heap = malloc(sizeof(int32_t));
  assert(hash_key_in_heap != NULL);

  memcpy(hash_key_in_heap, &hash_key, sizeof(int32_t));

  Object *value = eval_expression(value_node, eval_context->env);
  if (is_error(value)) {
    eval_context->error = value;
    return 1;
  }

  HashPair *hash_pair = malloc(sizeof(HashPair));
  assert(pair != NULL);
  hash_pair->key = *key;
  hash_pair->value = *value;

  hashmap_put(eval_context->evaluated_hash, hash_key_in_heap, sizeof(int32_t),
              hash_pair);

  return 0;
}

Object *eval_hash_literal(HashLiteral *lit, Environment *env) {
  Object *hash_obj = malloc(sizeof(Object));
  assert(hash_obj != NULL);
  hash_obj->type = HASH_OBJ;

  Hash *hash = malloc(sizeof(Hash));
  assert(hash != NULL);

  hashmap_create(10, &hash->pairs);

  hashEvaluationContext context;
  context.error = NULL;
  context.env = env;
  context.evaluated_hash = &hash->pairs;

  hashmap_iterate_pairs(&lit->pairs, &iter_eval_hash_literal, &context);

  if (context.error != NULL) {
    free(hash_obj);
    hashmap_destroy(&hash->pairs);
    free(hash);

    return context.error;
  }

  hash_obj->object = hash;

  return hash_obj;
}

Object *check_non_boolean_condition(Object *condition) {
  if (condition->type != BOOLEAN_OBJ) {
    free(condition);
    char error_message[255];
    sprintf(error_message, "Loop condition should produce a boolean value");
    return new_error(error_message);
  }

  return NULL;
}

Object *eval_loop_condition(Expression *condition_expr, Environment *env) {
  if (condition_expr == NULL) {
    return &obj_true;
  }

  return eval_expression(condition_expr, env);
}

Object *eval_loop(Expression *condition_expr, BlockStatement *body,
                  Statement *update, Environment *env) {
  Object *condition = eval_loop_condition(condition_expr, env);
  Object *error = check_non_boolean_condition(condition);
  if (error != NULL) {
    return error;
  }

  Object *result;
  while ((condition = eval_loop_condition(condition_expr, env)) == &obj_true) {
    result = eval_block_statement(body->statements, env);
    if (result == NULL) {
      continue;
    }

    switch (result->type) {
    case RETURN_OBJ:
      return result;
    case CONTINUE_OBJ:
      continue;
    default:
      break;
    }

    if (result->type == BREAK_OBJ) {
      break;
    }

    if (update != NULL) {
      eval(update, env);
    }
  }

  return result;
}

Object *eval_while_loop(WhileLoop *loop, Environment *env) {
  Environment *extended_env = new_enclosed_environment(env);
  return eval_loop(loop->condition, loop->body, NULL, extended_env);
}

Object *eval_for_loop(ForLoop *loop, Environment *env) {
  Environment *extended_env = new_enclosed_environment(env);

  if (loop->initialization != NULL) {
    // might turn into runtime error
    assert(loop->initialization->type == LET_STATEMENT);
    eval(loop->initialization, extended_env);
  }

  return eval_loop(loop->condition, loop->body, loop->update, extended_env);
}

Object *inner_env_get(Environment *env, char *key) {
  return hashmap_get(env->store, key, strlen(key));
}

Object *eval_reassignment(Reassignment *stmt, Environment *env) {
  Object *cur_value = inner_env_get(env, stmt->name->value);
  bool val_in_outer_env = !cur_value;

  if (val_in_outer_env) {
    cur_value = env_get(env, stmt->name->value);
    val_in_outer_env = cur_value;
  }

  if (cur_value == NULL) {
    char error_message[255];
    sprintf(error_message, "%s is not defined", stmt->name->value);

    return new_error(error_message);
  }

  Object *val = eval_expression(stmt->value, env);
  if (is_error(val)) {
    return val;
  }

  if (val_in_outer_env) {
    assert(env->outer != NULL);
    env_set(env->outer, stmt->name->value, val);
  } else {
    env_set(env, stmt->name->value, val);
  }

  return NULL;
}

Object *eval_expression(Expression *expr, Environment *env) {
  switch (expr->type) {
  case INT_EXPR:
    return new_integer(expr->value);
  case BOOL_EXPR:
    return new_boolean(expr->value);
  case STRING_EXPR:
    return eval_string_expression(expr->value);
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
  case ARRAY_EXPR:
    return eval_array_literal(expr->value, env);
  case INDEX_EXPR:
    return eval_index_expression(expr->value, env);
  case HASH_EXPR:
    return eval_hash_literal(expr->value, env);
  case WHILE_EXPR:
    return eval_while_loop(expr->value, env);
  case FOR_EXPR:
    return eval_for_loop(expr->value, env);
  case REASSIGN_EXPR:
    return eval_reassignment(expr->value, env);
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

Object *eval_continue_statement(Statement *stmt) {
  Object *obj = malloc(sizeof(Object));
  assert(obj != NULL);

  obj->type = CONTINUE_OBJ;
  obj->object = NULL;

  return obj;
}

Object *eval_break_statement(Statement *stmt) {
  Object *obj = malloc(sizeof(Object));
  assert(obj != NULL);

  obj->type = BREAK_OBJ;
  obj->object = NULL;

  return obj;
}

Object *eval(Statement *stmt, Environment *env) {
  switch (stmt->type) {
  case EXPR_STATEMENT:
    return eval_expression(stmt->expression, env);
  case RETURN_STATEMENT:
    return eval_return_statement(stmt->expression, env);
  case LET_STATEMENT:
    return eval_let_statement(stmt, env);
  case CONTINUE_STATEMENT:
    return eval_continue_statement(stmt);
  case BREAK_STATEMENT:
    return eval_break_statement(stmt);
  default:
    assert(0 && "unreachable");
    return NULL;
  }
}
