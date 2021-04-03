#include "ted.h"

// Displayed instead of invalid utf8 (codepoint U+FFFD)
const uchar32_t substitute_char = 0xEF + (0xBF << 8) + (0xBD << 16);
const char *substitute_string = "\xEF\xBF\xBD";

/*
Utf8 sequences range
1 byte sequence (1* INCLUSIVE 0x00 - x7F)
2 bytes sequence (1* INCLUSIVE 0xC2 - 0xDF) (2* INCLUSIVE 0x80 - 0xBF)
3 bytes sequence (1* INCLUSIVE 0xE0 - 0xEF) (2* INCLUSIVE 0xA0 - 0xBF) (3* INCLUSIVE 0x80 - 0xBF)
4 bytes sequence (1* INCLUSIVE 0xF0 - 0xF4) (2* INCLUSIVE 0x90 - 0xBF) (3* INCLUSIVE 0x80 - 0xBF) (4* INCLUSIVE 0x80 - 0xBF)
*/

void utf8ReadFile(unsigned char uc, uchar32_t *out, FILE *fp_) {
    if (uc <= 0x7F) {
        *out = uc;

    } else if (IN_RANGE(uc, 0xC2, 0xDF)) {
        int uc2 = fgetc(fp_);
        if (OUT_RANGE(uc2, 0x80, 0xBF)) {
            ungetc(uc2, fp_);
            goto invalid;
        }

        *out = uc;
        *out += (unsigned int)uc2 << 8;

    } else if (IN_RANGE(uc, 0xE0, 0xEF) && (!config.strict_utf8 || uc != 0xED)) {
        int uc2 = fgetc(fp_);
        if (OUT_RANGE(uc2, 0xA0, 0xBF)) {
            ungetc(uc2, fp_);
            goto invalid;
        }

        int uc3 = fgetc(fp_);
        if (OUT_RANGE(uc3, 0x80, 0xBF)) {
            ungetc(uc3, fp_);
            ungetc(uc2, fp_);
            goto invalid;
        }

        *out = uc;
        *out += (unsigned int)uc2 << 8;
        *out += (unsigned int)uc3 << 16;

    } else if (IN_RANGE(uc, 0xF0, 0xF4)) {
        int uc2 = fgetc(fp_);
        if (OUT_RANGE(uc2, 0x90, 0xBF)) {
            ungetc(uc2, fp_);
            goto invalid;
        }

        int uc3 = fgetc(fp_);
        if (OUT_RANGE(uc3, 0x80, 0xBF)) {
            ungetc(uc3, fp_);
            ungetc(uc2, fp_);
            goto invalid;
        }

        int uc4 = fgetc(fp_);
        if (OUT_RANGE(uc4, 0x80, 0xBF)) {
            ungetc(uc4, fp_);
            ungetc(uc3, fp_);
            ungetc(uc2, fp_);
            goto invalid;
        }

        *out = uc;
        *out += (unsigned int)uc2 << 8;
        *out += (unsigned int)uc3 << 16;
        *out += (unsigned int)uc4 << 24;
    } else goto invalid;
    return;

invalid:
    *out = uc; //threat as a single char
}

int utf8ToMultibyte(uchar32_t c, unsigned char *out, bool validate) {
    out[0] = (unsigned char)(c % (1 << 8));
    out[1] = (unsigned char)((c >> 8) % (1 << 8));
    out[2] = (unsigned char)((c >> 16) % (1 << 8));
    out[3] = (unsigned char)(c >> 24);

    if (out[0] <= 0x7F)
        return 1;

    if (IN_RANGE(out[0], 0xC2, 0xDF)) {
        if (OUT_RANGE(out[1], 0x80, 0xBF))
            goto invalid;
        return 2;
    }
    if (IN_RANGE(out[0], 0xE0, 0xEF)) {
        if (OUT_RANGE(out[1], 0xA0, 0xBF) || OUT_RANGE(out[2], 0x80, 0xBF) || (config.strict_utf8 && out[0] == 0xED))
            goto invalid;
        return 3;
    }
    if (IN_RANGE(out[0], 0xF0, 0xF4)) {
        if (OUT_RANGE(out[1], 0x90, 0xBF) || OUT_RANGE(out[2], 0x80, 0xBF) || OUT_RANGE(out[3], 0x80, 0xBF))
            goto invalid;
        return 4;
    }

invalid: // may do other things here
    return validate ? -1 : 1; //threat as a single char
}

bool validate_utf8(unsigned char *ucs) {
    if (ucs[0] <= 0x7F)
        return 1;
    if (IN_RANGE(ucs[0], 0xC2, 0xDF)) {
        if (OUT_RANGE(ucs[1], 0x80, 0xBF))
            return 0;
        return 1;
    }
    if (IN_RANGE(ucs[0], 0xE0, 0xEF)) {
        if (OUT_RANGE(ucs[1], 0xA0, 0xBF) || OUT_RANGE(ucs[2], 0x80, 0xBF) || (config.strict_utf8 && ucs[0] == 0xED))
            return 0;
        return 1;
    }
    if (IN_RANGE(ucs[0], 0xF0, 0xF4)) {
        if (OUT_RANGE(ucs[1], 0x90, 0xBF) || OUT_RANGE(ucs[2], 0x80, 0xBF) || OUT_RANGE(ucs[3], 0x80, 0xBF))
            return 0;
        return 1;
    }
    return 0;
}
