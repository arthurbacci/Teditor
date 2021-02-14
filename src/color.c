#include "ted.h"

void syntaxHighlight(unsigned int at) {
    for (unsigned int i = 0; i < lines[at].length; i++) {
        if (lines[at].data[i] != '(' && lines[at].data[i] != ')')
            lines[at].color[i] = 0x0;
        else
            lines[at].color[i] = 0x10;
    }
}

void readColor(unsigned int at, unsigned int at1, unsigned char *fg, unsigned char *bg) {
    *fg = (lines[at].color[at1] & 0xF0) >> 4;
    *bg = lines[at].color[at1] & 0x0F;
}

