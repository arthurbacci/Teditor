#include "ted.h"

void calculate_scroll(Buffer *buf, size_t screen_size) {
    // y
    if (buf->cursor.y < buf->scroll.y)
        buf->scroll.y = buf->cursor.y;
    if (buf->cursor.y > buf->scroll.y + LINES - 2)
        buf->scroll.y = buf->cursor.y - LINES;

    char *thisln = buf->lines[buf->cursor.y].data;
    size_t x_width = gi_to_wi(buf->cursor.x_grapheme, thisln); 

    // x
    if (x_width < buf->scroll.x) {
        buf->scroll.x = x_width;
        message("A");
    } else if (x_width > buf->scroll.x + screen_size) {
        buf->scroll.x = x_width - screen_size;
        message("B");
    }
}