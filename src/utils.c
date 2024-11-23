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


void replace_fd(int fd, const char *filename, int flags) {
    close(fd);
    if (fd != open(filename, flags))
        die("couldn't get the same file descriptor as the original");
}
