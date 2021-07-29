#ifndef TED_HEADER
#define TED_HEADER

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

#define READ_BLOCKSIZE 100
#define ctrl(x) ((x) & 0x1f)
#define cx cursor.x
#define cy cursor.y

#define CTRL_KEY_RIGHT 0x232
#define CTRL_KEY_LEFT  0x223

#define NUM_PAIRS 6

#define IN_RANGE(x, min, max)   ((x) >= (min)) && ((x) <= (max))
#define OUT_RANGE(x, min, max)  ((x) < (min)) || ((x) > (max))

#define INPUT_TIMEOUT  5     //timeout for input in ncurses (in milliseconds)

typedef uint32_t uchar32_t;

// message_and_prompt.c
struct HINTS {
    const char *word;
    const char *hint;
};
char *prompt(const char *msgtmp, char *def);
char *prompt_hints(const char *msgtmp, char *def, char *shadow, struct HINTS *hints);
void message(char *msg);

// ted.c
void setcolor(int c);

// config_dialog.c
void config_dialog(void);
bool run_command(char **words, int words_len);
void parse_command(char *command);

// open_and_save.c
void savefile(void);
void read_lines(void);
void detect_linebreak(void);
void openFile(char *fname, bool needs_to_free);
void detect_read_only(char *fname);

// show.c
void show_menu(char *message, char *shadow);
void show_lines(void);

// free.c
void free_lines(void);

// keypress.c
void expandLine(unsigned int at, int x);
void new_line(unsigned int at, int x);
void process_keypress(int c);

// cursor_in_valid_position.c
void cursor_in_valid_position(void);
void change_position(unsigned int x, unsigned int y);

// mouse.c
void processMouseEvent(MEVENT ev);

// utf8.c
void utf8ReadFile(unsigned char uc, uchar32_t *out, FILE *fp_);
int utf8ToMultibyte(uchar32_t c, unsigned char *out, bool validate);
bool validate_utf8(unsigned char *ucs);

// utils.c
void die(const char *s);
char *home_path(const char *path);
char *split_spaces(char *str, char **save);
char **split_str(const char *str, int *num_str);
void calculate_len_line_number(void);
int uchar32_cmp(const uchar32_t *s1, const char *s2, unsigned int stringlen);
int uchar32_casecmp(const uchar32_t *s1, const char *s2, unsigned int stringlen);
int uchar32_sub(const uchar32_t *hs, const char *sub, unsigned int hslen, unsigned int sublen);
struct LINE blank_line(void);


// modify.c
bool modify(void);
bool add_char(int x, int y, uchar32_t c);
bool remove_char(int x, int y);


// Types

struct BUFFER {
    bool modified;
    bool read_only;
    bool can_write;
};

struct CFG {
    bool strict_utf8; // high/low surrogates will be replaced (for now leave it always set)
    unsigned int tablen;
    int lines;
    unsigned char line_break_type; // 0: LF  1: CRLF  2: CR
    bool use_spaces;
    bool autotab;
    bool automatch;
    bool insert_newline;
    struct BUFFER selected_buf;
    char *word_separators;
};


struct LINE {
    unsigned int len;
    uchar32_t *data;
    unsigned int length;
    unsigned int ident;
};


struct CURSOR {
    unsigned int x;
    unsigned int y;
};

struct TEXT_SCROLL {
    unsigned int x;
    unsigned int y;
};

extern struct CFG config;
extern char *filename;
extern unsigned int num_lines;
extern unsigned int len_line_number;
extern struct LINE *lines;
extern FILE *fp;
extern struct CURSOR cursor;
extern struct TEXT_SCROLL text_scroll;
extern unsigned int last_cursor_x;
extern bool colors_on;
extern bool needs_to_free_filename;
extern char *menu_message;
extern struct SHD default_syntax;
extern const uchar32_t substitute_char;
extern const char *substitute_string;
extern jmp_buf end;

#endif

