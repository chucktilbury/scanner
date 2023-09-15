
#include <stdio.h>
#include <stdlib.h>

#include "tok.h"
#include "strs.h"

int main(int argc, char** argv) {

    if(argc < 2) {
        fprintf(stderr, "%s: need file name\n", argv[0]);
        exit(1);
    }

    init_tokens(argv[1]);

    while(crnt_token()->tok != EOI) {
        printf("%s: %s\n", tok_to_str(crnt_token()->tok), raw_str(crnt_token()->str));
        consume_token();
    }

    return 0;
}
