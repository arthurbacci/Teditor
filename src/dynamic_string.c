#include "ted.h"

#define STEP 100

String dynamic_string(const char *x, size_t length) {
    size_t cap = length + STEP;
    char *s = malloc(cap);
    memcpy(s, x, length);
    String r = {s, length, cap};
    return r;
}

void dynamic_string_check_available(String *s, size_t length) {
    if (s->len + length > s->cap) {
        s->cap = s->len + length + STEP;
        char *olds = s->start;
        s->start = malloc(s->cap);
        memcpy(s->start, olds, s->len);
        free(olds);
    }
}

void dynamic_string_push(String *s, char x) {
    dynamic_string_check_available(s, 1);
    s->start[s->len++] = x;
}

void dynamic_string_push_str(String *s, const char *x, size_t length) {
    dynamic_string_check_available(s, length);
    memcpy(s->start + s->len, x, length);
    s->len += length;
}

bool dynamic_string_pop(String *s) {
    if (s->len == 0)
        return false;
    s->len -= 1;
    return true;
}

void dynamic_string_free(String *s) {
    free(s->start);
}

char *dynamic_string_to_str(String s) {
    char *d = malloc(s.len + 1);
    memcpy(d, s.start, s.len);
    d[s.len] = '\0';
    return d;
}

String dynamic_string_concat(String a, String b) {
    size_t len = a.len + b.len;
    char *s = malloc(len);
    memcpy(s        , a.start, a.len);
    memcpy(s + a.len, b.start, b.len);
    String r = dynamic_string(s, len);
    free(s);
    return r;
}

bool dynamic_string_eq(String a, String b) {
    if (a.len != b.len)
        return 0;
    for (size_t i = 0; i < a.len; i++)
        if (a.start[i] != b.start[i])
            return 0;
    return 1;
}

bool dynamic_string_starts_with(String a, String b) {
    for (size_t i = 0; i < b.len; i++)
        if (a.start[i] != b.start[i])
            return 0;
    return 1;
}
