#ifndef GETTER_H_
#define GETTER_H_

#include "symbols.h"

typedef enum {
    ES_ARG_NUMBER,
    ES_ARG_STRING,
    ES_ARG_END
} EsArgType;

#define ES_INDEX_ARG(value) ES_ARG_NUMBER, (size_t)(value)
#define ES_KEY_ARG(value) ES_ARG_STRING, (value)

long es_get_integer(Symbols symbols, ...);
bool es_get_boolean(Symbols symbols, ...);
double es_get_float(Symbols symbols, ...);
String es_get_string(Symbols symbols, ...);

#endif // !GETTER_H_
