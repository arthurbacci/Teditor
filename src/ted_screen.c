#include <ted_screen.h>
#include <ncurses.h>

ScreenInfo get_screen_info(int len_line_number) {
    ScreenInfo r = {COLS - 1, LINES - 1, COLS - len_line_number - 2, LINES - 2};
    return r;
}

