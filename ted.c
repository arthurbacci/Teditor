#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define READ_BLOCKSIZE 10

struct line
{
    unsigned int len;
    unsigned char *data;
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
        lines[i].data = malloc(lines[i].len * sizeof(wchar_t));

        char c;
        unsigned int j;
        for (j = 0; (c = fgetc(fp)) != '\n' && c != EOF; j++)
        {
            if (j >= READ_BLOCKSIZE)
            {
                lines[i].len += READ_BLOCKSIZE;
                lines[i].data = realloc(lines[i].data, lines[i].len * sizeof(wchar_t));
            }
            lines[i].data[j] = c;
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


    fp = fopen("test.txt", "r");

    read_lines();


    {
        char tmp[50];
        len_line_number = snprintf(tmp, 50, "%u", num_lines + 1);
    }


    show_lines();


    free_lines();
    fclose(fp);

    getch();

    endwin();
    return 0;
}
