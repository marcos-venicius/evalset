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
#define s_strcmp(a, b) cmp_sized_strings(a, strlen(a), b, strlen(b))

char *shift(int *argc, char ***argv) {
    if ((*argc)-- == 0) return NULL;

    return *((*argv)++);
}

void usage(FILE *stream, const char *program_name) {
    fprintf(stream, "usage: %s <filename> [--format|--to-json]\n", program_name);
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

    Ast ast = parse_tokens(head);

    if (flag != NULL && s_strcmp(flag, "--format")) {
        for (size_t i = 0; i < ast.length; i++) {
            Var var = ast.vars[i];

            print_var(var, false, 0);

            if (i < ast.length - 1) printf("\n");
        }
    } else if (flag != NULL && s_strcmp(flag, "--to-json")) {
        to_json(ast.vars, ast.length);
    } else {
        interpret(ast.vars, ast.length);
    }

    ast_free(ast);
    lexer_free(&lexer);

    return 0;
}
