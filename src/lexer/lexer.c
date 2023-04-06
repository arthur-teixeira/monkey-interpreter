#include "lexer.h"
#include <stdlib.h>
#include <string.h>

void read_char(Lexer *);

Lexer *new_lexer(char *input) {
  Lexer *l = malloc(sizeof(Lexer));
  l->input = input;
  read_char(l);

  return l;
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
  default:
    tok = new_token(END_OF_FILE, '\0');
    break;
  }

  read_char(l);
  return tok;
}
