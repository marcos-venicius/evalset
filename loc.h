#ifndef _LOC_H_
#define _LOC_H_

#define LOC_ERROR_FMT "%s:%d:%d: \033[1;31merror\033[0m"
#define LOC_ERROR_ARG(loc) loc.filename, loc.line, loc.col

typedef struct {
    unsigned int line, col;
    const char *filename;
} Location;

#endif // !_LOC_H_
