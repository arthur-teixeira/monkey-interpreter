#define MAX_LEN 100

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

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum TokenType {
    FOREACH_TOKEN_TYPE(GENERATE_ENUM)
};

extern const char *TOKEN_STRING[];

typedef struct {
  enum TokenType Type;
  char literal[MAX_LEN];
} Token;

enum TokenType lookup_ident(char *ident);
