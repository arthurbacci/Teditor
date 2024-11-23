#ifndef TED_HEADER
#define TED_HEADER

#define _POSIX_C_SOURCE 1
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <setjmp.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>

#include <ted_string_utils.h>
#include <ted_xdg.h>
#include <ted_die.h>
#include <ted_longjmp.h>
#include <ted_grapheme.h>
#include <ted_buffer.h>
#include <ted_screen.h>
#include <ted_prompt.h>

#include <ted_config.h>

#define NUM_PAIRS 6

#define IN_RANGE(x, min, max)  ((x) >= (min)) && ((x) <= (max))
#define OUT_RANGE(x, min, max) ((x) < (min)) || ((x) > (max))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// timeout for input in ncurses (in milliseconds)
#define INPUT_TIMEOUT 5

#define MSG_SZ 512

#define CMD_WORD_SZ 128
#define CMD_ARR_SZ 30

#define PRETEND_TO_USE(x) (void)(x)


// config_dialog.c
void config_dialog(void);
void parse_command(char *command);

// display.c
void display_menu(const char *message, const char *shadow);
void display_buffer(Buffer buf, int len_line_number);

// keypress.c
void expand_line(Line *ln, size_t x);
void process_keypress(int c);

// modify.c
bool modify(Buffer *buf);
void add_char(Grapheme c, size_t x, Line *ln);
void remove_char(size_t x, Line *ln);

// utils.c
size_t split_cmd_string(const char *s, char ret[CMD_ARR_SZ + 1][CMD_WORD_SZ]);
char *bufn(int a);
size_t get_ident_sz(char *s);
int invoke_editorconfig(const char *prop, const char *filename);
void configure_editorconfig(Buffer *b);
void replace_fd(int fd, const char *filename, int flags);

#endif
