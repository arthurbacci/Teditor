/*#include <ted_buffer.h>
#include <ted_grapheme.h>
#include <ted_xdg.h>
#include <ted_string_utils.h>*/
#include <stdio.h>
#include <string.h>

// FIXME: temporary
#include "ted.h"

BufferList buffer_list = {0};

void calculate_scroll(Buffer *buf, size_t screen_width) {
    // y
    if (buf->cursor.y < buf->scroll.y)
        buf->scroll.y = buf->cursor.y;
    if (buf->cursor.y >= buf->scroll.y + SROW)
        buf->scroll.y = buf->cursor.y + 1 - SROW;
    
    // x
    if (buf->cursor.x_width < buf->scroll.x_width)
        buf->scroll.x_width = buf->cursor.x_width;
    if (buf->cursor.x_width > buf->scroll.x_width + screen_width)
        buf->scroll.x_width = buf->cursor.x_width - screen_width;
}

// TODO: check if there's any usage of truncate_cur except for recalc_cur
// and if not, join the two functions into one
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

Buffer default_buffer() {
    char *filename = printdup("%s/buffer", ted_data_home);

    FILE *fp = fopen(filename, "r");
    return read_lines(fp, filename, can_write(filename));
}

void open_buffer(Buffer b) {
    if (buffer_list.len + 1 < NUM_BUFFERS) {
        buffer_list.selected = buffer_list.len;
        buffer_list.bufs[buffer_list.len++] = b;
    }
}

void buffer_close(void) {
    if (buffer_list.len == 1)
        TED_CALL_LONGJMP(TED_LONGJMP_USER_EXIT);

    buffer_list.len--;

    memmove(
        &buffer_list.bufs[buffer_list.selected],
        &buffer_list.bufs[buffer_list.selected + 1],
        buffer_list.len - buffer_list.selected
    );

    if (buffer_list.selected > 0)
        buffer_list.selected--;
}

void next_buffer(void) {
    buffer_list.selected++;
    if (buffer_list.selected >= buffer_list.len)
        buffer_list.selected = 0;
}

void previous_buffer(void) {
    if (buffer_list.selected == 0)
        buffer_list.selected = buffer_list.len;
    buffer_list.selected--;
}


