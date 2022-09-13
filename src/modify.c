#include "ted.h"

bool modify(Buffer *buf) {
    if (buf->read_only)
        message("Can't modify a read-only file.");
    else
        buf->modified = 1;

    return !buf->read_only;
}

void add_char(Grapheme c, size_t x, Line *ln) {
    expand_line(ln, c.sz);
    memmove(
        &ln->data[x + c.sz],
        &ln->data[x],
        ln->length - x
    );
    memcpy(&ln->data[x], c.dt, c.sz);
    ln->data[ln->length += c.sz] = '\0';
}

// Note that `x` must point to a char boundary
void remove_char(size_t x, Line *ln) {
    char *afterpos = &ln->data[x];

    size_t grapheme_sz = get_next_grapheme(
        &afterpos,
        SIZE_MAX
    ).sz;

    memmove(
        &ln->data[x],
        afterpos,
        ln->length - x + grapheme_sz
    );
    ln->data[ln->length -= grapheme_sz] = '\0';
}
