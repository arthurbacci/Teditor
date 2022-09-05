#include "ted.h"

// Calculates `x` from `x_grapheme`, truncating both if needed
void calculate_cursor_x(Buffer *buf) {
    char *s = buf->lines[buf->cursor.y].data;

    size_t i;
    for (i = 0; i < buf->cursor.x_grapheme; i++) {
        if (*s == '\0')
            break;

        size_t off = grapheme_next_character_break_utf8(s, SIZE_MAX);
        s += off;

        if (0 == off)
            break;
    }

    buf->cursor.x_grapheme = i;
    buf->cursor.x = s - buf->lines[buf->cursor.y].data;
}
