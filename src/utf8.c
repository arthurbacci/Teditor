#include "ted.h"

void utf8ReadFile(unsigned char uc, unsigned int lc, unsigned int i, FILE *fp) {
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
}

uint16_t utf8ToMultibyte(uchar32_t c, unsigned char *out) {
    out[0] = c % (1 << 8);
    out[1] = (c >> 8) % (1 << 8);
    out[2] = (c >> 16) % (1 << 8);
    out[3] = (c >> 24);

    if (out[0] >= 0xC0 && out[0] <= 0xDF)
        return 2;
    if (out[0] >= 0xE0 && out[0] <= 0xEF)
        return 3;
    if (out[0] >= 0xF0 && out[0] <= 0xF7)
        return 4;
    return 1;
}
