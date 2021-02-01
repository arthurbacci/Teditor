#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define READ_BLOCKSIZE 10
#define ctrl(x) ((x) & 0x1f)
#define cx cursor.x
#define cy cursor.y

#define CTRL_KEY_RIGHT 0x232
#define CTRL_KEY_LEFT  0x223

// message_and_prompt.c
int message(char *msg);
char *prompt(char *msg);

// ted.c
void setcolor(int c);

struct CFG
{
    unsigned int tablen;
    unsigned int LINES;
    unsigned char line_break_type : 2; // 0: LF  1: CRLF  2: CR
};

extern struct CFG config;

