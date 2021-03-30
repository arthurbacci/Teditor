#include "ted.h"

bool modify(void) {
    return !read_only;
}

bool add_char(int x, int y, uchar32_t c) {
    if (modify()) {
        expandLine(cy, 1);
        memmove(&lines[y].data[x + 1], &lines[y].data[x], (lines[y].length - x) * sizeof(uchar32_t));
        lines[y].data[x] = c;
        lines[y].data[++lines[y].length] = '\0';
        syntaxHighlight();
        return 1;
    }
    return 0;
}
