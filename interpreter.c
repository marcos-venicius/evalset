#include "./parser.h"
#include "./interpreter.h"
#include "./map.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum {
    SK_NIL = 0,
    SK_INTEGER,
    SK_STRING,
    SK_FLOAT,
    SK_BOOLEAN,
} Symbol_Kind;

typedef union {
    Nil nil;
    Integer integer;
    String string;
    Float floating;
    Boolean boolean;
} Symbol_Value;

typedef struct {
    char *value;
    size_t size;
} Symbol_Name;

typedef struct {
    Symbol_Name name;
    Symbol_Kind kind;
    Symbol_Value as; // evaluated
} Symbol;

typedef Map* Symbols;

void interpret(const Var *vars, size_t length) {
    Symbols symbols = map_new();

    for (size_t i = 0; i < length; i++) {
        Var var = vars[i];

        Symbol symbol = {
            .name = {
                .value = var.name.value,
                .size = var.name.size
            },
        };

        switch (var.kind) {
            case VK_NIL: { symbol.kind = SK_NIL; } break;
            case VK_INTEGER: { symbol.kind = SK_INTEGER; symbol.as.integer = var.as.integer; } break;
            case VK_STRING: { symbol.kind = SK_STRING; symbol.as.string = var.as.string; } break;
            case VK_FLOAT: { symbol.kind = SK_FLOAT; symbol.as.floating = var.as.floating; } break;
            case VK_BOOLEAN: { symbol.kind = SK_BOOLEAN; symbol.as.boolean = var.as.boolean; } break;
            default: assert(0 && "kind not evaluated yet");
        }

        map_set(symbols, symbol.name.value, &symbol, sizeof(Symbol));
    }

    printf("Symbols table (%ld)\n", symbols->length);
    for (size_t i = 0; i < MAP_BUCKET_SIZE; ++i) {
        MapNode *node = symbols->nodes[i];

        while (node != NULL) {
            Symbol symbol = *(Symbol*)node->data;

            switch (symbol.kind) {
                case SK_NIL: {
                    printf("  %s = nil\n", symbol.name.value);
                } break;
                case SK_STRING: {
                    printf("  %s = %s\n", symbol.name.value, symbol.as.string.value);
                } break;
                case SK_INTEGER: {
                    printf("  %s = %lu\n", symbol.name.value, symbol.as.integer.value);
                } break;
                case SK_FLOAT: {
                    printf("  %s = %lf\n", symbol.name.value, symbol.as.floating.value);
                } break;
                case SK_BOOLEAN: {
                    printf("  %s = %s\n", symbol.name.value, symbol.as.boolean.value == 1 ? "true" : "false");
                } break;
                default: printf("unkonwn\n"); break;
            }
            node = node->next;
        }
    }

    map_free(symbols);
}
