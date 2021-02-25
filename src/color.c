#include "ted.h"

void syntaxHighlight() {
    bool multi_line_comment = 0;
    bool backslash = 0;
    char string = '\0';
    for (unsigned int at = 0; at < num_lines; at++) {
        bool comment = 0;
        for (unsigned int i = 0; i <= lines[at].length; i++) {
            if (lines[at].data[i] == '\\') {
                lines[at].color[i] = string ? config.syntax_string_color : 0x0;
                backslash = !backslash;
                continue;
            }
            if (!(comment || multi_line_comment) && strchr(config.stringchars, lines[at].data[i]) && !backslash) {
                if (!string)
                    string = lines[at].data[i];
                else if (lines[at].data[i] == (uchar32_t)string) {
                    lines[at].color[i] = config.syntax_string_color;
                    string = '\0';
                    continue;
                }
            }
            
            backslash = 0;
            
            if (string) {
                lines[at].color[i] = config.syntax_string_color;
                continue;
            }
        
            unsigned int slinecommentlen   = strlen(config.singleline_comment);
            unsigned int mlinecommentstart = strlen(config.multiline_comment[0]);
            unsigned int mlinecommentend   = strlen(config.multiline_comment[1]);
            char *datachar = malloc(lines[at].length + 1);
            
            for (unsigned int l = 0; l <= lines[at].length; l++)
                datachar[l] = (char)lines[at].data[l];
                
            if (lines[at].length >= slinecommentlen && i <= lines[at].length - slinecommentlen
            && memcmp(&datachar[i], config.singleline_comment, slinecommentlen) == 0)
                comment = 1;
            else if (lines[at].length >= slinecommentlen && i <= lines[at].length - mlinecommentstart
            && memcmp(&datachar[i], config.multiline_comment[0], mlinecommentstart) == 0)
                multi_line_comment = 1;
            else if (i >= mlinecommentend
            && memcmp(&datachar[i - mlinecommentend], config.multiline_comment[1], mlinecommentend) == 0)
                multi_line_comment = 0;
                
            free(datachar);
            
            lines[at].color[i] = comment || multi_line_comment ? config.syntax_comment_color : 0x0;
            if (comment || multi_line_comment) continue;
        
            for (unsigned int k = 0; k < config.kwdlen; k++) {
                unsigned int stringlen = strlen(config.keywords[k].string);
                if (lines[at].length - i < stringlen) continue;
                bool c = 0;
                for (unsigned int j = 0; j < stringlen; j++)
                    if ((uchar32_t)config.keywords[k].string[j] != lines[at].data[i + j]) {
                        c = 1;
                        break;
                    }
                if (c) continue;
                for (unsigned int j = 0; j < stringlen; j++)
                    lines[at].color[i + j] = config.keywords[k].color;
                i += stringlen - 1;
                break;
            }
        }
    }
}

void readColor(unsigned int at, unsigned int at1, unsigned char *fg, unsigned char *bg) {
    *fg = (lines[at].color[at1] & 0xF0) >> 4;
    *bg = lines[at].color[at1] & 0x0F;
}

