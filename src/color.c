#include "ted.h"

void syntaxHighlight(void) {
    bool multi_line_comment = 0;
    bool backslash = 0;
    char string = '\0';
    unsigned int waiting_to_close = 0;
    for (unsigned int at = 0; at < num_lines; at++) {
        if (!config.syntax_on) {
            for (unsigned int i = 0; i <= lines[at].length; i++)
                lines[at].color[i] = 0x0;
            continue;
        }
        bool comment = 0;
        for (unsigned int i = 0; i <= lines[at].length; i++) {
            if (lines[at].data[i] == '\\') {
                lines[at].color[i] = string ? config.current_syntax->syntax_string_color : 0x0;
                backslash = !backslash;
                continue;
            }
            if (!(comment || multi_line_comment) && strchr(config.current_syntax->stringchars, lines[at].data[i]) && !backslash) {
                if (!string)
                    string = lines[at].data[i];
                else if (lines[at].data[i] == (uchar32_t)string) {
                    lines[at].color[i] = config.current_syntax->syntax_string_color;
                    string = '\0';
                    continue;
                }
            }
            
            backslash = 0;
            
            if (string) {
                lines[at].color[i] = config.current_syntax->syntax_string_color;
                continue;
            }
        
            unsigned int slinecommentlen   = strlen(config.current_syntax->singleline_comment);
            unsigned int mlinecommentstart = strlen(config.current_syntax->multiline_comment[0]);
            unsigned int mlinecommentend   = strlen(config.current_syntax->multiline_comment[1]);
            char *datachar = malloc(lines[at].length + 1);
            
            for (unsigned int l = 0; l <= lines[at].length; l++)
                datachar[l] = (char)lines[at].data[l];
                
            if (lines[at].length >= slinecommentlen && i <= lines[at].length - slinecommentlen
            && memcmp(&datachar[i], config.current_syntax->singleline_comment, slinecommentlen) == 0)
                comment = 1;
            else if (lines[at].length >= slinecommentlen && i <= lines[at].length - mlinecommentstart
            && memcmp(&datachar[i], config.current_syntax->multiline_comment[0], mlinecommentstart) == 0)
                multi_line_comment = 1;
            else if (i >= mlinecommentend
            && memcmp(&datachar[i - mlinecommentend], config.current_syntax->multiline_comment[1], mlinecommentend) == 0)
                multi_line_comment = 0;
                
            free(datachar);
            
            lines[at].color[i] = comment || multi_line_comment ? config.current_syntax->syntax_comment_color : 0x0;
            if (comment || multi_line_comment) continue;
            
            // if lines[at].data[i] is a null byte, strchr will return
            if (lines[at].data[i] && 
                (strchr(config.current_syntax->match[0], lines[at].data[i]) || strchr(config.current_syntax->match[1], lines[at].data[i]))
            ) {
                bool opening = strchr(config.current_syntax->match[0], lines[at].data[i]);
                
                if (waiting_to_close && !opening) {
                    if (waiting_to_close == 1)
                        lines[at].color[i] = config.current_syntax->match_color;
                    waiting_to_close--;
                    continue;
                } else if (waiting_to_close && opening)
                    waiting_to_close++;
                
                if (at == cursor.y && i + 1 == cursor.x) {
                    lines[at].color[i] = config.current_syntax->match_color;
                    if (opening) {
                        waiting_to_close = 1;
                    } else {
                        unsigned int lay = 1;
                        for (int _at = at; _at >= 0; _at--) {
                            for (int _i = (int)at == _at ? i : lines[_at].length - 1; _i >= 0; _i--) {
                                if (strchr(config.current_syntax->match[0], lines[_at].data[_i])) {
                                    lay--;
                                    if (lay == 1) {
                                        lines[_at].color[_i] = config.current_syntax->match_color;
                                        _at = -1;
                                        break;
                                    }
                                } else if (strchr(config.current_syntax->match[1], lines[_at].data[_i])) {
                                    lay++;
                                }
                            }
                        }
                    }
                }
            }
            
            for (unsigned int k = 0; k < config.current_syntax->kwdlen; k++) {
                unsigned int stringlen = strlen(config.current_syntax->keywords[k].string);
                if (lines[at].length - i < stringlen) continue;

                if ((i != 0 && !strchr(config.current_syntax->word_separators, lines[at].data[i - 1]))
                    || !strchr(config.current_syntax->word_separators, lines[at].data[i + stringlen]))
                    continue;

                bool c = 0;
                for (unsigned int j = 0; j < stringlen; j++)
                    if ((uchar32_t)config.current_syntax->keywords[k].string[j] != lines[at].data[i + j]) {
                        c = 1;
                        break;
                    }
                if (c) continue;
                for (unsigned int j = 0; j < stringlen; j++)
                    lines[at].color[i + j] = config.current_syntax->keywords[k].color;
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
