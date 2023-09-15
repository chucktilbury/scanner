#ifndef _TOKENS_H
#define _TOKENS_H

#include "strs.h"

typedef enum {
    ERROR,
    NAME,
    COLON,
    RULE,
    LITERAL,
    EOI,
} TOK_TYPE;

typedef struct {
    TOK_TYPE tok;
    STR str;
} TOKEN;

void init_tokens(const char* fname);
TOKEN* crnt_token();
void consume_token();
const char* tok_to_str(TOK_TYPE tok);

#endif /* _TOKENS_H */
