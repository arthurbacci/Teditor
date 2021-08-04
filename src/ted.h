#ifndef TED_HEADER
#define TED_HEADER

#define _POSIX_C_SOURCE 1

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <setjmp.h>
#include <limits.h>

#define READ_BLOCKSIZE 100
#define ctrl(x) ((x) & 0x1f)

#define CTRL_KEY_RIGHT 0x232
#define CTRL_KEY_LEFT  0x223

#define NUM_PAIRS 6

#define IN_RANGE(x, min, max)   ((x) >= (min)) && ((x) <= (max))
#define OUT_RANGE(x, min, max)  ((x) < (min)) || ((x) > (max))

#define INPUT_TIMEOUT  5     //timeout for input in ncurses (in milliseconds)

#define USE(x) (void)(x)

// Types

typedef uint32_t uchar32_t;

typedef struct {
    char *start;
    size_t len;
    size_t cap;
} String;

typedef struct {
    unsigned int len;
    uchar32_t *data;
    unsigned int length;
    unsigned int ident;
} Line;

typedef struct {
    size_t x;
    size_t last_x;
    size_t y;
} Cursor;

typedef struct {
    unsigned int x;
    unsigned int y;
} TextScroll;

typedef struct {
    bool modified;
    bool read_only;
    bool can_write;
    unsigned char line_break_type; // 0: LF  1: CRLF  2: CR
    Line *lines;
    size_t num_lines;
    Cursor cursor;
    TextScroll scroll;
    char *name;
    char *filename;
} Buffer;

typedef struct {
    bool strict_utf8; // high/low surrogates will be replaced (for now leave it always set)
    unsigned int tablen;
    int lines;
    bool use_spaces;
    bool autotab;
    bool automatch;
    bool insert_newline;
    char *word_separators;
} GlobalCfg;

typedef struct {
    const char *command;
    const char *hint;
} Hints;

typedef struct Node {
    Buffer data;
    struct Node *next;
    struct Node *prev;
} Node;


// message_and_prompt.c
char *prompt(const char *msgtmp, char *def);
char *prompt_hints(const char *msg, char *def, char *base, Hints *hints);
void message(char *msg);

// ted.c
void setcolor(int c);

// config_dialog.c
bool config_dialog(Node **n);
int run_command(char **words, int words_len, Node **n);
bool parse_command(char *command, Node **n);

// open_and_save.c
void savefile(Buffer *buf);
Buffer read_lines(FILE *fp, char *filename, bool read_only);
unsigned char detect_linebreak(FILE *fp);
void open_file(char *fname, Node **n);
bool can_write(char *fname);

// display.c
void display_menu(const char *message, char *shadow, Node *n);
void display_buffer(Buffer buf, int len_line_number);

// free.c
void free_buffer(Buffer *buf);

// keypress.c
void expand_line(unsigned int at, int x, Buffer *buf);
void new_line(unsigned int at, int x, Buffer *buf);
bool process_keypress(int c, Node **n);

// cursor_in_valid_position.c
void cursor_in_valid_position(Buffer *buf);
void change_position(unsigned int x, unsigned int y, Buffer *buf);

// mouse.c
bool process_mouse_event(MEVENT ev, Node **n);

// utf8.c
void utf8ReadFile(unsigned char uc, uchar32_t *out, FILE *fp_);
int utf8ToMultibyte(uchar32_t c, unsigned char *out, bool validate);
bool validate_utf8(unsigned char *ucs);

// utils.c
void die(const char *s);
char *home_path(const char *path);
char *split_spaces(char *str, char **save);
char **split_str(const char *str, int *num_str);
int calculate_len_line_number(Buffer buf);
int uchar32_cmp(const uchar32_t *s1, const char *s2, unsigned int stringlen);
int uchar32_casecmp(const uchar32_t *s1, const char *s2, unsigned int stringlen);
int uchar32_sub(const uchar32_t *hs, const char *sub, unsigned int hslen, unsigned int sublen);
Line blank_line(void);
char *bufn(int a);

// modify.c
bool modify(Buffer *buf);
bool add_char(int x, int y, uchar32_t c, Buffer *buf);
bool remove_char(int x, int y, Buffer *buf);

// scroll.c
void calculate_scroll(Buffer *buf, int len_line_number);

// buffer_list.c
Node *allocate_node(Node n);
void deallocate_node(Node *n);
Node *single_buffer(Buffer b);
void buffer_add_next(Node *n, Buffer b);
void buffer_add_prev(Node *n, Buffer b);
void buffer_close(Node *n);
void free_buffer_list(Node *n);

// dynamic_string.c
String dynamic_string(const char *x, size_t length);
void dynamic_string_check_available(String *s, size_t length);
void dynamic_string_push(String *s, char x);
void dynamic_string_push_str(String *s, const char *x, size_t length);
bool dynamic_string_pop(String *s);
void dynamic_string_free(String *s);
char *dynamic_string_to_str(String s);
String dynamic_string_concat(String a, String b);
bool dynamic_string_eq(String a, String b);
bool dynamic_string_starts_with(String a, String b);


extern GlobalCfg config;
extern char *menu_message;
extern const uchar32_t substitute_char;
extern const char *substitute_string;
extern jmp_buf end;

#endif

