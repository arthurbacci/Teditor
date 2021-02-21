#include "ted.h"

void show_menu() {
    setcolor(COLOR_PAIR(2));
    move(config.LINES, 0);
    for (unsigned int i = 0; i < (unsigned int)COLS; i++)
        addch(' ');

    move(config.LINES, 0);
    printw("I:%u", lines[cy].ident);

    char buf[100];
    switch (config.line_break_type) {
    case 0:
        snprintf(buf, 100, "LF");
        break;
    case 1:
        snprintf(buf, 100, "CRLF");
        break;
    case 2:
        snprintf(buf, 100, "CR");
        break;
    }
    move(config.LINES, COLS - strlen(buf));
    printw("%s", buf);
}

void show_lines() {
    
    for (unsigned int i = text_scroll.y; i < text_scroll.y + config.LINES; i++) {
        move(i - text_scroll.y, 0);
        if (i >= num_lines) {
            setcolor(COLOR_PAIR(1));
            for (unsigned int j = 0; j < len_line_number - 1; j++)
                addch(' ');
            addch('~');
            for (unsigned int j = 0; j < (unsigned int)COLS - len_line_number; j++)
                addch(' ');
            setcolor(COLOR_PAIR(2));
            continue;
        }
        
                
        setcolor(COLOR_PAIR(1));

        printw("%*d ", len_line_number, i + 1);

        setcolor(COLOR_PAIR(2));

        unsigned int size = 0;
        char passed_limit = 0;
        for (unsigned int j = 0; size < (unsigned int)COLS - len_line_number - 1 + text_scroll.x; j++) {

            if (passed_limit) {
                addch(' ');
                size++;
                continue;
            }
            if (lines[i].data[j] == '\0') {
                addch(' ');
                passed_limit = 1;
                size++;
                continue;
            }
            unsigned char fg, bg;
            readColor(i, j, &fg, &bg);
            int palette[] = {-1, COLOR_RED, COLOR_BLUE, COLOR_GREEN, COLOR_MAGENTA, COLOR_CYAN};
            if (colors_on) {
                init_pair(3 + lines[i].color[j], palette[fg], palette[bg]);
            }
            if (fg || bg)
                setcolor(COLOR_PAIR(3 + lines[i].color[j]));
            else
                setcolor(COLOR_PAIR(2));
            if (isprint(lines[i].data[j]) || lines[i].data[j] == '\t') {
                if (size >= text_scroll.x) {
                    if (lines[i].data[j] == '\t') {
                        for (unsigned int k = 0; k < config.tablen; k++)
                            addch(' ');
                        size += config.tablen - 1;
                    } else
                        addch(lines[i].data[j]);                    
                }
            } else {
                unsigned char b[4];
                printw("%.*s", utf8ToMultibyte(lines[i].data[j], b), b);
            }
            
            if (lines[i].data[j] == '\0') {
                addch(' ');
                passed_limit = 1;
                size++;
                continue;
            }
            size++;
        }

    }
}
