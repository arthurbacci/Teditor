#include "ted.h"

void syntaxHighlight(unsigned int at) {
    const char *strings[] = {"if", "else", "int ", "char ", "*", "\"", "\'", "//", "unsigned ", "long ", "double ", "float ", "struct ", "const ", "return", ";"};
    unsigned int slen = sizeof(strings) / sizeof(char *);
    for (unsigned int i = 0; i < lines[at].length; i++) {
        lines[at].color[i] = 0x0;
        for (unsigned int k = 0; k < slen; k++) {
            if (lines[at].length - i < strlen(strings[k]))
                continue;
            bool c = 0;
            for (unsigned int j = 0; j < strlen(strings[k]); j++)
                if ((uchar32_t)strings[k][j] != lines[at].data[i + j]) {
                    c = 1;
                    break;
                }
            if (c)
                continue;
            for (unsigned int j = 0; j < strlen(strings[k]); j++)
                lines[at].color[i + j] = 0x10;
            i += strlen(strings[k]) - 1;
            break;
        }
    }
}

void readColor(unsigned int at, unsigned int at1, unsigned char *fg, unsigned char *bg) {
    *fg = (lines[at].color[at1] & 0xF0) >> 4;
    *bg = lines[at].color[at1] & 0x0F;
}

