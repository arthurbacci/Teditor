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
