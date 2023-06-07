#define MAX_LEN 1000

#define FOREACH_TOKEN_TYPE(TYPE) \
  TYPE(ILLEGAL) \
  TYPE(END_OF_FILE) \
  TYPE(IDENT) \
  TYPE(INT) \
  TYPE(ASSIGN) \
  TYPE(PLUS) \
  TYPE(COMMA) \
  TYPE(SEMICOLON) \
  TYPE(LPAREN) \
  TYPE(RPAREN) \
  TYPE(LBRACE) \
  TYPE(RBRACE) \
  TYPE(BANG) \
  TYPE(LT) \
  TYPE(GT) \
  TYPE(MINUS) \
  TYPE(SLASH) \
  TYPE(ASTERISK) \
  TYPE(EQ) \
  TYPE(NOT_EQ) \
  TYPE(FUNCTION) \
  TYPE(LET) \
  TYPE(TRUE) \
  TYPE(FALSE) \
  TYPE(IF) \
  TYPE(ELSE) \
  TYPE(RETURN) \
  TYPE(STRING) \
  TYPE(LBRACKET) \
  TYPE(RBRACKET) \
  TYPE(COLON) \
  TYPE(WHILE) \
  TYPE(BREAK) \
  TYPE(CONTINUE) \
  TYPE(FOR) \
  TYPE(BINARY) \
  TYPE(HEX) \
  TYPE(TOKEN_COUNT) \


#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
    FOREACH_TOKEN_TYPE(GENERATE_ENUM)
} TokenType;

extern const char *TOKEN_STRING[];

typedef struct {
  TokenType Type;
  char literal[MAX_LEN];
} Token;

TokenType lookup_ident(char *ident);
