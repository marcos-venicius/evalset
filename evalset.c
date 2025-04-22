#include <stdio.h>
#include <stdlib.h>
#include "./lexer.h"
#include "./parser.h"
#include "./io.h"

void print_var(Var var) {
    switch (var.kind) {
        case VK_ARRAY: {
            printf(
                "%.*s = [",
                (int)var.name.size,
                var.name.value
            );
            for (size_t i = 0; i < var.array.length; ++i) {
                print_var(var.array.data[i]);
            }
            printf("]\n");
            break;
        };
        case VK_STRING: {
            printf(
                "%.*s = %.*s\n",
                (int)var.name.size,
                var.name.value,
                (int)var.string.size,
                var.string.value
            );
        } break;
        case VK_INTEGER: {
            printf(
                "%.*s = %ld\n",
                (int)var.name.size,
                var.name.value,
                var.integer.value
            );
        } break;
        case VK_FLOAT: {
            printf(
                "%.*s = %f\n",
                (int)var.name.size,
                var.name.value,
                var.floating.value
            );
        } break;
        case VK_BOOLEAN: {
            printf(
                "%.*s = %s\n",
                (int)var.name.size,
                var.name.value,
                var.boolean.value == 1 ? "true" : "false"
            );
        } break;
        case VK_NIL: {
            printf(
                "%.*s = nil\n",
                (int)var.name.size,
                var.name.value
            );
        } break;
        default: {
            printf("unimplemented printing for kind: %s\n", var_kind_name(var.kind));
            exit(1);
        } break;
    }
}

int main(void) {
    const char *filename = "./examples/settings.es";

    char *content;

    size_t data_size = read_from_file(filename, &content);

    Lexer lexer = create_lexer(filename, content, data_size);

    Token *head;

    if ((head = lex(&lexer))) {
        print_tokens(head);
    }

    Parser parser = parse_tokens(head);

    for (size_t i = 0; i < parser.length; i++) {
        Var var = parser.data[i];

        print_var(var);
    }

    parser_free(parser);
    lexer_free(&lexer);

    return 0;
}
