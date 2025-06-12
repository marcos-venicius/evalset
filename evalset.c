#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "./lexer.h"
#include "./parser.h"
#include "./io.h"

void print_var(Var var, bool is_inside_array, int level);

static void print_argument(Argument argument, int level) {
    switch (argument.kind) {
        case AK_NIL: {
            printf("%*.snil", level, "");
        } break;
        case AK_INTEGER: {
            printf("%*.s%ld", level, "", argument.as.integer.value);
        } break;
        case AK_STRING: {
            printf("%*.s%.*s", level, "", (int)argument.as.string.size, argument.as.string.value);
        } break;
        case AK_ARRAY: {
            printf("%*.s[", level, "");
            if (argument.as.array.length > 0) printf("\n");
            for (size_t i = 0; i < argument.as.array.length; ++i) {
                print_argument(argument.as.array.data[i], level + 2);

                if (i < argument.as.array.length - 1) printf(",\n");
            }
            if (argument.as.array.length > 0) printf("\n%*.s", level, "");
            printf("]");
        } break;
        case AK_BOOLEAN: {
            printf("%*.s%s", level, "", argument.as.boolean.value == 1 ? "true" : "false");
        } break;
        case AK_FLOAT: {
            printf("%*.s%f", level, "", argument.as.floating.value);
        } break;
        case AK_OBJECT: {
            printf("%*.s{", level, "");
            if (argument.as.object.length > 0) printf("\n");
            for (size_t i = 0; i < argument.as.object.length; ++i) {
                print_var(argument.as.object.data[i], false, level + 2);

                if (i < argument.as.object.length - 1) printf(",\n");
            }
            if (argument.as.object.length > 0) printf("\n%*.s", level, "");
            printf("}");
        } break;
        case AK_PATH: {
            printf("%*.s$", level, "");

            for (size_t i = 0; i < argument.as.path.length; ++i) {
                String string = argument.as.path.data[i];

                printf("%.*s", (int)string.size, string.value);
            }
        } break;
        case AK_FUN_CALL: {
            printf("%.*s(", (int)argument.as.fun_call->name.size, argument.as.fun_call->name.value);
            for (size_t i = 0; i < argument.as.fun_call->arguments.length; ++i) {
                if (i > 0) printf(", ");

                print_argument(argument.as.fun_call->arguments.data[i], level);
            }
            printf(")");
        } break;
        default: assert(0 && "fix print_argument");
    }
}

void print_var(Var var, bool is_inside_array, int level) {
    switch (var.kind) {
        case VK_FUN_CALL: {
            printf("%.*s = %.*s(", (int)var.name.size, var.name.value, (int)var.as.fun_call->name.size, var.as.fun_call->name.value);
            for (size_t i = 0; i < var.as.fun_call->arguments.length; ++i) {
                if (i > 0) printf(", ");

                print_argument(var.as.fun_call->arguments.data[i], level);
            }
            printf(")");
        } break;
        case VK_ARRAY: {
            if (is_inside_array) {
                printf("%*.s[", level, "");
            } else {
                printf(
                    "%*.s%.*s = [",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value
                );
            }
            if (var.as.array.length > 0) printf("\n");
            for (size_t i = 0; i < var.as.array.length; ++i) {
                print_argument(var.as.array.data[i], level + 2);

                if (i < var.as.array.length - 1) printf(",\n");
            }
            if (var.as.array.length > 0) printf("\n%*.s", level, "");
            if (is_inside_array) {
                printf("]");
            } else {
                printf("]");
            }
            break;
        };
        case VK_OBJECT: {
            if (is_inside_array) {
                printf("%*.s{\n", level, "");
            } else {
                printf(
                    "%*.s%.*s = {",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value
                );
            }
            if (var.as.object.length > 0) printf("\n");
            for (size_t i = 0; i < var.as.object.length; ++i) {
                print_var(var.as.object.data[i], false, level + 2);

                if (i < var.as.object.length - 1) printf(",\n");
            }
            if (var.as.object.length > 0) printf("\n%*.s", level, "");
            if (is_inside_array) {
                printf("}");
            } else {
                printf("}");
            }
            break;
        };
        case VK_STRING: {
            if (is_inside_array) {
                printf(
                    "%*.s%.*s",
                    level,
                    "",
                    (int)var.as.string.size,
                    var.as.string.value
                );
            } else {
                printf(
                    "%*.s%.*s = %.*s",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value,
                    (int)var.as.string.size,
                    var.as.string.value
                );
            }
        } break;
        case VK_INTEGER: {
            if (is_inside_array) {
                printf("%*.s%ld", level, "", var.as.integer.value);
            } else {
                printf(
                    "%*.s%.*s = %ld",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value,
                    var.as.integer.value
                );
            }
        } break;
        case VK_FLOAT: {
            if (is_inside_array) {
                printf("%*.s%f", level, "", var.as.floating.value);
            } else {
                printf(
                    "%*.s%.*s = %f",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value,
                    var.as.floating.value
                );
            }
        } break;
        case VK_BOOLEAN: {
            if (is_inside_array) {
                printf("%*.s%s", level, "", var.as.boolean.value == 1 ? "true" : "false");
            } else {
                printf(
                    "%*.s%.*s = %s",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value,
                    var.as.boolean.value == 1 ? "true" : "false"
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

                for (size_t i = 0; i < var.as.path.length; ++i) {
                    String string = var.as.path.data[i];
                    printf("%.*s", (int)string.size, string.value);
                }
            } else {
                printf(
                    "%*.s%.*s = $",
                    level,
                    "",
                    (int)var.name.size,
                    var.name.value
                );
                for (size_t i = 0; i < var.as.path.length; ++i) {
                    String string = var.as.path.data[i];
                    printf("%.*s", (int)string.size, string.value); // TODO: why the print works fine even without null char
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
