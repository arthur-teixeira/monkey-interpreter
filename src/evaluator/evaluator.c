#include "./evaluator.h"
#include "../object/builtins.h"
#include "../object/object.h"
#include "../str_utils/str_utils.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_error(Object *obj) {
  if (obj != NULL) {
    return obj->type == ERROR_OBJ;
  }

  return false;
}

Boolean obj_true = {
    .type = BOOLEAN_OBJ,
    .value = true,
};

Boolean obj_false = {
    .type = BOOLEAN_OBJ,
    .value = false,
};

Null obj_null = {
    .type = NULL_OBJ,
};

Object *native_bool_to_boolean_object(bool condition) {
  if (condition) {
    return (Object *)&obj_true;
  }

  return (Object *)&obj_false;
}

Object *eval_block_statement(DynamicArray *statements, Environment *env) {
  Object *result;
  for (uint32_t i = 0; i < statements->len; i++) {
    result = eval(statements->arr[i], env);

    if (result != NULL &&
        (result->type == RETURN_OBJ || result->type == ERROR_OBJ ||
         result->type == CONTINUE_OBJ || result->type == BREAK_OBJ)) {
      return result;
    }
  }

  return result;
}

Object *eval_program(Program *program, Environment *env) {
  Object *result;

  for (uint32_t i = 0; i < program->statements.len; i++) {
    result = eval(program->statements.arr[i], env);

    if (result != NULL && result->type == RETURN_OBJ) {
      ReturnValue *ret = (ReturnValue *)result;
      Object *ret_value = ret->value;
      free_object(result);
      return ret_value;
    }

    if (result != NULL && result->type == ERROR_OBJ) {
      return result;
    }
  }

  return result;
}

static Object *new_object_boolean(BooleanLiteral *bol) {
  return native_bool_to_boolean_object(bol->value);
}

Object *inverted_boolean(Boolean *bol) {
  return native_bool_to_boolean_object(!bol->value);
}

Object *eval_string_expression(StringLiteral *expr) {
  return new_string(expr->value);
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

  return new_concatted_string(left_str, right_str);
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
    return eval_block_statement(&expr->consequence->statements, env);
  } else if (expr->alternative != NULL) {
    return eval_block_statement(&expr->alternative->statements, env);
  }

  return NULL;
}

