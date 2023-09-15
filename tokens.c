#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "fileio.h"
#include "memory.h"
#include "tokens.h"

static FSTK fp;
static TOKEN token;

void init_tokens(const char* fname) {

    assert(fname != NULL);
    fp = create_file_stk(fname);

    token.str = create_str(NULL);

    consume_token();
}

TOKEN* crnt_token() {

    return &token;
}

// characters in the set of [a-zA-Z_]
static void get_name(int ch) {

    while(isalpha(ch) || ch == '_') {
        append_str_char(token.str, ch);
        ch = consume_char(fp);
    }
}

// characters in the set of [ \t\f\n\r]
static void consume_space() {

    int ch = get_char(fp);

    while(isspace(ch) && ch != EOF)
        ch = consume_char(fp);
}

// consume all characters until a EOL is encountered
// the '#' has already been read
static void consume_comment() {

    int ch = consume_char(fp);

    while(ch != '\n' && ch != EOF)
        ch = consume_char(fp);
}

// characters between enclosing ')' Parens can be nested.
static void get_rule() {

    // consume the opening '(' and don't save it
    int ch = consume_char(fp);
    int lev = 1;

    while(lev > 0) {
        if(ch == '(') {
            lev++;
            append_str_char(token.str, ch);
        }
        else if(ch == ')') {
            lev--;
            // don't save the last one
            if(lev > 0)
                append_str_char(token.str, ch);
            else
                consume_char(fp);
        }
        else if(ch == '\n') {
            fprintf(stderr, "ERROR: unexpected end of line encountered.\n");
            exit(1);
        }
        else if(ch == EOF) {
            fprintf(stderr, "ERROR: unexpected end of file encountered.\n");
            exit(1);
        }
        else
            append_str_char(token.str, ch);

        ch = consume_char(fp);
    }
}

// Characters until a '"' en encountered.
static void get_literal() {

    // consume the leading '"' and don't save it
    int ch = consume_char(fp);

    while(1) {
        if(ch == '\n') {
            fprintf(stderr, "ERROR: unexpected end of line encountered.\n");
            exit(1);
        }
        else if(ch == EOF) {
            fprintf(stderr, "ERROR: unexpected end of file encountered.\n");
            exit(1);
        }
        else if(ch == '\"') {
            consume_char(fp);
            break;
        }
        else
            append_str_char(token.str, ch);

        ch = consume_char(fp);
    }
}

void consume_token() {

    int ch = EOF;
    int finished = 0;
    clear_str(token.str);
    token.tok = ERROR;

    while(!finished) {
        ch = get_char(fp);
        if(ch == EOF) {
            token.tok = EOI;
            finished++;
        }
        else if(isalpha(ch)) {
            get_name(ch);
            token.tok = NAME;
            finished++;
        }
        else if(isspace(ch)) {
            consume_space();
        }
        else if(ch == '#') {
            consume_comment();
        }
        else if(ch == ':') {
            append_str_char(token.str, ch);
            token.tok = COLON;
            consume_char(fp);
            finished++;
        }
        else if(ch == '(') {
            get_rule();
            token.tok = RULE;
            finished++;
        }
        else if(ch == '\"') {
            get_literal();
            token.tok = LITERAL;
            finished++;
        }
        else {
            fprintf(stderr, "ERROR: Unknown character: %c (%02X)\n", ch, ch);
            fprintf(stderr, "    %s: %d: %d\n",
                    get_fname(fp), get_line_no(fp), get_col_no(fp));
            exit(1);
        }
    }
}

const char* tok_to_str(TOK_TYPE tok) {

    return (tok == ERROR)? "ERROR" :
        (tok == NAME)? "NAME" :
        (tok == COLON)? "COLON" :
        (tok == RULE)? "RULE" :
        (tok == LITERAL)? "LITERAL" :
        (tok == EOI)? "EOI" : "UNKNOWN";
}
