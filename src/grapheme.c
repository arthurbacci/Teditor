#include "ted.h"

/* Some functions built on top of suckless' libgrapheme */

Grapheme get_next_grapheme(char **str, size_t len) {
    Grapheme grapheme = {
        grapheme_next_character_break_utf8(*str, len),
        *str,
    };

    *str += grapheme.sz;

    return grapheme;
}

size_t grapheme_width(Grapheme g) {
    /*char *with_null = malloc(g.sz + 1);
    memcpy(with_null, g.dt, g.sz);
    with_null[g.sz] = '\0';

    // The UTF-32 representation can't be bigger than the UTF-8 one
    wchar_t *ws = malloc((g.sz + 1) * sizeof(wchar_t));

    size_t wslen = mbstowcs(ws, with_null, g.sz + 1);
    if (wslen == -1) {
        free(with_null);
        free(ws);
        return -1;
    }

    size_t width = wcswidth(ws, wslen);

    free(with_null);
    free(ws);
    return width;*/
    return 1;
}
