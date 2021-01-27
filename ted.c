#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#define READ_BLOCKSIZE 10

struct line
{
    unsigned int len;
    unsigned char *data;
    unsigned int length;
};

struct line *lines = NULL;
unsigned int num_lines;
unsigned int len_line_number; // The length of the number of the last line

FILE *fp = NULL;

struct
{
    unsigned int x;
    unsigned int y;
}
cursor = {0, 0};

struct
{
    unsigned int x;
    unsigned int y;
}
text_scroll = {0, 0};

char colors_on;

void setcolor(int c)
{
    if (colors_on)
    {
        attrset(c);
    }
}

void read_lines()
{
    num_lines = 0;
    for (unsigned int i = 0; !feof(fp); i++)
    {
        lines = realloc(lines, ++num_lines * sizeof(struct line));     

        lines[i].len = READ_BLOCKSIZE;
        lines[i].data = malloc(lines[i].len);
        lines[i].length = 0;

        char c;
        unsigned int j;
        for (j = 0; (c = fgetc(fp)) != '\n' && c != EOF; j++)
        {
            if (j >= READ_BLOCKSIZE)
            {
                lines[i].len += READ_BLOCKSIZE;
                lines[i].data = realloc(lines[i].data, lines[i].len);
            }

            unsigned char uc = *(unsigned char *)&c;
            
            if (uc >= 0xC0 && uc <= 0xDF)
            {
                if (uc == '\0')
                {
                    break;
                }
                if (j + 1 >= READ_BLOCKSIZE)
                {
                    lines[i].len += READ_BLOCKSIZE;
                    lines[i].data = realloc(lines[i].data, lines[i].len);
                }
                for (unsigned int k = 0; k < 2; k++)
                {
                    lines[i].data[j + k] = c;
                    if (k < 1)
                    {
                        c = fgetc(fp);
                    }
                }
                j++;
            }
            else if (uc >= 0xE0 && uc <= 0xEF)
            {
                if (uc == '\0')
                {
                    break;
                }
                if (j + 2 >= READ_BLOCKSIZE)
                {
                    lines[i].len += READ_BLOCKSIZE;
                    lines[i].data = realloc(lines[i].data, lines[i].len);
                }
                for (unsigned int k = 0; k < 3; k++)
                {
                    lines[i].data[j + k] = c;
                    if (k < 2)
                    {
                        c = fgetc(fp);
                    }
                }
                j += 2;
            }
            else if (uc >= 0xF0 && uc <= 0xF7)
            {
                if (uc == '\0')
                {
                    break;
                }
                if (j + 3 >= READ_BLOCKSIZE)
                {
                    lines[i].len += READ_BLOCKSIZE;
                    lines[i].data = realloc(lines[i].data, lines[i].len);
                }
                for (unsigned int k = 0; k < 4; k++)
                {
                    lines[i].data[j + k] = c;
                    if (k < 3)
                    {
                        c = fgetc(fp);
                    }
                }
                j += 3;
            }
            else
            {
                lines[i].data[j] = uc;
            }

            lines[i].length++;
        }
        lines[i].data[j] = '\0';
    }
}

void show_lines()
{
    for (unsigned int i = text_scroll.y; i < text_scroll.y + LINES; i++)
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
            setcolor(COLOR_PAIR(2));
            continue;
        }
        
        setcolor(COLOR_PAIR(1));

        printw("%*d ", len_line_number, i + 1);

        setcolor(COLOR_PAIR(2));


        unsigned int size = 0;
        for (unsigned int j = 0; size < (unsigned int)COLS - len_line_number - 1 + text_scroll.x; j++)
        {
            if (lines[i].data[j] == '\0')
            {
                break;
            }

            if (lines[i].data[j] >= 0xC0 && lines[i].data[j] <= 0xDF)
            {
                if (lines[i].data[j] == '\0')
                {
                    break;
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
                    break;
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
                    break;
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
                break;
            }
            size++;
        }

    }
}

void free_lines()
{
    for (unsigned int i = 0; i < num_lines; i++)
    {
        free(lines[i].data);
        lines[i].len = 0;
    }
}



int main()
{
    setlocale(LC_ALL, "");

    initscr();
    noecho();
    keypad(stdscr, TRUE);


    fp = fopen("test.txt", "r");

    read_lines();

    {
        char tmp[50];
        len_line_number = snprintf(tmp, 50, "%u", num_lines + 1);
    }


    if (!has_colors())
    {
        colors_on = 0;
    }
    else if (start_color() != OK)
    {
        colors_on = 0;
    }
    else {
        colors_on = 1;
    }


    if (colors_on)
    {
        use_default_colors();
        init_pair(1, COLOR_RED, -1);
        init_pair(2, -1, -1);
    }


    int c;
    while (1)
    {
        clear();
        show_lines();
        move(cursor.y - text_scroll.y, cursor.x - text_scroll.x + len_line_number + 1);
        refresh();

        c = getch();

        switch (c)
        {
            case KEY_UP:
                cursor.y -= (cursor.y > 0);
                if (cursor.x > lines[cursor.y].length)
                {
                    cursor.x = lines[cursor.y].length;
                }
                if (cursor.y < text_scroll.y)
                {
                    text_scroll.y = cursor.y;
                }

                if (cursor.x < text_scroll.x)
                {
                    text_scroll.x = cursor.x;
                }
                else if (cursor.x - text_scroll.x >= (unsigned int)COLS - len_line_number - 1)
                {
                    text_scroll.x += (cursor.x - text_scroll.x) - ((unsigned int)COLS - len_line_number - 2);
                }
                break;
            case KEY_DOWN:
                cursor.y += (cursor.y < num_lines - 1);
                if (cursor.x > lines[cursor.y].length)
                {
                    cursor.x = lines[cursor.y].length;
                }
                if (cursor.y > text_scroll.y + LINES - 1)
                {
                    text_scroll.y = cursor.y + 1 - LINES;
                }

                if (cursor.x < text_scroll.x)
                {
                    text_scroll.x = cursor.x;
                }
                else if (cursor.x - text_scroll.x >= (unsigned int)COLS - len_line_number - 1)
                {
                    text_scroll.x += (cursor.x - text_scroll.x) - ((unsigned int)COLS - len_line_number - 2);
                }
                break;
            case KEY_LEFT:
                cursor.x -= (cursor.x > 0);
                if (cursor.x < text_scroll.x)
                {
                    text_scroll.x = cursor.x;
                }
                break;
            case KEY_RIGHT:
                cursor.x += (cursor.x < lines[cursor.y].length);
                if (cursor.x - text_scroll.x >= (unsigned int)COLS - len_line_number - 1)
                {
                    text_scroll.x += (cursor.x - text_scroll.x) - ((unsigned int)COLS - len_line_number - 2);
                }
                break;
        }

        if (c == 'q')
        {
            break;
        }
    }


    free_lines();
    fclose(fp);


    endwin();
    return 0;
}
