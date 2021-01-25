#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define READ_BLOCKSIZE 10

struct line
{
    unsigned int len;
    char *data;
};

struct line *lines = NULL;
unsigned int num_lines;
unsigned int len_line_number; // The length of the number of the last line

FILE *fp = NULL;

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
    for (unsigned int i = 0; i < num_lines; i++)
    {
        printw("%*d %s\n", len_line_number, i + 1, lines[i].data);
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

    move(0, len_line_number + 1);

    free_lines();
    fclose(fp);

    getch();

    endwin();
    return 0;
}
