#include "ted.h"

// Should be called after changing cursor position
void cursor_in_valid_position(void) {
    /* Check if cursor is inside the borders */
    // y
    if (cursor.y >= num_lines)
        cursor.y = num_lines - 1;
    // x
    if (cursor.x > lines[cursor.y].length)
        cursor.x = lines[cursor.y].length;

    /* Scroll related */
    // y
    if (cursor.y < text_scroll.y)
        text_scroll.y = cursor.y;
    if (cursor.y > text_scroll.y + config.lines - 1)
        text_scroll.y = cursor.y + 1 - config.lines;
    // x
    if (cursor.x < text_scroll.x)
        text_scroll.x = cursor.x;
    else if (cursor.x > text_scroll.x + (COLS - len_line_number - 3))
        text_scroll.x = cursor.x - (COLS - len_line_number - 3);
}

// This should be an inline function
void change_position(unsigned int x, unsigned int y) {
    cursor.y = y;
    cursor.x = x;
    cursor_in_valid_position();
}

