#include <strings.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ted_longjmp.h>

void die(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    
    vfprintf(stderr, fmt, va);
    
    va_end(va);
    TED_CALL_LONGJMP(TED_LONGJMP_DIE);
}

int string_to_bool(const char *s) {
    if (s[0] && !s[1]) {
        if (*s == 't' || *s == 'T' || *s == '1') return 1;
        if (*s == 'f' || *s == 'F' || *s == '0') return 0;
    }
    if (0 == strcasecmp(s, "true")) return 1;
    if (0 == strcasecmp(s, "false")) return 0;
    return -1;
}

char *printdup(const char *fmt, ...) {
    va_list va1, va2;
    char *ret = NULL;
    
    va_start(va1, fmt);
    va_copy(va2, va1);
    
    int len = vsnprintf(NULL, 0, fmt, va1);
    if (len <= 0) goto END;
    
    ret = malloc(len + 1);
    if (0 >= vsnprintf(ret, len + 1, fmt, va2)) ret = NULL;
    
END:
    va_end(va1);
    va_end(va2);
    
    return ret;
}

char *next_word(char **s) {
    size_t fstlen = 0;
    for (; (*s)[fstlen] && (*s)[fstlen] != ' '; fstlen++);
    
    while ((*s)[fstlen] == ' ')
        (*s)[fstlen++] = '\0';
    
    *s += fstlen;
    return *s - fstlen;
}
