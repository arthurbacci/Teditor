#include "ted.h"

size_t calculate_from_grapheme(size_t *gi, char *s0) {
    char *s = s0;

    size_t i;
    for (i = 0; i < *gi; i++) {
        if ('\0' == *s)
            break;

        size_t off = grapheme_next_character_break_utf8(s, SIZE_MAX);
        s += off;

        if (0 == off)
            break;
    }

    *gi = i;
    return s - s0;
}

// Calculates `x` from `x_grapheme`, truncating both if needed
void calculate_cursor_x(Buffer *buf) {
    buf->cursor.x = calculate_from_grapheme(
        &buf->cursor.x_grapheme,
        buf->lines[buf->cursor.y].data
    );
}
