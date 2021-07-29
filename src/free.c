#include "ted.h"

void free_lines(void) {
    for (unsigned int i = 0; i < num_lines; i++) {
        free(lines[i].data);
        lines[i].len = 0;
    }
    free(lines);
}

