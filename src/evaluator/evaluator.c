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
    free(obj);
  }
}

Object *new_error(char *message) {
  Error *err = malloc(sizeof(Error));
  assert(err != NULL && "Error allocating memory for error");
  err->message = strdup(message);
  err->type = ERROR_OBJ;

  return (Object *)err;
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
      ReturnValue *ret = (ReturnValue *)result;
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

Object *new_integer(NumberLiteral *lit) {
  Number *int_obj = malloc(sizeof(Number));
  assert(int_obj != NULL && "Error allocating memory for integer");

  int_obj->value = lit->value;
  int_obj->type = NUMBER_OBJ;

  return (Object *)int_obj;
}

static Boolean obj_true = {
    .type = BOOLEAN_OBJ,
    .value = true,
};

static Boolean obj_false = {
    .type = BOOLEAN_OBJ,
    .value = false,
};

static Null obj_null = {
    .type = NULL_OBJ,
};

Object *native_bool_to_boolean_object(bool condition) {
  if (condition) {
    return (Object *)&obj_true;
  }

  return (Object *)&obj_false;
}

Object *new_boolean(BooleanLiteral *bol) {
  return native_bool_to_boolean_object(bol->value);
}

Object *inverted_boolean(Boolean *bol) {
  return native_bool_to_boolean_object(!bol->value);
}

Object *eval_string_expression(StringLiteral *expr) {
  String *str = malloc(sizeof(String));
  assert(str != NULL && "error allocating memory for string");

  str->type = STRING_OBJ;
  str->len = expr->len;
  str->value = strdup(expr->value);

  return (Object *)str;
}

Object *cast_int_to_boolean(Number *intt) {
  return native_bool_to_boolean_object(!intt->value);
}

Object *eval_bang_operator_expression(Object *right) {
  switch (right->type) {
  case BOOLEAN_OBJ:
    return inverted_boolean((Boolean *)right);
  case NULL_OBJ:
    return (Object *)&obj_true;
  case NUMBER_OBJ:
    return cast_int_to_boolean((Number *)right);
  default:
    return (Object *)&obj_false;
  }
}

Object *eval_minus_operator_expression(Object *right) {
  if (right->type != NUMBER_OBJ) {
    char error_message[255];
    sprintf(error_message, "unknown operator: -%s",
            ObjectTypeString[right->type]);

    return new_error(error_message);
  };

  Number *intt = (Number *)right;

  Number *new_int = malloc(sizeof(Number));
  assert(new_int != NULL && "error allocating memory for new integer");

  new_int->type = NUMBER_OBJ;
  new_int->value = -intt->value;

  return (Object *)new_int;
}

Object *eval_integer_boolean_operation(Object *left_obj, char *operator,
                                       Object * right_obj) {
  Number *left = (Number *)left_obj;
  Number *right = (Number *)right_obj;

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
  Number *evaluated = malloc(sizeof(Number));
  assert(evaluated != NULL && "error allocating memory for evaluated integer");

  evaluated->type = NUMBER_OBJ;
  Number *left = (Number *)left_obj;
  Number *right = (Number *)right_obj;

  if (strcmp(operator, "+") == 0) {
    evaluated->value = left->value + right->value;
  } else if (strcmp(operator, "-") == 0) {
    evaluated->value = left->value - right->value;
  } else if (strcmp(operator, "*") == 0) {
    evaluated->value = left->value * right->value;
  } else if (strcmp(operator, "/") == 0) {
    evaluated->value = left->value / right->value;
  } else if (strcmp(operator, "<<") == 0) {
    evaluated->value = (long)left->value << (long)right->value;
  } else if (strcmp(operator, ">>") == 0) {
    evaluated->value = (long)left->value >> (long)right->value;
  } else if (strcmp(operator, "|") == 0) {
    evaluated->value = (long)left->value | (long)right->value;
  } else if (strcmp(operator, "&") == 0) {
    evaluated->value = (long)left->value & (long)right->value;
  } else if (strcmp(operator, "^") == 0) {
    evaluated->value = (long)left->value ^ (long)right->value;
  } else if (strcmp(operator, "%") == 0) {
    evaluated->value = (long)left->value % (long)right->value;
  } else {
    free(evaluated);
    return eval_integer_boolean_operation(left_obj, operator, right_obj);
  }

  return (Object *)evaluated;
}

Object *eval_boolean_infix_expression(Object *left, char *operator,
                                      Object * right) {
  // These work because we only have a single instance for the true and false
  // objects, so we can simply do a pointer comparison.
  if (strcmp(operator, "==") == 0)
    return native_bool_to_boolean_object(right == left);

  if (strcmp(operator, "!=") == 0)
    return native_bool_to_boolean_object(right != left);

  if (strcmp(operator, "&&") == 0)
    return native_bool_to_boolean_object(((Boolean *)right == &obj_true) &&
                                         ((Boolean *)left == &obj_true));

  if (strcmp(operator, "||") == 0)
    return native_bool_to_boolean_object(((Boolean *)right == &obj_true) ||
                                         ((Boolean *)left == &obj_true));

  char error_message[255];
  sprintf(error_message, "unknown operator: %s %s %s",
          ObjectTypeString[left->type], operator,
          ObjectTypeString[right->type]);

  return new_error(error_message);
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

  String *left_str = (String *)left;
  String *right_str = (String *)right;

  String *new_string = malloc(sizeof(String));
  assert(new_string != NULL);

  new_string->type = STRING_OBJ;
  new_string->len = left_str->len + right_str->len;

  new_string->value = malloc(new_string->len + 1);

  strlcpy(new_string->value, left_str->value, new_string->len + 1);
  strncat(new_string->value, right_str->value, new_string->len + 1);

  return (Object *)new_string;
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

  if (right->type == NUMBER_OBJ && left->type == NUMBER_OBJ) {
    return eval_integer_infix_expression(left, expr->operator, right);
  }

  if (right->type == STRING_OBJ && left->type == STRING_OBJ) {
    return eval_string_infix_expression(left, expr->operator, right);
  }

  if (right->type == BOOLEAN_OBJ && left->type == BOOLEAN_OBJ) {
    return eval_boolean_infix_expression(left, expr->operator, right);
  }

  if ((right->type == NULL_OBJ || left->type == NULL_OBJ)) {
    if (strcmp(expr->operator, "==") == 0)
      return native_bool_to_boolean_object(right == left);

    if (strcmp(expr->operator, "!=") == 0)
      return native_bool_to_boolean_object(right != left);

    if (strcmp(expr->operator, "&&") == 0)
      return native_bool_to_boolean_object(((Boolean *)right == &obj_true) &&
                                           ((Boolean *)left == &obj_true));

    if (strcmp(expr->operator, "||") == 0)
      return native_bool_to_boolean_object(((Boolean *)right == &obj_true) ||
                                           ((Boolean *)left == &obj_true));
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
  if (obj == NULL || (Boolean *)obj == &obj_false) {
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
  Function *fn = malloc(sizeof(Function));
  fn->type = FUNCTION_OBJ;
  fn->env = env;

  fn->body = lit->body;
  fn->parameters = lit->parameters;

  return (Object *)fn;
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
  assert(args->size == fn->parameters.len);

  Environment *env = new_enclosed_environment(fn->env);
  Node *cur_argument_node = args->tail; // Object *

  for (uint32_t i = 0; i < fn->parameters.len;
       i++, cur_argument_node = cur_argument_node->next) {
    Identifier *parameter_ident = fn->parameters.arr[i];
    env_set(env, parameter_ident->value, cur_argument_node->value);
  }

  return env;
}

Object *unwrap_return_value(Object *evaluated) {
  if (evaluated->type == RETURN_OBJ) {
    ReturnValue *ret = (ReturnValue *)evaluated;
    return ret->value;
  }

  return evaluated;
}

Object *apply_function(Object *fn_obj, LinkedList *args) {
  if (fn_obj->type == BUILTIN_OBJ) {
    Builtin *builtin = (Builtin *)fn_obj;
    Object *result = builtin->fn(args);

    return result;
  }

  if (fn_obj->type != FUNCTION_OBJ) {
    char err_msg[255];
    sprintf(err_msg, "Not a function: %s", ObjectTypeString[FUNCTION_OBJ]);
    return new_error(err_msg);
  }
  Function *fn = (Function *)fn_obj;

  if (fn->parameters.len != args->size) {
    char err_msg[255];
    sprintf(err_msg, "Wrong parameter count: Expected %ld got %ld",
            fn->parameters.len, args->size);
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

  array->type = ARRAY_OBJ;
  return (Object *)array;
}

Object *eval_array_indexing(Object *left, IndexExpression *idx,
                            Environment *env) {
  Object *evaluated_index = eval_expression(idx->index, env);

  if (evaluated_index->type != NUMBER_OBJ) {
    char error_msg[255];
    sprintf(error_msg,
            "attempting to index array with non-integer index: got %s",
            ObjectTypeString[evaluated_index->type]);

    return new_error(error_msg);
  }

  assert(left->type == ARRAY_OBJ);
  Array *evaluated_array = (Array *)left;
  Number *index = (Number *)evaluated_index;

  if (index->value > evaluated_array->elements.len - 1 || index->value < 0) {
    return (Object *)&obj_null;
  }

  return evaluated_array->elements.arr[(long)index->value];
}

Object *eval_hash_indexing(Object *left, IndexExpression *idx,
                           Environment *env) {
  Object *evaluated_index = eval_expression(idx->index, env);

  assert(left->type == HASH_OBJ);
  Hash *hash_object = (Hash *)left;

  int32_t key = get_hash_key(evaluated_index);
  if (key == -1) {
    char error_msg[255];
    sprintf(error_msg, "unusable as hash key: %s",
            ObjectTypeString[evaluated_index->type]);

    return new_error(error_msg);
  }

  HashPair *pair = hashmap_get(&hash_object->pairs, &key, sizeof(int32_t));
  if (pair == NULL) {
    return (Object *)&obj_null;
  }

  return pair->value;
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
  hash_pair->key = key;
  hash_pair->value = value;

  hashmap_put(eval_context->evaluated_hash, hash_key_in_heap, sizeof(int32_t),
              hash_pair);

  return 0;
}

Object *eval_hash_literal(HashLiteral *lit, Environment *env) {
  Hash *hash = malloc(sizeof(Hash));
  assert(hash != NULL);

  hashmap_create(10, &hash->pairs);

  hashEvaluationContext context;
  context.error = NULL;
  context.env = env;
  context.evaluated_hash = &hash->pairs;

  hashmap_iterate_pairs(&lit->pairs, &iter_eval_hash_literal, &context);

  if (context.error != NULL) {
    hashmap_destroy(&hash->pairs);
    free(hash);

    return context.error;
  }

  hash->type = HASH_OBJ;
  return (Object *)hash;
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
    return (Object *)&obj_true;
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
  while ((condition = eval_loop_condition(condition_expr, env)) ==
         (Object *)&obj_true) {
    result = eval_block_statement(body->statements, env);
    if (result == NULL) {
      if (update != NULL) {
        eval(update, env);
      }
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
    Environment *cur_env = env->outer;
    Object *outer_env_value;
    while ((outer_env_value = env_get(cur_env, stmt->name->value)) == NULL) {
      cur_env = cur_env->outer;
    }

    assert(cur_env != NULL);
    env_set(cur_env, stmt->name->value, val);
  } else {
    env_set(env, stmt->name->value, val);
  }

  return NULL;
}

Object *eval_expression(Expression *expr, Environment *env) {
  switch (expr->type) {
  case INT_EXPR:
    return new_integer((NumberLiteral *)expr);
  case BOOL_EXPR:
    return new_boolean((BooleanLiteral *)expr);
  case STRING_EXPR:
    return eval_string_expression((StringLiteral *)expr);
  case PREFIX_EXPR:
    return eval_prefix_expression((PrefixExpression *)expr, env);
  case INFIX_EXPR:
    return eval_infix_expression((InfixExpression *)expr, env);
  case IF_EXPR:
    return eval_if_expression((IfExpression *)expr, env);
  case IDENT_EXPR:
    return eval_identifier((Identifier *)expr, env);
  case FN_EXPR:
    return eval_function_literal((FunctionLiteral *)expr, env);
  case CALL_EXPR:
    return eval_function_call((CallExpression *)expr, env);
  case ARRAY_EXPR:
    return eval_array_literal((ArrayLiteral *)expr, env);
  case INDEX_EXPR:
    return eval_index_expression((IndexExpression *)expr, env);
  case HASH_EXPR:
    return eval_hash_literal((HashLiteral *)expr, env);
  case WHILE_EXPR:
    return eval_while_loop((WhileLoop *)expr, env);
  case FOR_EXPR:
    return eval_for_loop((ForLoop *)expr, env);
  case REASSIGN_EXPR:
    return eval_reassignment((Reassignment *)expr, env);
  }

  assert(0 && "unreachable");
}

Object *eval_return_statement(Expression *expr, Environment *env) {
  ReturnValue *ret = malloc(sizeof(ReturnValue));
  assert(ret != NULL && "error allocating memory for return value");

  ret->value = eval_expression(expr, env);

  if (is_error(ret->value)) {
    Object *err = ret->value;
    free(ret);
    return err;
  }

  ret->type = RETURN_OBJ;
  return (Object *)ret;
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

  return obj;
}

Object *eval_break_statement(Statement *stmt) {
  Object *obj = malloc(sizeof(Object));
  assert(obj != NULL);

  obj->type = BREAK_OBJ;

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
