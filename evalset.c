#include <stdio.h>
#include <stdlib.h>
#include "./lexer.h"
#include "./parser.h"
#include "./io.h"

void print_var(Var var, bool is_inside_array) {
    switch (var.kind) {
        case VK_ARRAY: {
            printf(
                "%.*s = [",
                (int)var.name.size,
                var.name.value
            );
            for (size_t i = 0; i < var.array.length; ++i) {
                print_var(var.array.data[i], true);

                if (i < var.array.length - 1) printf(", ");
            }
            printf("]\n");
            break;
        };
        case VK_STRING: {
            if (is_inside_array) {
                printf(
                    "%.*s",
                    (int)var.string.size,
                    var.string.value
                );
            } else {
                printf(
                    "%.*s = %.*s\n",
                    (int)var.name.size,
                    var.name.value,
                    (int)var.string.size,
                    var.string.value
                );
            }
        } break;
        case VK_INTEGER: {
            if (is_inside_array) {
                printf("%ld", var.integer.value);
            } else {
                printf(
                    "%.*s = %ld\n",
                    (int)var.name.size,
                    var.name.value,
                    var.integer.value
                );
            }
        } break;
        case VK_FLOAT: {
            if (is_inside_array) {
                printf("%f", var.floating.value);
            } else {
                printf(
                    "%.*s = %f\n",
                    (int)var.name.size,
                    var.name.value,
                    var.floating.value
                );
            }
        } break;
        case VK_BOOLEAN: {
            if (is_inside_array) {
                printf("%s", var.boolean.value == 1 ? "true" : "false");
            } else {
                printf(
                    "%.*s = %s\n",
                    (int)var.name.size,
                    var.name.value,
                    var.boolean.value == 1 ? "true" : "false"
                );
            }
        } break;
        case VK_NIL: {
            if (is_inside_array) {
                printf("nil");
            } else {
                printf(
                    "%.*s = nil\n",
                    (int)var.name.size,
                    var.name.value
                );
            }
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

        print_var(var, false);
    }

    parser_free(parser);
    lexer_free(&lexer);

    return 0;
}
