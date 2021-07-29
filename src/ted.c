#include "ted.h"

struct LINE *lines = NULL;
unsigned int num_lines;
unsigned int len_line_number; // The length of the number of the last line

FILE *fp = NULL;
struct CURSOR cursor = {0, 0};
struct TEXT_SCROLL text_scroll = {0, 0};

char *filename = NULL;
char *menu_message = "";

bool colors_on = 0;
bool needs_to_free_filename = 0;

void setcolor(int c) {
    if (colors_on)
        attrset(c);
}

unsigned int last_cursor_x = 0;

struct CFG config = {
    1, 4, 0, 0, 1, 1, 1, 1,
    {0, 0, 1}, " \t~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?",
};

jmp_buf end;


int main(int argc, char **argv) {
    if (argc < 2) {
        // FIXME: Lots of calls to home_path

        struct stat st = {0};

        char *config = home_path(".config/");
        if (stat(config, &st) == -1)
            mkdir(config, 0777);

        char *config_ted = home_path(".config/ted/");
        if (stat(config_ted, &st) == -1)
            mkdir(config_ted, 0777);
            
        filename = home_path(".config/ted/buffer");
        needs_to_free_filename = 1;
        free(config);
        free(config_ted);
    } else {
        if (*argv[1] == '/')
            // Absolute file path
            filename = argv[1];
        else {
            // Relative file path

            // First create or buffer
            filename = malloc(1000 * sizeof *filename);
            *filename = '\0';

            // Write the directory path into filename
            if (getcwd(filename, 1000) != NULL)
                // Add / to the end of the path
                strncat(filename, "/", 1000);
            else
                die("Could not get cwd, try an absolute file path");

            // Add the relative file path to the cwd
            strncat(filename, argv[1], 1000);
            needs_to_free_filename = 1;

            // Now we have a absolute filename
        }
    }
    
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
    mouseinterval(1);
    curs_set(0);
    timeout(INPUT_TIMEOUT);

    calculate_len_line_number();


    config.lines = LINES - 1;
    int last_LINES = LINES;
    int last_COLS = COLS;

    fp = fopen(filename, "r");
    read_lines();
    if (fp) fclose(fp);
    detect_read_only(filename);

    int val = setjmp(end);

    if (!val) {
        while (1) {
            if (last_LINES != LINES || last_COLS != COLS) {
                last_LINES = LINES;
                last_COLS = COLS;
                config.lines = LINES - 1;
                cursor_in_valid_position();
            }
            
            show_lines();
            show_menu(menu_message, NULL);
            refresh();

            int c = getch();
            if (c == ctrl('c')) {
                if (config.selected_buf.modified) {
                    char *prt = prompt_hints("Unsaved changes: ", "", "'exit' to exit", NULL);
                    if (prt && !strcmp("exit", prt)) {
                        free(prt);
                        break;
                    }
                    free(prt);
                } else
                    break;
            }
            process_keypress(c);
        }
    }

    free_lines();
    if (needs_to_free_filename == 1)
        free(filename);

    endwin();
    return 0;
}

