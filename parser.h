#ifndef PARSER_H_
#define PARSER_H_

#include <stddef.h>
#include "./lexer.h"

#define DEFAULT_ARRAY_CAPACITY 100

typedef struct Var Var;
typedef struct Fun_Call Fun_Call;
typedef struct Argument Argument;

typedef enum {
    VK_NIL = 0,
    VK_STRING,
    VK_INTEGER,
    VK_FLOAT,
    VK_BOOLEAN,

    VK_ARRAY,
    VK_OBJECT,
    VK_PATH,
    VK_FUN_CALL,
    VK_SUM // HAVE TO BE THE LAST
} Var_Kind;

typedef enum {
    AK_NIL = VK_SUM,
    AK_INTEGER,
    AK_STRING,
    AK_FLOAT,
    AK_BOOLEAN,
    AK_PATH,
    AK_OBJECT,
    AK_ARRAY,
    AK_FUN_CALL,
    AK_SUM // HAVE TO BE THE LAST
} Argument_Kind;

typedef struct {
    // sized string
    char *value;
    size_t size;
} String;

typedef struct {
    int value; // 0 means false and 1 means true
} Boolean;

typedef struct {
    long value;
} Integer;

typedef struct {
    // TODO: add precision here (then update the display to the fixed precision)
    double value;
} Float;

// TODO: it'll be a hashmap (own implemenation)
typedef struct {
    size_t capacity;
    size_t length;
    Var *data;
} Object;

typedef struct {
    size_t capacity;
    size_t length;
    Argument *data;
} Array;

typedef struct {
    size_t capacity;
    size_t length;
    // Each string in this path have a new allocation, free it yourself
    String *data;
} Path;

typedef void* Nil;

typedef union {
    Integer integer;
    String string;
    Float floating;
    Boolean boolean;
    Path path;
    Object object;
    Array array;
    Fun_Call *fun_call;
} Argument_Data_Types;

struct Argument {
    Argument_Kind kind;

    Location loc;

    Argument_Data_Types as;
};

struct Fun_Call {
    String name;

    struct {
        size_t length, capacity;

        Argument *data;
    } arguments;
};

typedef union {
    String string;
    Boolean boolean;
    Integer integer;
    Float floating;
    Object object;
    Array array;
    Path path;
    Fun_Call *fun_call;
    Nil nil;
} Var_Data_Types;

struct Var {
    Var_Kind kind;
    // In this specific case, there is a new allocation, so be aware you need to free yourself
    String name;

    Location loc;

    Var_Data_Types as;
};

typedef struct {
    size_t length, capacity;
    union {
        Var *data;
        Var *vars;
    };
} Parser;

#define array_append(array, item) do { \
    if ((array)->length >= (array)->capacity) { \
        if ((array)->capacity == 0) (array)->capacity = DEFAULT_ARRAY_CAPACITY; \
        else (array)->capacity = (array)->capacity * 2; \
        void *output = realloc((array)->data, (array)->capacity * sizeof((array)->data[0])); \
        assert(output != NULL && "failed to reallocate array"); \
        (array)->data = output; \
    } \
    (array)->data[(array)->length++] = item; \
} while (0);

#define array_free(array) do { \
    free((array)->data); \
    (array)->capacity = DEFAULT_ARRAY_CAPACITY; \
    (array)->length = 0; \
} while (0);

#define array_flush(array) (array)->length = 0;

Parser parse_tokens(Token *head);
void parser_free(Parser parser);
const char *var_kind_name(Var_Kind var_kind);
const char *argument_kind_name(Argument_Kind kind);

#endif // PARSER_H_
