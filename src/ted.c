#include <ncurses.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <ted_xdg.h>
#include <ted_input.h>
#include <ted_output.h>
#include <ted_utils.h>
#include <ted_buffer.h>
#include <ted_prompt.h>
#include <ted_config.h>


bool is_ted_longjmp_set = false;
jmp_buf ted_longjmp_end;

void replace_fd(int fd, const char *filename, int flags) {
    close(fd);
    if (fd != open(filename, flags))
        die("couldn't get the same file descriptor as the original");
}

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

    open_default_buffer();
    
    for (int i = 1; i < argc; i++) {
        // TODO: refactor this ugly piece of code
        char *filename = malloc(1000);
        size_t len = 0;

        if (*argv[i] == '/') {
            /* Absolute file path */
            len = strlen(argv[i]);
            memcpy(filename, argv[i], len + 1);
        } else {
            /* Relative file path */

            // Write the directory path into filename
            if (getcwd(filename, 1000) != NULL) {
                len = strlen(filename);
                len += snprintf(
                    filename + len,
                    1000 - len,
                    "/%s",
                    argv[i]
                );
            } else
                die("Could not get cwd, try an absolute file path");

            // Now we have an absolute filename
        }

        open_file(printdup("%s", filename));
        free(filename);
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
            ScreenInfo screen_info = get_screen_info(len_line_number);
        
            calculate_scroll(&SEL_BUF, screen_info);

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