Object *eval_identifier(Identifier *ident, Environment *env) {
  const Builtin *builtin = get_builtin_by_name(ident->value);

  if (builtin) {
    return (Object *)builtin;
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

DynamicArray eval_expressions(DynamicArray *expressions, Environment *env) {
  DynamicArray evaluated;
  array_init(&evaluated, expressions->len);

  for (uint32_t i = 0; i < expressions->len; i++) {
    Object *evaluated_expression = eval_expression(expressions->arr[i], env);

    if (is_error(evaluated_expression)) {
      array_free(&evaluated);
      array_init(&evaluated, 1);
      array_append(&evaluated, evaluated_expression);
      return evaluated;
    }

    array_append(&evaluated, evaluated_expression);
  }

  return evaluated;
}

Environment *extend_function_env(const Function *fn, const DynamicArray *args) {
  assert(args->len == fn->parameters.len);

  Environment *env = new_enclosed_environment(fn->env);

  for (uint32_t i = 0; i < fn->parameters.len; i++) {
    Identifier *parameter_ident = fn->parameters.arr[i];
    env_set(env, parameter_ident->value, args->arr[i]);
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

Object *apply_function(Object *fn_obj, DynamicArray args) {
  if (fn_obj->type == BUILTIN_OBJ) {
    Builtin *builtin = (Builtin *)fn_obj;
    return builtin->fn(args);
  }

  if (fn_obj->type != FUNCTION_OBJ) {
    char err_msg[255];
    sprintf(err_msg, "Not a function: %s", ObjectTypeString[FUNCTION_OBJ]);
    return new_error(err_msg);
  }
  Function *fn = (Function *)fn_obj;

  if (fn->parameters.len != args.len) {
    char err_msg[255];
    sprintf(err_msg, "Wrong parameter count: Expected %ld got %ld",
            fn->parameters.len, args.len);
    return new_error(err_msg);
  }

  Environment *extended_env = extend_function_env(fn, &args);
  Object *evaluated = eval_block_statement(&fn->body->statements, extended_env);

  return unwrap_return_value(evaluated);
}

Object *eval_function_call(CallExpression *call, Environment *env) {
  Object *fn = eval_expression(call->function, env);
  if (is_error(fn)) {
    return fn;
  }

  DynamicArray args = eval_expressions(&call->arguments, env);
  if (args.len == 1 && is_error(args.arr[0])) {
    free_object(fn);
    Object *ret_value = malloc(sizeof(Object));

    memcpy(args.arr[0], ret_value, sizeof(Object));
    array_free(&args);
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
    result = eval_block_statement(&body->statements, env);
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

Object *resolve_ident(char *name, Environment *env, int *level) {
  *level = 0;
  Environment *cur_env = env;
  Object *value = NULL;

  while (!value && cur_env) {
    value = hashmap_get(cur_env->store, name, strlen(name));
    cur_env = env->outer;
    if (!value) {
      (*level)++;
    }
  }

  return value;
}

void set_ident(char *name, Environment *env, int level, Object *value) {
  Environment *cur_env = env;
  while (level > 0) {
    cur_env = cur_env->outer;
    level--;
  }

  env_set(cur_env, name, value);
}

Object *undefined_variable_error(char *name) {
  char error_message[255];
  sprintf(error_message, "%s is not defined", name);

  return new_error(error_message);
}

Object *eval_ident_reassignment(Identifier *name, Object *new_value,
                                Environment *env) {
  int level = 0;
  Object *cur_value = resolve_ident(name->value, env, &level);

  if (!cur_value) {
    return undefined_variable_error(name->value);
  }

  set_ident(name->value, env, level, new_value);

  return NULL;
}

Object *eval_array_index_reassignment(Expression *index_expr, Object *new_value,
                                      Array *arr, Environment *env) {
  assert(index_expr->type == INT_EXPR);
  NumberLiteral *index = (NumberLiteral *)index_expr;

  // TODO: if reassigning to greater index, resize array and set previous values
  // to null.
  assert((size_t)index->value < arr->elements.len);
  arr->elements.arr[(size_t)index->value] = new_value;

  return (Object *)arr;
}

Object *eval_array_literal_index_reassignment(IndexExpression *value,
                                              Object *new_value,
                                              Environment *env) {
  Array *arr = (Array *)eval_expression(value->left, env);

  return eval_array_index_reassignment(value->index, new_value, arr, env);
}

Object *unhandled_object_reassignment_error(ObjectType type) {
  char error_message[255];
  sprintf(error_message, "cannot reassign type %s", ObjectTypeString[type]);

  return new_error(error_message);
}

Object *eval_ident_index_reassignment(IndexExpression *index_expr,
                                      Object *new_value, Environment *env) {
  Identifier *ident = (Identifier *)index_expr->left;

  int level;
  Object *cur_value = resolve_ident(ident->value, env, &level);

  if (!cur_value) {
    return undefined_variable_error(ident->value);
  }

  switch (cur_value->type) {
  case ARRAY_OBJ:
    return eval_array_index_reassignment(index_expr->index, new_value,
                                         (Array *)cur_value, env);
  default:
    return unhandled_object_reassignment_error(cur_value->type);
  }
}

Object *unhandled_reassignment_error(ExprType type) {
  char error_message[255];
  sprintf(error_message, "cannot reassign type %d", type);

  return new_error(error_message);
}

Object *eval_index_reassignment(Reassignment *stmt, Object *new_value,
                                Environment *env) {
  IndexExpression *value = (IndexExpression *)stmt->name;

  switch (value->left->type) {
  case IDENT_EXPR:
    return eval_ident_index_reassignment(value, new_value, env);
  case ARRAY_EXPR:
    return eval_array_literal_index_reassignment(value, new_value, env);
  default:
    return unhandled_reassignment_error(value->left->type);
  }
}

Object *eval_reassignment(Reassignment *stmt, Environment *env) {
  Object *new_value = eval_expression(stmt->value, env);

  switch (stmt->name->type) {
  case IDENT_EXPR:
    return eval_ident_reassignment((Identifier *)stmt->name, new_value, env);
  case INDEX_EXPR:
    return eval_index_reassignment(stmt, new_value, env);
  default:
    return unhandled_reassignment_error(stmt->name->type);
  }
}

Object *eval_expression(Expression *expr, Environment *env) {
  switch (expr->type) {
  case INT_EXPR:
    return new_number(((NumberLiteral *)expr)->value);
  case BOOL_EXPR:
    return new_object_boolean((BooleanLiteral *)expr);
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
