#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lexer.h"
#include "./parser.h"
#include "./io.h"
#include "./print.h"
#include "./interpreter.h"
#include "map.h"
#include "utils.h"
#include "./getter.h"

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






























        Symbols symbols = interpret(parser.vars, parser.length);

        long result = es_get_integer(
            symbols,
            ES_KEY_ARG("assets"),
            ES_INDEX_ARG(0),
            ES_KEY_ARG("size"),
            ES_KEY_ARG("width"),
            ES_ARG_END
        );

        bool active = es_get_boolean(
            symbols,
            ES_KEY_ARG("is_true"),
            ES_ARG_END
        );

        double money = es_get_float(
            symbols,
            ES_KEY_ARG("assets"),
            ES_INDEX_ARG(3),
            ES_KEY_ARG("money"),
            ES_ARG_END
        );
        
        String second_path = es_get_string(
            symbols,
            ES_KEY_ARG("assets"),
            ES_INDEX_ARG(1),
            ES_KEY_ARG("path"),
            ES_ARG_END
        );

        printf("result: %ld\n", result);
        printf("active: %s\n", active ? "true" : "false");
        printf("money: %lf\n", money);
        printf("path: [%.*s]\n", (int)second_path.size, second_path.value);























































        map_free(symbols);
    }

    parser_free(parser);
    lexer_free(&lexer);

    return 0;
}
