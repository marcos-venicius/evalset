#ifndef GETTER_H_
#define GETTER_H_

#include "symbols.h"

typedef enum {
    ES_ARG_NUMBER,
    ES_ARG_STRING,
    ES_ARG_END
} EsArgType;

#define ES_NUMBER_ARG(value) ES_ARG_NUMBER, (size_t)(value)
#define ES_STRING_ARG(value) ES_ARG_STRING, (value)

long es_get_integer(Symbols symbols, ...);
bool es_get_boolean(Symbols symbols, ...);

#endif // !GETTER_H_
