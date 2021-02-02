#include "ted.h"

void show_menu()
{
    move(config.LINES, 0);
    for (unsigned int i = 0; i < (unsigned int)COLS; i++)
    {
        addch(' ');
    }

    move(config.LINES, 0);
    printw("I:%u", lines[cy].ident);

    char buf[100];
    switch (config.line_break_type)
    {
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

void show_lines()
{
    
    for (unsigned int i = text_scroll.y; i < text_scroll.y + config.LINES; i++)
    {
        move(i - text_scroll.y, 0);
        if (i >= num_lines)
        {
            setcolor(COLOR_PAIR(1));
            for (unsigned int j = 0; j < len_line_number - 1; j++)
            {
                addch(' ');
            }
            addch('~');
            for (unsigned int j = 0; j < (unsigned int)COLS - len_line_number; j++)
            {
                addch(' ');
            }
            setcolor(COLOR_PAIR(2));
            continue;
        }
        
        
        
        setcolor(COLOR_PAIR(1));

        printw("%*d ", len_line_number, i + 1);

        setcolor(COLOR_PAIR(2));


        unsigned int size = 0;
        char passed_limit = 0;
        for (unsigned int j = 0; size < (unsigned int)COLS - len_line_number - 1 + text_scroll.x; j++)
        {

            if (passed_limit)
            {
                addch(' ');
                size++;
                continue;
            }

            if (lines[i].data[j] == '\0')
            {
                addch(' ');
                passed_limit = 1;
                size++;
                continue;
            }

            if (lines[i].data[j] >= 0xC0 && lines[i].data[j] <= 0xDF)
            {
                if (lines[i].data[j] == '\0')
                {
                    addch(' ');
                    passed_limit = 1;
                    size++;
                    continue;
                }
                if (size >= text_scroll.x)
                {
                    printw("%.2s", &lines[i].data[j]);
                }
                j++;
            }
            else if (lines[i].data[j] >= 0xE0 && lines[i].data[j] <= 0xEF)
            {
                if (lines[i].data[j] == '\0')
                {
                    addch(' ');
                    passed_limit = 1;
                    size++;
                    continue;
                }
                if (size >= text_scroll.x)
                {
                    printw("%.3s", &lines[i].data[j]);
                }
                j += 2;
            }
            else if (lines[i].data[j] >= 0xF0 && lines[i].data[j] <= 0xF7)
            {
                if (lines[i].data[j] == '\0')
                {
                    addch(' ');
                    passed_limit = 1;
                    size++;
                    continue;
                }
                if (size >= text_scroll.x)
                {
                    printw("%.4s", &lines[i].data[j]);
                }
                j += 3;
            }
            else
            {
                if (size >= text_scroll.x)
                {
                    printw("%c", lines[i].data[j]);
                }
            }

            if (lines[i].data[j] == '\0')
            {
                addch(' ');
                passed_limit = 1;
                size++;
                continue;
            }
            size++;
        }

    }
}
