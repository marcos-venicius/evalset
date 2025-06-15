#include "./parser.h"
#include "./interpreter.h"
#include "./utils.h"
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

typedef struct {
    size_t length, capacity;
    Symbol *data;
} Symbols; // TODO: use a hashmap

void interpret(const Var *vars, size_t length) {
    Symbols symbols = {0};

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

        int update_index = -1;

        // TODO: **high priority** implement a hash map
        for (size_t j = 0; j < i; ++j) {
            Symbol sym = symbols.data[j];

            if (cmp_sized_strings(sym.name.value, sym.name.size, symbol.name.value, symbol.name.size)) {
                update_index = j;
                break;
            }
        }

        if (update_index != -1) {
            symbols.data[update_index] = symbol;
        } else {
            array_append(&symbols, symbol);
        }
    }

    printf("Symbols table:\n");
    for (size_t i = 0; i < symbols.length; ++i) {
        Symbol symbol = symbols.data[i];

        switch (symbol.kind) {
            case SK_NIL: {
                printf("  %.*s = nil\n", (int)symbol.name.size, symbol.name.value);
            } break;
            case SK_STRING: {
                printf("  %.*s = %.*s\n", (int)symbol.name.size, symbol.name.value, (int)symbol.as.string.size, symbol.as.string.value);
            } break;
            case SK_INTEGER: {
                printf("  %.*s = %lu\n", (int)symbol.name.size, symbol.name.value, symbol.as.integer.value);
            } break;
            case SK_FLOAT: {
                printf("  %.*s = %lf\n", (int)symbol.name.size, symbol.name.value, symbol.as.floating.value);
            } break;
            case SK_BOOLEAN: {
                printf("  %.*s = %s\n", (int)symbol.name.size, symbol.name.value, symbol.as.boolean.value == 1 ? "true" : "false");
            } break;
            default: printf("unkonwn\n"); break;
        }
    }
}
