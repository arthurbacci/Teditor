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
    for (unsigned int i = 0; i < (unsigned int)LINES; i++)
    {
        if (i >= num_lines)
        {
            printw("~\n");
            continue;
        }
        
        printw("%*d ", len_line_number, i + 1);

        unsigned int size = 0;
        for (unsigned int j = 0; j < (unsigned int)COLS - len_line_number - 2; j++)
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
                printw("%.2s", &lines[i].data[j]);
                j++;
            }
            else if (lines[i].data[j] >= 0xE0 && lines[i].data[j] <= 0xEF)
            {
                if (lines[i].data[j] == '\0')
                {
                    break;
                }
                printw("%.3s", &lines[i].data[j]);
                j += 2;
            }
            else if (lines[i].data[j] >= 0xF0 && lines[i].data[j] <= 0xF7)
            {
                if (lines[i].data[j] == '\0')
                {
                    break;
                }
                printw("%.4s", &lines[i].data[j]);
                j += 3;
            }
            else
            {
                printw("%c", lines[i].data[j]);
            }

            if (lines[i].data[j] == '\0')
            {
                break;
            }
            size++;
        }

        printw("\n");
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


    int c;
    while (1)
    {
        clear();
        move(0, 0);
        show_lines();
        move(cursor.y, cursor.x + len_line_number + 1);
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
                break;
            case KEY_DOWN:
                cursor.y += (cursor.y < num_lines - 1);
                if (cursor.x > lines[cursor.y].length)
                {
                    cursor.x = lines[cursor.y].length;
                }
                break;
            case KEY_LEFT:
                cursor.x -= (cursor.x > 0);
                break;
            case KEY_RIGHT:
                cursor.x += (cursor.x < lines[cursor.y].length);
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
