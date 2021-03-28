#include "ted.h"

char *home_path(const char *path) {
    char *ret = malloc(1000 * sizeof *ret);
    snprintf(ret, 1000, "%s/%s", getenv("HOME"), path);
    
    return ret;
}

/*
this functions is similar to strtok_r, but tokenizes only on spaces
*/
char *split_spaces(char *str, char **save) {
    if (str == NULL) {
        if (*save == NULL)
            return NULL;
        str = *save;
    }

    if (*str == '\0') {
        *save = str;
        return NULL;
    }

    char *end = strchr(str, ' ');
    if (end == NULL) {
        *save = end;
        return str;
    }

    *end = '\0';
    *save = end + 1;
    // skip spaces
    while (isspace(**save)) (*save)++;
    return str;
}
