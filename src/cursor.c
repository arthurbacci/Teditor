#include "ted.h"

void calculate_scroll(Buffer *buf, size_t screen_size) {
    // y
    if (buf->cursor.y < buf->scroll.y)
        buf->scroll.y = buf->cursor.y;
    if (buf->cursor.y > buf->scroll.y + LINES - 2)
        buf->scroll.y = buf->cursor.y - LINES;

    // x
    if (buf->cursor.x_width < buf->scroll.x_width)
        buf->scroll.x_width = buf->cursor.x_width;
    else if (buf->cursor.x_width > buf->scroll.x_width + screen_size)
        buf->scroll.x_width = buf->cursor.x_width - screen_size;
}

void truncate_cursor_x(Buffer *buf) {
    char *s = buf->lines[buf->cursor.y].data;
    // TODO: check if it is neg before incrementing

    ssize_t iw = index_by_width(buf->cursor.x_width, &s);
    if (iw > 0)
        buf->cursor.x_width -= iw;
}

