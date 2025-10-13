#include "./getter.h"
#include "map.h"
#include "parser.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    _DK_NIL = 0,
    _DK_INTEGER,
    _DK_STRING,
    _DK_FLOAT,
    _DK_BOOLEAN,
    _DK_ARRAY,
    _DK_OBJECT,
    _DK_SYMBOLS
} _Data_Kind;

typedef struct {
    _Data_Kind kind;

    union {
        Nil nil;
        Integer integer;
        String string;
        Float floating;
        Boolean boolean;
        Array array;
        Object object;
        Symbols symbols;
    } as;
} _Data_Type;

static _Data_Type load_data_type(Symbols symbols, va_list args) {
    _EsArgType type;

    _Data_Type dt = {
        .as.symbols = symbols,
        .kind = _DK_SYMBOLS
    };

    while ((type = va_arg(args, _EsArgType)) != ES_END) {
        switch (type) {
            case ES_ARG_NUMBER: {
                size_t value = va_arg(args, long);

                if (dt.kind != _DK_ARRAY) {
                    fprintf(stderr, "error: cannot index a non-array object\n"); // TODO: improve error messages
                    exit(1);
                }

                if (value < 0 || value >= dt.as.array.length) {
                    fprintf(stderr, "error: index %ld out of bounds. array have length %ld\n", value, dt.as.array.length);
                    exit(1);
                }

                Argument arg = dt.as.array.data[value];

                switch (arg.kind) {
                    case AK_NIL: {
                        dt.kind = _DK_NIL;
                    } break;
                    case AK_INTEGER: {
                        dt.kind = _DK_INTEGER;
                        dt.as.integer = arg.as.integer;
                    } break;
                    case AK_STRING: {
                        dt.kind = _DK_STRING;
                        dt.as.string = arg.as.string;
                    } break;
                    case AK_FLOAT: {
                        dt.kind = _DK_FLOAT;
                        dt.as.floating = arg.as.floating;
                    } break;
                    case AK_BOOLEAN: {
                        dt.kind = _DK_BOOLEAN;
                        dt.as.boolean = arg.as.boolean;
                    } break;
                    case AK_OBJECT: {
                        dt.kind = _DK_OBJECT;
                        dt.as.object = arg.as.object;
                    } break;
                    case AK_ARRAY: {
                        dt.kind = _DK_ARRAY;
                        dt.as.array = arg.as.array;
                    } break;
                    default: {
                        fprintf(stderr, "error: unexpected index type\n");
                        exit(1);
                    } break;
                }
            } break;
            case ES_ARG_STRING: {
                char *str = va_arg(args, char *);

                switch (dt.kind) {
                    case _DK_SYMBOLS: {
                        Symbol* symbol = map_get(dt.as.symbols, str);

                        if (symbol == NULL) {
                            fprintf(stderr, "error: key \"%s\" not found\n", str);
                            exit(1);
                        }

                        switch (symbol->value.kind) {
                            case SK_NIL: {
                                dt.kind = _DK_NIL;
                            } break;
                            case SK_INTEGER: {
                                dt.kind = _DK_INTEGER;
                                dt.as.integer = symbol->value.as.integer;
                            } break;
                            case SK_STRING: {
                                dt.kind = _DK_STRING;
                                dt.as.string = symbol->value.as.string;
                            } break;
                            case SK_FLOAT: {
                                dt.kind = _DK_FLOAT;
                                dt.as.floating = symbol->value.as.floating;
                            } break;
                            case SK_BOOLEAN: {
                                dt.kind = _DK_BOOLEAN;
                                dt.as.boolean = symbol->value.as.boolean;
                            } break;
                            case SK_ARRAY: {
                                dt.kind = _DK_ARRAY;
                                dt.as.array = symbol->value.as.array;
                            } break;
                            case SK_OBJECT: {
                                dt.kind = _DK_OBJECT;
                                dt.as.object = symbol->value.as.object;
                            } break;
                        }
                    } break;
                    case _DK_OBJECT: {
                        size_t str_size = strlen(str);
                        bool found = false;

                        for (size_t i = 0; i < dt.as.object.length; ++i) {
                            Var var = dt.as.object.data[i];

                            if (cmp_sized_strings(var.name.value, var.name.size, str, str_size)) {
                                found = true;

                                switch (var.kind) {
                                    case VK_NIL: {
                                        dt.kind = _DK_NIL;
                                    } break;
                                    case VK_STRING: {
                                        dt.kind = _DK_STRING;
                                        dt.as.string = var.as.string;
                                    } break;
                                    case VK_INTEGER: {
                                        dt.kind = _DK_INTEGER;
                                        dt.as.integer = var.as.integer;
                                    } break;
                                    case VK_FLOAT: {
                                        dt.kind = _DK_FLOAT;
                                        dt.as.floating = var.as.floating;
                                    } break;
                                    case VK_BOOLEAN: {
                                        dt.kind = _DK_BOOLEAN;
                                        dt.as.boolean = var.as.boolean;
                                    } break;
                                    case VK_ARRAY: {
                                        dt.kind = _DK_ARRAY;
                                        dt.as.array = var.as.array;
                                    } break;
                                    case VK_OBJECT: {
                                        dt.kind = _DK_OBJECT;
                                        dt.as.object = var.as.object;
                                    } break;
                                    default: {
                                        fprintf(stderr, "error: unexpected object value type\n");
                                        exit(1);
                                    } break;
                                }

                                break;
                            }
                        }

                        if (!found) {
                            fprintf(stderr, "error: key \"%s\" not found\n", str);
                            exit(1);
                        }
                    } break;
                    default: {
                        fprintf(stderr, "error: could not handle string key\n");
                        exit(1);
                    }
                }
            } break;
            default: break;
        }
    }

    return dt;
}

long es_get_integer(Symbols symbols, ...)
{
    va_list args;
    va_start(args, symbols);

    _Data_Type dt = load_data_type(symbols, args);

    va_end(args);

    if (dt.kind != _DK_INTEGER) {
        fprintf(stderr, "error: object type is not an integer\n");
        exit(1);
    }

    return dt.as.integer.value;
}

bool es_get_boolean(Symbols symbols, ...)
{
    va_list args;
    va_start(args, symbols);

    _Data_Type dt = load_data_type(symbols, args);

    va_end(args);

    if (dt.kind != _DK_BOOLEAN) {
        fprintf(stderr, "error: object type is not a boolean\n");
        exit(1);
    }

    return dt.as.boolean.value;
}

double es_get_float(Symbols symbols, ...)
{
    va_list args;
    va_start(args, symbols);

    _Data_Type dt = load_data_type(symbols, args);

    va_end(args);

    if (dt.kind != _DK_FLOAT) {
        fprintf(stderr, "error: object type is not a float\n");
        exit(1);
    }

    return dt.as.floating.value;
}

String es_get_string(Symbols symbols, ...)
{
    va_list args;
    va_start(args, symbols);

    _Data_Type dt = load_data_type(symbols, args);

    va_end(args);

    if (dt.kind != _DK_STRING) {
        fprintf(stderr, "error: object type is not a string\n");
        exit(1);
    }

    return dt.as.string;
}
