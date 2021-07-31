#include "ted.h"

bool modify(Buffer *buf) {
    if (buf->read_only)
        message("Can't modify a read-only file.");
    else
        buf->modified = 1;

    return !buf->read_only;
}

bool add_char(int x, int y, uchar32_t c, Buffer *buf) {
    if (modify(buf)) {
        expand_line(buf->cursor.y, 1, buf);
        memmove(
            &buf->lines[y].data[x + 1],
            &buf->lines[y].data[x],
            (buf->lines[y].length - x) * sizeof(uchar32_t)
        );
        buf->lines[y].data[x] = c;
        buf->lines[y].data[++buf->lines[y].length] = '\0';
        return 1;
    }
    return 0;
}

bool remove_char(int x, int y, Buffer *buf) {
    if (modify(buf)) {
        memmove(
            &buf->lines[y].data[x],
            &buf->lines[buf->cursor.y].data[x + 1],
            (buf->lines[buf->cursor.y].length - x + 1) * sizeof(uchar32_t));
        buf->lines[y].data[--buf->lines[y].length] = '\0';
        return 1;
    }
    return 0;
}
