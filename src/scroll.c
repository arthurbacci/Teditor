#include "ted.h"

void calculate_scroll(Buffer buf, int len_line_number) {
    // y
    if (buf.cursor.y < buf.scroll.y)
        buf.scroll.y = buf.cursor.y;
    if (buf.cursor.y > buf.scroll.y + config.lines - 1)
        buf.scroll.y = buf.cursor.y + 1 - config.lines;
    // x
    if (buf.cursor.x < buf.scroll.x)
        buf.scroll.x = buf.cursor.x;
    else if (buf.cursor.x > buf.scroll.x + (COLS - len_line_number - 3))
        buf.scroll.x = buf.cursor.x - (COLS - len_line_number - 3);
}