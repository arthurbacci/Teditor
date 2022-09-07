#include "ted.h"

void calculate_scroll(Buffer *buf, size_t screen_size) {
    // y
    if (buf->cursor.y < buf->scroll.y)
        buf->scroll.y = buf->cursor.y;
    if (buf->cursor.y > buf->scroll.y + LINES - 2)
        buf->scroll.y = buf->cursor.y - LINES;
    // x
    if (buf->cursor.x_grapheme < buf->scroll.x_grapheme) {
        buf->scroll.x_grapheme = buf->cursor.x_grapheme;
    } else if (buf->cursor.x_grapheme >= buf->scroll.x_grapheme + screen_size)
        buf->scroll.x_grapheme = buf->cursor.x_grapheme - screen_size;
    /*
    else if (buf->cursor.x > buf->scroll.x + (COLS - len_line_number - 2)) {
        buf->scroll.x = buf->cursor.x - (COLS - len_line_number - 2);
    }*/
}