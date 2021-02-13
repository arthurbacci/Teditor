#include "ted.h"

void utf8ReadFile(unsigned char uc, unsigned int lc, FILE *fp) {
    if (uc >= 0xC0 && uc <= 0xDF) {
        lines[i].data[lc] = uc;
        lines[i].data[lc] += fgetc(fp) << 8;
    } else if (uc >= 0xE0 && uc <= 0xEF) {
        lines[i].data[lc] = uc;
        lines[i].data[lc] += fgetc(fp) << 8;
        lines[i].data[lc] += fgetc(fp) << 16;
    } else if (uc >= 0xF0 && uc <= 0xF7) {
        lines[i].data[lc] = uc;
        lines[i].data[lc] += fgetc(fp) << 8;
        lines[i].data[lc] += fgetc(fp) << 16;
        lines[i].data[lc] += fgetc(fp) << 24;
    } else
        lines[i].data[lc] = uc;
    return 0;
}
