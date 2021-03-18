#include "ted.h"

void show_menu(char *message) {
    setcolor(COLOR_PAIR(2));
    move(config.lines, 0);
    for (unsigned int i = 0; i < (unsigned int)COLS; i++)
        addch(' ');

    move(config.lines, 0);
    if (!*message) {
        printw("I:%u %s", lines[cy].ident, filename);
        move(config.lines, COLS - (config.line_break_type == 1 ? 4 : 2));
        printw("%s", config.line_break_type == 0 ? "LF" : (config.line_break_type == 1 ? "CRLF" : "CR"));
    } else
        printw("%s", message);
}

void show_lines(void) {
    for (unsigned int i = text_scroll.y; i < text_scroll.y + config.lines; i++) {
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

        printw("%*d ", len_line_number + 1, i + 1);

        setcolor(COLOR_PAIR(2));
        unsigned int size = 0;
        for (unsigned int j = 0; size < (unsigned int)COLS - len_line_number - 1; j++) {
            if (text_scroll.x + j >= lines[i].length) {
                if (j + text_scroll.x == cursor.x && i == cursor.y)
                    attron(A_REVERSE);
                    
                addch(' ');
                attroff(A_REVERSE);
                size++;
                continue;
            }
            uchar32_t el = lines[i].data[text_scroll.x + j];
            
            unsigned char fg, bg;
            readColor(i, text_scroll.x + j, &fg, &bg);
            int palette[] = {-1, COLOR_RED, COLOR_BLUE, COLOR_GREEN, COLOR_MAGENTA, COLOR_CYAN};
            if (colors_on)
                init_pair(3 + lines[i].color[text_scroll.x + j], palette[fg], palette[bg]);
            
            if (fg || bg)
                setcolor(COLOR_PAIR(3 + lines[i].color[text_scroll.x + j]));
            else
                setcolor(COLOR_PAIR(2));
            
            if (j + text_scroll.x == cursor.x && i == cursor.y)
                attron(A_REVERSE);
            
            if (el == '\t') {
                for (unsigned int k = 0; k < config.tablen; k++)
                    addch(' ');
                size += config.tablen - 1;
            } else {
                unsigned char b[4];
                printw("%.*s", utf8ToMultibyte(el, b), b);
            }
            
            if (j == cursor.x + text_scroll.x && i == cursor.y)
                attroff(A_REVERSE);
            if (bg)
                setcolor(COLOR_PAIR(2));
            
            
            size++;
        }
    }
}
