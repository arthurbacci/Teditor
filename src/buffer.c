#include "ted.h"

char **buffers = NULL;
unsigned int num_buffers = 0;
unsigned int in_buffer = 0;

void appendBuffer(char *bufname) {
    unsigned int bufname_len = strlen(bufname);
    char *buf = malloc(bufname_len + 1);
    strcpy(buf, bufname);
    buffers = realloc(buffers, ++num_buffers * sizeof *buffers);
    buffers[num_buffers - 1] = buf;
    in_buffer = num_buffers - 1;
    openFile(buffers[in_buffer], 0);
}

void nextBuffer() {
    in_buffer++;
    if (in_buffer >= num_buffers) in_buffer = 0;
    openFile(buffers[in_buffer], 0);
}

void prevBuffer() {
    if (in_buffer == 0) in_buffer = num_buffers - 1;
    else                in_buffer--;
    openFile(buffers[in_buffer], 0);
}

void freeBuffers() {
    for (unsigned int i = 0; i < num_buffers; i++)
        free(buffers[i]);
    free(buffers);
}

