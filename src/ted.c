#include "ted.h"

struct LINE *lines = NULL;
unsigned int num_lines;
unsigned int len_line_number; // The length of the number of the last line

FILE *fp = NULL;
struct CURSOR cursor = {0, 0};
struct TEXT_SCROLL text_scroll = {0, 0};

char *filename;

char colors_on;
char needs_to_free_filename;

void setcolor(int c)
{
    if (colors_on)
    {
        attrset(c);
    }
}

unsigned int last_cursor_x = 0;

struct CFG config = {4, 0, 0};


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        struct stat st = {0};

        char *home = getenv("HOME");

        char config[strlen(home) + strlen("/.config/") + 1];

        strcpy(config, home);
        strcat(config, "/.config/");


        if (stat(config, &st) == -1)
        {
            mkdir(config, 0777);
        }

        char config_ted[strlen(config) + strlen("ted/") + 1];

        strcpy(config_ted, config);
        strcat(config_ted, "ted/");

        if (stat(config_ted, &st) == -1)
        {
            mkdir(config_ted, 0777);
        }

        filename = malloc(strlen(config_ted) + strlen("buffer") + 1);
        
        strcpy(filename, config_ted);
        strcat(filename, "buffer");

        needs_to_free_filename = 1;
    }
    else {
        filename = argv[1];
        needs_to_free_filename = 0;
    }

    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    
    fp = fopen(filename, "r");

    read_lines();

    if (fp != NULL)
    {
        fclose(fp);
    }

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
        config.LINES = LINES - 1;

        show_lines();
        show_menu();
        move(cursor.y - text_scroll.y, cursor.x - text_scroll.x + len_line_number + 1);
        refresh();

        c = getch();

        if (c == ctrl('c'))
        {
            break;
        }

        process_keypress(c);
    }


    free_lines();

    if (needs_to_free_filename == 1)
    {
        free(filename);
    }

    endwin();
    return 0;
}
