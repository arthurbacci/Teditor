#ifndef TED_SCREEN_H
#define TED_SCREEN_H

#include <stddef.h>

typedef struct {
    // The width and height of the terminal
    size_t width;
    size_t height;
    
    // The width and height of the screen minus the menu and the line numbers
    size_t text_width;
    size_t text_height;
} ScreenInfo;

ScreenInfo get_screen_info(int len_line_number);

#endif
