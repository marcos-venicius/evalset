#include "./utils.h"
#include <string.h>

bool cmp_sized_strings(const char *a, size_t as, const char *b, size_t bs) {
    if (as != bs) return false;

    return strncmp(a, b, as) == 0;
}
