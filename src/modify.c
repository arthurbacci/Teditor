#include "ted.h"

bool modify(Buffer *buf) {
    if (buf->read_only)
        message("Can't modify a read-only file.");
    else
        buf->modified = 1;

    return !buf->read_only;
}

bool add_char(size_t x, size_t y, const Grapheme c, Buffer *buf) {
    if (modify(buf)) {
        expand_line(buf->cursor.y, c.sz, buf);
        memmove(
            &buf->lines[y].data[x + c.sz],
            &buf->lines[y].data[x],
            buf->lines[y].length - x
        );
        memcpy(&buf->lines[y].data[x], c.dt, c.sz);
        buf->lines[y].data[buf->lines[y].length += c.sz] = '\0';
        return 1;
    }
    return 0;
}

// Asserts that `x` points to a char boundary
bool remove_char(size_t x, size_t y, Buffer *buf) {
    if (modify(buf)) {
        char *afterpos = &buf->lines[y].data[x];

        size_t grapheme_sz = get_next_grapheme(
            &afterpos,
            SIZE_MAX
        ).sz;

        memmove(
            &buf->lines[y].data[x],
            afterpos,
            buf->lines[y].length - x + grapheme_sz
        );
        buf->lines[y].data[buf->lines[y].length -= grapheme_sz] = '\0';

        return 1;
    }
    return 0;
}
