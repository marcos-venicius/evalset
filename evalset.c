#include <stdio.h>
#include <stdlib.h>
#include "./lexer.h"
#include "./parser.h"
#include "./io.h"

void print_var(Var var, bool is_inside_array, int level) {
    switch (var.kind) {
        case VK_FUN_CALL: {
            printf("%.*s = %.*s()", (int)var.name.size, var.name.value, (int)var.func_call.name.size, var.func_call.name.value);
        } break;
        case VK_ARRAY: {
            if (is_inside_array) {
                printf("%*.s[\n", level, "");
            } else {
                printf(
                    "%*.s%.*s = [\n",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value
                );
            }
            for (size_t i = 0; i < var.array.length; ++i) {
                print_var(var.array.data[i], true, level + 2);

                if (i < var.array.length - 1) printf(",\n");
            }
            if (is_inside_array) {
                printf("\n%*.s]", level, "");
            } else {
                printf("\n%*.s]", level, "");
            }
            break;
        };
        case VK_OBJECT: {
            if (is_inside_array) {
                printf("%*.s{\n", level, " ");
            } else {
                printf(
                    "%*.s%.*s = {\n",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value
                );
            }
            for (size_t i = 0; i < var.array.length; ++i) {
                print_var(var.array.data[i], false, level + 2);

                if (i < var.array.length - 1) printf(",\n");
            }
            if (is_inside_array) {
                printf("\n%*.s}", level, "");
            } else {
                printf("\n%*.s}", level, "");
            }
            break;
        };
        case VK_STRING: {
            if (is_inside_array) {
                printf(
                    "%*.s%.*s",
                    level,
                    "",
                    (int)var.string.size,
                    var.string.value
                );
            } else {
                printf(
                    "%*.s%.*s = %.*s",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value,
                    (int)var.string.size,
                    var.string.value
                );
            }
        } break;
        case VK_INTEGER: {
            if (is_inside_array) {
                printf("%*.s%ld", level, "", var.integer.value);
            } else {
                printf(
                    "%*.s%.*s = %ld",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value,
                    var.integer.value
                );
            }
        } break;
        case VK_FLOAT: {
            if (is_inside_array) {
                printf("%*.s%f", level, "", var.floating.value);
            } else {
                printf(
                    "%*.s%.*s = %f",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value,
                    var.floating.value
                );
            }
        } break;
        case VK_BOOLEAN: {
            if (is_inside_array) {
                printf("%*.s%s", level, "", var.boolean.value == 1 ? "true" : "false");
            } else {
                printf(
                    "%*.s%.*s = %s",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value,
                    var.boolean.value == 1 ? "true" : "false"
                );
            }
        } break;
        case VK_NIL: {
            if (is_inside_array) {
                printf("%*.snil", level, "");
            } else {
                printf(
                    "%*.s%.*s = nil",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value
                );
            }
        } break;
        case VK_PATH: {
            if (is_inside_array) {
                printf("%*.s$", level, "");

                for (size_t i = 0; i < var.path.length; ++i) {
                    printf("%s", (char*)var.path.data[i]); // TODO: why the print works fine even without null char
                }
            } else {
                printf(
                    "%*.s%.*s = $",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value
                );
                for (size_t i = 0; i < var.path.length; ++i) {
                    printf("%s", (char*)var.path.data[i]); // TODO: why the print works fine even without null char
                }
            }
        } break;
        default: {
            printf("unimplemented printing for kind: %s\n", var_kind_name(var.kind));
            exit(1);
        } break;
    }

    if (level == 0) printf("\n");
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

        print_var(var, false, 0);

        if (i < parser.length - 1) printf("\n");
    }

    parser_free(parser);
    lexer_free(&lexer);

    return 0;
}
