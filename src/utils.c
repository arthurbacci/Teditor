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

char *bufn(int a) {
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *s = malloc(100);
    char *p = s;

    do {
        *(p++) = letters[a % sizeof(letters)];
        a = a / sizeof(letters);
    } while (a >= sizeof(letters));
    *p = '\0';

    return s;
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
    return strchr(config.whitespace_chars, c) != NULL;
}


void ensure_data_dir() {
    struct stat st = {0};

    // TODO: support $XDG_DATA_DIR
    char *data_dir = home_path(".local/state/");
    if (-1 == stat(data_dir, &st))
        mkdir(data_dir, 0770);

    free(data_dir);

    char *data_ted_dir = home_path(".local/state/ted/");
    if (-1 == stat(data_ted_dir, &st))
        mkdir(data_ted_dir, 0770);

    free(data_ted_dir);
}

Node *default_buffer() {
    ensure_data_dir();

    char *filename = home_path(".local/state/ted/buffer");

    FILE *fp = fopen(filename, "r");
    return single_buffer(read_lines(fp, filename, can_write(filename)));
}

char *log_file_path() {
    ensure_data_dir();
    return home_path(".local/state/ted/log");
}

char *strdup(const char *s) {
    size_t len = strlen(s);
    char *r = malloc(len + 1);
    memcpy(r, s, len + 1);
    return r;
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


