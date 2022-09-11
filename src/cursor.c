#include "ted.h"

void calculate_scroll(Buffer *buf, size_t screen_width) {
    // y
    if (buf->cursor.y < buf->scroll.y)
        buf->scroll.y = buf->cursor.y;
    if (buf->cursor.y >= buf->scroll.y + SROW)
        buf->scroll.y = buf->cursor.y + 1 - SROW;

    // x
    if (buf->cursor.x_width < buf->scroll.x_width)
        buf->scroll.x_width = buf->cursor.x_width;
    else if (buf->cursor.x_width > buf->scroll.x_width + screen_width)
        buf->scroll.x_width = buf->cursor.x_width - screen_width;
}

void truncate_cur(Buffer *buf) {
    Line *ln = &buf->lines[buf->cursor.y];
    char *s = ln->data;

    buf->cursor.x_width -= index_by_width(buf->cursor.x_width, &s);
    buf->cursor.x_bytes = s - ln->data;
}

void recalc_cur(Buffer *buf) {
    buf->cursor.x_width = buf->cursor.lx_width;
    truncate_cur(buf);
}
