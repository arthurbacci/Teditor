#include "ted.h"

size_t split_cmd_string(const char *s, char ret[CMD_ARR_SZ + 1][CMD_WORD_SZ]) {
    for (; *s == ' '; s++);

    size_t i;
    for (i = 0; *s; i++) {
        const char *n = s;
        for (; *n != ' ' && *n != '\0'; n++);

        size_t cpsz = MIN(n - s, CMD_WORD_SZ - 1);
        memcpy(ret[i], s, cpsz);
        ret[i][cpsz] = '\0';

        for (; *n == ' '; n++);
        s = n;
    }

    return i;
}

Line blank_line(void) {
    Line ln;

    ln.cap = READ_BLOCKSIZE;
    ln.data = malloc(ln.cap);
    ln.length = 0;

    *ln.data = '\0';
    return ln;
}

size_t get_ident_sz(char *s) {
    size_t ident_sz;
    char c;
    for (
        ident_sz = 0;
        ' ' == (c = s[ident_sz])
        || '\t' == c;
        ident_sz++
    );

    return ident_sz;
}

int invoke_editorconfig(const char *prop, const char *filename) {
    char *program_path = printdup("%s/" TED_EDITORCONFIG_PLUGIN_NAME, ted_data_home);

    int status;
    pid_t child;

    switch (child = fork()) {
        case -1:
            return 127;
        case 0:
            execl(program_path, TED_EDITORCONFIG_PLUGIN_NAME, prop, filename, (char *) NULL);
            _exit(127);
        default:
            if (waitpid(child, &status, 0) == -1)
                return 127;
    }

    free(program_path);

    return WEXITSTATUS(status);
}

void configure_editorconfig(Buffer *b) {
    bool use_tabs = 1 == invoke_editorconfig("indent_style", b->filename);
    int indent_size = invoke_editorconfig("indent_size", b->filename);
    int tab_width = invoke_editorconfig("tab_width", b->filename);

    if (indent_size < 100)
        b->indent_size = indent_size;
    if (tab_width < 100)
        b->tab_width = tab_width;
    if (use_tabs)
        b->indent_size = 0;
}

void replace_fd(int fd, const char *filename, int flags) {
    close(fd);
    if (fd != open(filename, flags))
        die("couldn't get the same file descriptor as the original");
}
