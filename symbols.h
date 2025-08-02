#ifndef SYMBOLS_H_
#define SYMBOLS_H_

#include "./parser.h"
#include "./map.h"

typedef enum {
    SK_NIL = 0,
    SK_INTEGER,
    SK_STRING,
    SK_FLOAT,
    SK_BOOLEAN,
    SK_ARRAY,
    SK_OBJECT,
} Symbol_Kind;

typedef union {
    Nil nil;
    Integer integer;
    String string;
    Float floating;
    Boolean boolean;
    Array array;
    Object object;
} Symbol_Data_Types;

typedef struct {
    char *value;
    size_t size;
} Symbol_Name;

typedef struct {
    Symbol_Kind kind;
    Symbol_Data_Types as; // evaluated
} Symbol_Value;

typedef struct {
    Symbol_Name name;
    Symbol_Value value;
} Symbol;

typedef Map* Symbols;

#endif // !SYMBOLS_H_
