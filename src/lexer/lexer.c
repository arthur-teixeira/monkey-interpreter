#include "lexer.h"
#include "../str_utils/str_utils.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_char(Lexer *);

Lexer *new_lexer(char *input) {
  Lexer *l = malloc(sizeof(Lexer));
  l->input = input;
  read_char(l);

  return l;
}

void free_lexer(Lexer *l) { free(l); }

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

char peek_char(Lexer *l) {
  if (l->read_position >= strlen(l->input)) {
    return '\0';
  }

  return l->input[l->read_position];
}

bool is_letter(char c) { return isalpha(c) || c == '_'; }

void read_identifier(Lexer *l, char *result) {
  uint32_t position = l->position;
  while (is_letter(l->ch)) {
    read_char(l);
  }

  slice(l->input, result, position, l->position);
}

void read_number(Lexer *l, char *result) {
  uint32_t position = l->position;
  while (isdigit(l->ch) || l->ch == '.') {
    read_char(l);
  }

  slice(l->input, result, position, l->position);
}

void read_hex(Lexer *l, char *result) {
  uint32_t position = l->position;
  while (isalnum(l->ch)) {
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
  while (l->ch == ' ' || l->ch == '\n' || l->ch == '\t' || l->ch == '\r')
    read_char(l);
}

void read_string(char *result, Lexer *l) {
  int start = l->read_position;
  for (;;) {
    read_char(l);

    if (l->ch == '"' || l->ch == 0) {
      break;
    }
  }

  int end = l->position;

  slice(l->input, result, start, end);
}

Token read_special_number(Lexer *l) {
  Token tok;

  char peek = peek_char(l);
  read_char(l);
  read_char(l);

  switch(peek) {
    case 'x':
      tok.Type = HEX;
      read_hex(l, tok.literal);
      break;
    case 'b':
      tok.Type = BINARY;
      read_number(l, tok.literal);
      break;
    default:
      assert(0 && "unreachable");
  }

  return tok;
}

Token read_lshift(Lexer *l) {
  read_char(l);
  read_char(l);

  Token tok = {
    .Type = LSHIFT,
    .literal = "<<",
  };

  return tok;
}

Token read_rshift(Lexer *l) {
  read_char(l);
  read_char(l);

  Token tok = {
    .Type = RSHIFT,
    .literal = ">>",
  };

  return tok;
}

Token next_token(Lexer *l) {
  Token tok;

  skip_whitespace(l);

  switch (l->ch) {
  case '=':
    if (peek_char(l) == '=') {
      tok.Type = EQ;
      strlcpy(tok.literal, "==", 3);
      read_char(l);
    } else {
      tok = new_token(ASSIGN, l->ch);
    }
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
  case '[':
    tok = new_token(LBRACKET, l->ch);
    break;
  case ']':
    tok = new_token(RBRACKET, l->ch);
    break;
  case ':':
    tok = new_token(COLON, l->ch);
    break;
  case '"':
    tok.Type = STRING;
    read_string(tok.literal, l);
    break;
  case '!':
    if (peek_char(l) == '=') {
      tok.Type = NOT_EQ;
      strlcpy(tok.literal, "!=", 3);
      read_char(l);
    } else {
      tok = new_token(BANG, l->ch);
    }
    break;
  case '<':
    if (peek_char(l) == '<') {
      return read_lshift(l);
    }
    tok = new_token(LT, l->ch);
    break;
  case '>':
    if (peek_char(l) == '>') {
      return read_rshift(l);
    }
    tok = new_token(GT, l->ch);
    break;
  case '-':
    tok = new_token(MINUS, l->ch);
    break;
  case '/':
    tok = new_token(SLASH, l->ch);
    break;
  case '*':
    tok = new_token(ASTERISK, l->ch);
    break;
  case '^':
    tok = new_token(BXOR, l->ch);
    break;
  case '&':
    if (peek_char(l) == '&') {
      tok.Type = AND;
      strlcpy(tok.literal, "&&", 3);
      read_char(l);
    } else {
      tok = new_token(BAND, l->ch);
    }
    break;
  case '|':
    if (peek_char(l) == '|') {
      tok.Type = OR;
      strlcpy(tok.literal, "||", 3);
      read_char(l);
    } else {
      tok = new_token(BOR, l->ch);
    }
    break;
  case '%':
    tok = new_token(MOD, l->ch);
    break;
  default:
    if (is_letter(l->ch)) {
      read_identifier(l, tok.literal);
      tok.Type = lookup_ident(tok.literal);
      return tok;
    } else if (isdigit(l->ch)) {
      if (l->ch == '0' && is_letter(peek_char(l))) {
        return read_special_number(l);
      }

      tok.Type = NUMBER;
      read_number(l, tok.literal);
      return tok;
    } else if (strcmp("\n", &l->ch) || strcmp("\0", &l->ch)) {
      tok = new_token(END_OF_FILE, '\0');
      return tok;
    } else {
      tok = new_token(ILLEGAL, l->ch);
    }
  }

  read_char(l);
  return tok;
}
