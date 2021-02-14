#include "ted.h"

void free_lines() {
    for (unsigned int i = 0; i < num_lines; i++) {
        free(lines[i].data);
        free(lines[i].color);
        lines[i].len = 0;
    }
}

