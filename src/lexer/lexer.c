#include "lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void read_char(Lexer *);

Lexer *new_lexer(char *input) {
  Lexer *l = malloc(sizeof(Lexer));
  l->input = input;
  read_char(l);

  return l;
}

// Stolen from
// https://android.googlesource.com/platform/system/core.git/+/master/libcutils/strlcpy.c
// Not using strncpy because it does not null-terminate the string,
// but converts a string into a raw character buffer. More can be found at
// https://devblogs.microsoft.com/oldnewthing/20050107-00/?p=36773
size_t strlcpy(char *dst, const char *src, size_t siz) {
  char *d = dst;
  const char *s = src;
  size_t n = siz;
  /* Copy as many bytes as will fit */
  if (n != 0) {
    while (--n != 0) {
      if ((*d++ = *s++) == '\0')
        break;
    }
  }
  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (siz != 0)
      *d = '\0'; /* NUL-terminate dst */
    while (*s++)
      ;
  }
  return (s - src - 1); /* count does not include NUL */
}

void slice(const char *str, char *result, size_t start, size_t end) {
  strlcpy(result, str + start, end - start + 1);
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

bool is_letter(char c) { return isalpha(c) || c == '_'; }

void read_identifier(Lexer *l, char *result) {
  uint32_t position = l->position;
  while (is_letter(l->ch)) {
    read_char(l);
  }

  slice(l->input, result, position, l->position);
}

// TODO: add support to hex, binary, octal and float numbers
void read_number(Lexer *l, char *result) {
  uint32_t position = l->position;
  while (isdigit(l->ch)) {
    read_char(l);
  }

  slice(l->input, result, position, l->position);
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

void skip_whitespace(Lexer *l) {
  if (l->ch == ' ' || l->ch == '\n' || l->ch == '\t' || l->ch == '\r')
    read_char(l);
}

Token next_token(Lexer *l) {
  Token tok;

  skip_whitespace(l);

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
      read_identifier(l, tok.literal);
      tok.Type = lookup_ident(tok.literal);
      return tok;
    } else if (isdigit(l->ch)) {
      tok.Type = INT;
      read_number(l, tok.literal);
      return tok;
    } else {
      tok = new_token(ILLEGAL, l->ch);
    }
  }

  read_char(l);
  return tok;
}
