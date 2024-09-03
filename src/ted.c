/*
TODO: remove the Grapheme type and do it without overhead
*/

#include "ted.h"
#include <curses.h>
#include <unistd.h>

char *menu_message = "";

GlobalCfg config = {4, 0, 1, " \t~!@#$%^&*()-=+[{]}\\|;:'\",.<>/?"};

bool is_jmp_set = false;
jmp_buf end;


int main(int argc, char **argv) {
    if (!isatty(STDIN_FILENO)) {
        // TODO: read pipe contents to buffer before closing it
        close(STDIN_FILENO);
        if (STDIN_FILENO != open("/dev/tty", O_RDONLY))
            die("didn't open as stdin");
    }

    /*
    if (isatty(STDERR_FILENO)) {
        close(STDERR_FILENO);
        if (STDERR_FILENO != creat("log.txt", O_WRONLY))
            die("didn't open as stderr");
    }
    */



    Node *buf = NULL;
    if (argc < 2) {
        // FIXME: Lots of calls to home_path

        struct stat st = {0};

        char *config = home_path(".config/");
        if (stat(config, &st) == -1)
            mkdir(config, 0777);

        char *config_ted = home_path(".config/ted/");
        if (stat(config_ted, &st) == -1)
            mkdir(config_ted, 0777);
            
        char *filename = home_path(".config/ted/buffer");
        free(config);
        free(config_ted);

        FILE *fp = fopen(filename, "r");
        buf = single_buffer(read_lines(fp, filename, can_write(filename)));
        if (fp)
            fclose(fp);
    } else {
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
            if (i == 1)
                buf = single_buffer(b);
            else
                buffer_add_prev(buf, b);

            if (fp)
                fclose(fp);
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


    int val = setjmp(end);

    if (!val) {
        is_jmp_set = true;

        while (1) {
            int len_line_number = calculate_len_line_number(buf->data);

            calculate_scroll(&buf->data, COLS - len_line_number - 2);

            display_buffer(buf->data, len_line_number);
            display_menu(menu_message, NULL, buf);
            refresh();

            int c = getch();
            process_keypress(c, &buf);
        }
    }

    free_buffer_list(buf);

    endwin();
    return val - 1;
}

