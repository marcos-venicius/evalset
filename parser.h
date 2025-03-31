#ifndef PARSER_H_
#define PARSER_H_

#include <stddef.h>
#include "./lexer.h"

typedef struct Var Var;

typedef enum {
    VK_STRING = 0,
    VK_INTEGER,
    VK_FLOAT,
    VK_NIL,
    VK_BOOLEAN,

    VK_OBJECT,
    VK_ARRAY
} Var_Kind;

typedef struct {
    char *value;
} String;

typedef struct {
    int value; // 0 means false and 1 means true
} Boolean;

typedef struct {
    int value;
} Integer;

typedef struct {
    double value;
} Float;

typedef struct {
} Nil;

// TODO: it'll be a hashmap (own implemenation)
typedef struct {
} Object;

typedef struct {
    size_t capacity;
    size_t length;
    Var *data;
} Array;

struct Var {
    Var_Kind kind;
    char *name;

    union {
        String string;
        Boolean boolean;
        Integer integer;
        Float floating;
        Object object;
        Array array;
        Nil nil;
    };
};

typedef struct {
    size_t capacity;
    size_t length;
    Var *data;
} Parser;

Parser parse_tokens(Token *head);
void parser_free(Parser parser);

/*

Parser {
    data = [
        {
            name = "name"
            kind = string,
            string = {
                char *value
            }
        },
        {
            name = "version"
            kind = integer,
            integer = {
                int value
            }
        },
        {
            name = "float"
            kind = float,
            integer = {
                double value
            }
        },
        {
            name = "booleanTrue"
            kind = boolean,
            integer = {
                bool value
            }
        },
        {
            name = "booleanFalse"
            kind = boolean,
            integer = {
                boolean value
            }
        },
        {
            name = "nullable"
            kind = nil,
            nil = { }
        },
        {
            name = "dependencies"
            kind = array,
            array = [
                {
                    name = "0"
                    kind = object,
                    object = {
                        name = {
                            name = "name"
                            kind = string,
                            stirng = {
                                char *value;
                            }
                        }
                        version = {
                            name = "version"
                            kind = array,
                            array = [
                                {
                                    name = "0"
                                    kind = integer,
                                    integer = {
                                        int value;
                                    }
                                },
                                {
                                    name = "1"
                                    kind = integer,
                                    integer = {
                                        int value;
                                    }
                                }
                            ]
                        }
                    }
                }
            ]
        }
    ]
}


*/

#endif // PARSER_H_
