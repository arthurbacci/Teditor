#include "ted.h"

BufferList buffer_list = {0};

Buffer default_buffer() {
    ensure_data_dir();

    char *filename = home_path(".local/state/ted/buffer");

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
    if (buffer_list.len == 1) {
        if (is_jmp_set)
            longjmp(end, TED_LONGJMP_USER_EXIT);
        else
            exit(EXIT_SUCCESS);
    }

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


