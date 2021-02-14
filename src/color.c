#include "ted.h"

void syntaxHighlight(unsigned int at) {
    for (unsigned int i = 0; i < lines[at].length; i++) {
        lines[at].color[i] = 0x0;
        const char *string = "if";
        if (lines[at].length - i < strlen(string))
            continue;
        bool c = 0;
        for (unsigned int j = 0; j < strlen(string); j++)
            if ((uchar32_t)string[j] != lines[at].data[i + j]) {
                c = 1;
                break;
            }
        if (c)
            continue;
        for (unsigned int j = 0; j < strlen(string); j++)
            lines[at].color[i + j] = 0x10;
        i += strlen(string) - 1;
    }
}

void readColor(unsigned int at, unsigned int at1, unsigned char *fg, unsigned char *bg) {
    *fg = (lines[at].color[at1] & 0xF0) >> 4;
    *bg = lines[at].color[at1] & 0x0F;
}

