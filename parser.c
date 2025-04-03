#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./parser.h"

Parser parse_tokens(Token *head) {
    if (head == NULL) return (Parser){0};

    Parser parser = {0};

    Var var = {
        .kind = VK_FLOAT,
        .name = "something",
        .floating = (Float){
            .value = 10.5
        }
    };

    array_append(&parser, var);
    var.name = "other";
    array_append(&parser, var);
    var.name = "one more stuff";
    array_append(&parser, var);
    var.kind = VK_STRING;
    var.name = "a string";
    var.string = (String){
        .value = "Hello, World!"
    };
    array_append(&parser, var);

    printf("data size: %zu\n", parser.length);
    for (size_t i = 0; i < parser.length; ++i) {
        switch (parser.data[i].kind) {
            case VK_STRING:
                printf("[string] %s: \"%s\"\n", parser.data[i].name, parser.data[i].string.value);
                break;
            case VK_FLOAT:
                printf("[float] %s: \"%lf\"\n", parser.data[i].name, parser.data[i].floating.value);
                break;
            default:
                break;
        }
    }

    array_free(&parser);


    return (Parser){0};
}

void parser_free(Parser parser) {
    (void)parser;
}

