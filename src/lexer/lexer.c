#include "lexer.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

void read_char(Lexer *);

Lexer *new_lexer(char *input) {
  Lexer *l = malloc(sizeof(Lexer));
  l->input = input;
  read_char(l);

  return l;
}

void slice(const char *str, char *result, size_t start, size_t end) {
  strncpy(result, str+start, end-start);
}

void read_char(Lexer *l) {
  if (l->read_position >= strlen(l->input)) {
    l->ch = 0;
  } else {
    l->ch = l->input[l->read_position];
  }

  l->position = l->read_position;
  l->read_position += 1;
}

bool is_letter(char c) {
  return isalpha(c) || c == '_';
}

char *read_identifier(Lexer *l) {
  uint32_t position = l->position;
  while (is_letter(l->ch)) {
    read_char(l);
  }

  char *result = NULL;
  slice(l->input, result, position, l->position);

  return result;
}

Token new_token(TokenType type, char literal) {
  Token tok;
  tok.Type = type;
  char buf[2];
  buf[0] = literal;
  buf[1] = '\0';

  strncpy(tok.literal, buf, sizeof(tok.literal));

  return tok;
}

Token next_token(Lexer *l) {
  Token tok;

  switch (l->ch) {
  case '=':
    tok = new_token(ASSIGN, l->ch);
    break;
  case ';':
    tok = new_token(SEMICOLON, l->ch);
    break;
  case '(':
    tok = new_token(LPAREN, l->ch);
    break;
  case ')':
    tok = new_token(RPAREN, l->ch);
    break;
  case '{':
    tok = new_token(LBRACE, l->ch);
    break;
  case '}':
    tok = new_token(RBRACE, l->ch);
    break;
  case ',':
    tok = new_token(COMMA, l->ch);
    break;
  case '+':
    tok = new_token(PLUS, l->ch);
    break;
  case '\0':
    tok = new_token(END_OF_FILE, '\0');
  default:
    if (is_letter(l->ch)) {
      strncpy(tok.literal, read_identifier(l), sizeof(tok.literal));
      tok.Type = lookup_ident(tok.literal);
      return tok;
    } else {
      tok = new_token(ILLEGAL, l->ch);
    }

  }

  read_char(l);
  return tok;
}
