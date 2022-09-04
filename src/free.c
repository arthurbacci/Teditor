#include "ted.h"

void free_buffer(Buffer *buf) {
    for (size_t i = 0; i < buf->num_lines; i++) {
        free(buf->lines[i].data);
    }
    free(buf->lines);
    free(buf->filename);
    free(buf->name);
}

