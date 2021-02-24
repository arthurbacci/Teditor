#include "ted.h"

void syntaxHighlight(unsigned int at) {
    fprintf(stderr, "START\n");
    bool comment = 0;
    for (unsigned int i = 0; i < lines[at].length; i++) {
        if (i < lines[at].length - 1 && lines[at].data[i] == '/' && lines[at].data[i + 1] == '/')
            comment = 1;
        lines[at].color[i] = comment ? 0x50 : 0x0;
        if (comment) continue;
        
        for (unsigned int k = 0; k < config.kwdlen; k++) {
            unsigned int stringlen = strlen(config.keywords[k].string);
            if (lines[at].length - i < stringlen) continue;
            bool c = 0;
            for (unsigned int j = 0; j < stringlen; j++)
                if ((uchar32_t)config.keywords[k].string[j] != lines[at].data[i + j]) {
                    c = 1;
                    break;
                }
            if (c)
                continue;
            for (unsigned int j = 0; j < stringlen; j++)
                lines[at].color[i + j] = config.keywords[k].color;
            i += stringlen - 1;
            break;
        }
    }
}

void readColor(unsigned int at, unsigned int at1, unsigned char *fg, unsigned char *bg) {
    *fg = (lines[at].color[at1] & 0xF0) >> 4;
    *bg = lines[at].color[at1] & 0x0F;
}

