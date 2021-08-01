#include "ted.h"

void free_buffer(Buffer *buf) {
    for (unsigned int i = 0; i < buf->num_lines; i++) {
        free(buf->lines[i].data);
        buf->lines[i].len = 0;
    }
    free(buf->lines);
    free(buf->filename);
    free(buf->name);
}

