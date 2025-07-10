#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lexer.h"
#include "./parser.h"
#include "./io.h"
#include "./print.h"
#include "./interpreter.h"
#include "utils.h"

#define arg() shift(&argc, &argv)

char *shift(int *argc, char ***argv) {
    if ((*argc)-- == 0) return NULL;

    return *((*argv)++);
}

void usage(FILE *stream, const char *program_name) {
    fprintf(stream, "usage: %s <filename> [--format]\n", program_name);
}

int main(int argc, char **argv) {
    const char *program_name = arg();
    const char *filename = arg();
    const char *flag = arg();

    if (filename == NULL) {
        usage(stderr, program_name);

        return 1;
    }

    char *content;

    size_t data_size = read_from_file(filename, &content);

    Lexer lexer = create_lexer(filename, content, data_size);

    Token *head;

    if ((head = lex(&lexer))) {
        print_tokens(head);
    }

    Parser parser = parse_tokens(head);

    if (flag != NULL && cmp_sized_strings(flag, strlen(flag), "--format", 8)) {
        for (size_t i = 0; i < parser.length; i++) {
            Var var = parser.vars[i];

            print_var(var, false, 0);

            if (i < parser.length - 1) printf("\n");
        }
    } else {
        interpret(parser.vars, parser.length);
    }

    parser_free(parser);
    lexer_free(&lexer);

    return 0;
}
