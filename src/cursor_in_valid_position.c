#include "ted.h"

// Should be called after changing cursor position
void cursor_in_valid_position(Buffer *buf) {
    /* Check if cursor is inside the borders */
    // y
    if (buf->cursor.y >= buf->num_lines)
        buf->cursor.y = buf->num_lines - 1;
    // x
    if (buf->cursor.x > buf->lines[buf->cursor.y].length)
        buf->cursor.x = buf->lines[buf->cursor.y].length;
}

void change_position(unsigned int x, unsigned int y, Buffer *buf) {
    buf->cursor.y = y;
    buf->cursor.x = x;
    cursor_in_valid_position(buf);
}

