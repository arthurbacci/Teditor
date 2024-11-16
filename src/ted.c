/*
TODO: remove the Grapheme type and do it without overhead
*/

#include "ted.h"
#include <curses.h>
#include <unistd.h>

char *menu_message = "";

bool is_ted_longjmp_set = false;
jmp_buf ted_longjmp_end;

int main(int argc, char **argv) {
    ensure_ted_dirs();

    // It may be interesting to support pipes some point in the future
    if (!isatty(STDOUT_FILENO) || !isatty(STDIN_FILENO))
        die("This editor doesn't support pipes");

    if (isatty(STDERR_FILENO)) {
        /*char *filename = log_file_path();

        // Create file if it doesn't exist (creat() gives it some strange
        // permissions)
        FILE *fp = fopen(filename, "w");
        if (fp) fclose(fp);

        replace_fd(STDERR_FILENO, filename, O_WRONLY);

        free(filename);*/
    }

    open_buffer(default_buffer());
    
    for (int i = 1; i < argc; i++) {
        char *filename = malloc(PATH_MAX + 1);
        size_t len = 0;

        if (*argv[i] == '/') {
            /* Absolute file path */
            len = strlen(argv[i]);
            memcpy(filename, argv[i], len + 1);
        } else {
            /* Relative file path */

            // Write the directory path into filename
            if (getcwd(filename, PATH_MAX) != NULL) {
                len = strlen(filename);
                len += snprintf(
                    filename + len,
                    PATH_MAX - len,
                    "/%s",
                    argv[i]
                );
            } else
                die("Could not get cwd, try an absolute file path");

            // Now we have an absolute filename
        }

        char *smaller_filename = malloc(len + 1);
        memcpy(smaller_filename, filename, len + 1);
        free(filename);
        filename = smaller_filename;

        FILE *fp = fopen(filename, "r");
        Buffer b = read_lines(fp, filename, can_write(filename));
        open_buffer(b);
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


    int val = setjmp(ted_longjmp_end);

    if (!val) {
        is_ted_longjmp_set = true;

        while (1) {
            int len_line_number = snprintf(NULL, 0, "%lu", SEL_BUF.num_lines);
        
            calculate_scroll(&SEL_BUF, COLS - len_line_number - 2);

            display_buffer(SEL_BUF, len_line_number);
            display_menu(menu_message, NULL);
            refresh();

            int c = getch();
            process_keypress(c);
        }
    }

    endwin();
    return val - 1;
}

