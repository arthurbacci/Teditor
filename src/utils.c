#include "ted.h"

void die(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
    if (is_jmp_set)
        longjmp(end, TED_LONGJMP_DIE);
    else
        exit(2);
}

char *home_path(const char *path) {
    char *ret = malloc(1000 * sizeof *ret);
    snprintf(ret, 1000, "%s/%s", getenv("HOME"), path);
    return ret;
}


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


int calculate_len_line_number(Buffer buf) {
    // + 1 because lines numbers start with 1, not 0
    int num = buf.num_lines + 1;
    int i = 1;

    do {
        num /= 10;// base 10
        ++i;
    } while (num);
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

bool is_whitespace(char c) {
    return strchr(WHITESPACE_CHARS, c) != NULL;
}

void ensure_ted_dirs(void) {
    char *ted_dirs[] = {get_ted_data_home(), get_ted_config_home(), get_ted_state_home(),
                        get_ted_cache_home()};
    
    for (int i = 0; i < (sizeof(ted_dirs) / sizeof(char *)); i++) {
        struct stat st = {0};
        
        if (-1 == stat(ted_dirs[i], &st))
            if (-1 == mkdir(ted_dirs[i], 0770))
                die("Couldn't create ted directories");
        
        free(ted_dirs[i]);
    }
}

int process_as_bool(const char *s) {
    size_t len = strlen(s);
    
    if (len == 1) {
        if (*s == 't' || *s == 'T' || *s == '1')
            return 1;
        if (*s == 'f' || *s == 'F' || *s == '0')
            return 0;
        return -1;
    }

    // NOTE: this function only works if the first letter of t is different
    // than the first letter of f
    const char *t = "true";
    const char *f = "false";

    for (size_t c = 0; c < len; c++)
        if (tolower(s[c]) != t[c] && tolower(s[c]) != f[c])
            return -1;

    return tolower(*s) == 't';
}

int invoke_editorconfig(const char *prop, const char *filename) {
    char *program_path = home_path(".local/share/ted/ted_editorconfig");

    int status;
    pid_t child;

    switch (child = fork()) {
        case -1:
            return 127;
        case 0:
            execl(program_path, "ted_editorconfig", prop, filename, (char *) NULL);
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

char *printdup(const char *format, ...) {
    va_list va1, va2;
    char *ret = NULL;
    
    va_start(va1, format);
    va_copy(va2, va1);
    
    int len = vsnprintf(NULL, 0, format, va1);
    if (len <= 0) goto END;
    
    ret = malloc(len + 1);
    if (0 >= vsnprintf(ret, len + 1, format, va2))
        ret = NULL;
    
END:
    va_end(va1);
    va_end(va2);
    
    return ret;
}

