#include "ted.h"

void set_syntax_change(unsigned int at, bool update_fast) {
    config.selected_buf.syntax_at = at; // update from current position
    syntax_change = 1, syntax_update_fast = update_fast;// signal change to syntaxHighlight
}

void reset_brackets(void) {
    for (unsigned int i = 0; i < config.selected_buf.brackets_len; i++) {
        struct CURSOR pos = config.selected_buf.highlighted_brackets[i];
        lines[pos.y].color[pos.x] = config.current_syntax->match_color;
    }

    config.selected_buf.brackets_len = 0;
    free(config.selected_buf.highlighted_brackets);
    config.selected_buf.highlighted_brackets = NULL;
}

void add_highlighted_bracket(unsigned int at, unsigned int x) {
    config.selected_buf.highlighted_brackets = realloc(config.selected_buf.highlighted_brackets, ++config.selected_buf.brackets_len * sizeof(struct CURSOR));
    config.selected_buf.highlighted_brackets[config.selected_buf.brackets_len - 1].y = at;
    config.selected_buf.highlighted_brackets[config.selected_buf.brackets_len - 1].x = x;
}

int syntaxHighlight(void) {
    struct itimerval interval = {0}, zeroed = {0};// set preemptive timer
    interval.it_value.tv_usec = SYNTAX_TIMEOUT;
    setitimer(ITIMER_REAL, &interval, NULL);

    if (config.current_syntax == &default_syntax) {// just reset color to all lines
        for (unsigned int at = config.selected_buf.syntax_at; at < num_lines; at++) {
            if (syntax_yield) {
                syntax_yield = 0;
                config.selected_buf.syntax_at = at - (at > 0);
                return SYNTAX_TODO;
            }
            memset(lines[at].color, 0, (lines[at].length + 1) * sizeof(*lines[at].color));
            memset(&lines[at].state, 0, sizeof(lines[at].state));
        }
        goto END;
    }
    unsigned int slinecommentlen = config.current_syntax->singleline_comment.length; //todo: these are superfluous
    unsigned int mlinecommentstart = config.current_syntax->multiline_comment[0].length;
    unsigned int mlinecommentend = config.current_syntax->multiline_comment[1].length;
    unsigned int hexprefixlen = config.current_syntax->number_prefix[0].length;
    unsigned int octprefixlen = config.current_syntax->number_prefix[1].length;
    unsigned int binprefixlen = config.current_syntax->number_prefix[2].length;

    for (unsigned int at = config.selected_buf.syntax_at; at < num_lines; at++) {
        if (syntax_yield) {//save state
            config.selected_buf.syntax_at = at - (at > 0);
            syntax_yield = 0;
            return SYNTAX_TODO;
        }
        // restore a state consistent with the previous line
        if (at == 0) memset(&lines[at].state, 0, sizeof(lines[at].state));
        else memcpy(&lines[at].state, &lines[at - 1].state, sizeof(lines[at].state));

        memset(lines[at].color, 0, (lines[at].length + 1) * sizeof(*lines[at].color));
        bool comment = 0;
        for (unsigned int i = 0; i <= lines[at].length; i++) {
            if (lines[at].data[i] == '\\') {
                lines[at].color[i] = lines[at].state.string ? config.current_syntax->string_color : 0;
                lines[at].state.backslash = !lines[at].state.backslash;
                for (unsigned int j = 0; j < config.current_syntax->stringmatch_len; j++)
                    if (!uchar32_cmp(&lines[at].data[i], config.current_syntax->stringmatch[j].name, config.current_syntax->stringmatch[j].length)) {
                        unsigned int len = config.current_syntax->stringmatch[j].length;
                        memset(&lines[at].color[i], config.current_syntax->stringmatch_color, len);
                        i += len - 1;
                        if (strchr(config.current_syntax->stringmatch[j].name, '\\'))
                            lines[at].state.backslash = !lines[at].state.backslash;
                        break;
                    }
                continue;
            }
            if (!(comment || lines[at].state.multi_line_comment)
                && strchr(config.current_syntax->stringchars, lines[at].data[i]) && !lines[at].state.backslash) {
                if (!lines[at].state.string)
                    lines[at].state.string = lines[at].data[i];
                else if (lines[at].data[i] == (uchar32_t)lines[at].state.string) {
                    lines[at].color[i] = config.current_syntax->string_color;
                    lines[at].state.string = '\0';
                    continue;
                }
            }
            lines[at].state.backslash = 0;
            
            if (lines[at].state.string) {
                unsigned int len = 0;
                for (unsigned int j = 0; j < config.current_syntax->stringmatch_len; j++)
                    if (!uchar32_cmp(&lines[at].data[i], config.current_syntax->stringmatch[j].name, config.current_syntax->stringmatch[j].length)) {
                        len = config.current_syntax->stringmatch[j].length;
                        break;
                    }

                if (len) {
                    memset(&lines[at].color[i], config.current_syntax->stringmatch_color, len);
                    i += len - 1;
                } else
                    lines[at].color[i] = config.current_syntax->string_color;
                continue;
            }
        
            char *datachar = malloc(lines[at].length + 1);
            for (unsigned int l = 0; l <= lines[at].length; l++)
                datachar[l] = (char)lines[at].data[l];
                
            if (slinecommentlen != 0 &&
                lines[at].length >= slinecommentlen && i <= lines[at].length - slinecommentlen &&
                memcmp(&datachar[i], config.current_syntax->singleline_comment.name, slinecommentlen) == 0)
                comment = 1;

            else if (mlinecommentstart != 0
                     && lines[at].length >= slinecommentlen && i <= lines[at].length - mlinecommentstart
                     && memcmp(&datachar[i], config.current_syntax->multiline_comment[0].name, mlinecommentstart) == 0)
                lines[at].state.multi_line_comment = 1;

            else if (mlinecommentend != 0 && i >= mlinecommentend
                     && memcmp(&datachar[i - mlinecommentend], config.current_syntax->multiline_comment[1].name, mlinecommentend) == 0)
                lines[at].state.multi_line_comment = 0;

            free(datachar);
            lines[at].color[i] = comment || lines[at].state.multi_line_comment ? config.current_syntax->comment_color : 0;
            if (comment || lines[at].state.multi_line_comment) continue;

            bool opening = strchr(config.current_syntax->match[0], lines[at].data[i]);
            bool closing = strchr(config.current_syntax->match[1], lines[at].data[i]);
            if (opening || closing) lines[at].color[i] = config.current_syntax->match_color;

            if (lines[at].data[i] && (opening || closing)) {
                if (lines[at].state.waiting_to_close && !opening) {
                    if (lines[at].state.waiting_to_close == 1) {
                        lines[at].color[i] = config.current_syntax->hover_match_color;
                        add_highlighted_bracket(at, i);
                    }
                    lines[at].state.waiting_to_close--;
                    continue;
                } else if (lines[at].state.waiting_to_close && opening)
                    lines[at].state.waiting_to_close++;
                
                if (at == cursor.y && (i + 1) == cursor.x) {
                    lines[at].color[i] = config.current_syntax->hover_match_color;
                    add_highlighted_bracket(at, i);

                    if (opening) {
                        lines[at].state.waiting_to_close = 1;
                    } else {
                        int lay = 1;
                        for (int _at = at; _at >= 0; _at--) {
                            for (int _i = (int)at == _at ? i : lines[_at].length - 1; _i >= 0; _i--) {
                                if (strchr(config.current_syntax->match[0], lines[_at].data[_i])) {
                                    lay--;
                                    if (lay == 1) {
                                        add_highlighted_bracket(at, i);
                                        lines[_at].color[_i] = config.current_syntax->hover_match_color;
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

            if (i == 0 || strchr(config.current_syntax->word_separators, lines[at].data[i - 1])) {
                unsigned int numlen = 0, prefixlen = 0, suffixlen = 0;
                const char *numbers = config.current_syntax->number_strings[3];

                if (hexprefixlen != 0 && lines[at].length - i >= hexprefixlen
                    && !uchar32_casecmp(&lines[at].data[i], config.current_syntax->number_prefix[0].name, hexprefixlen)
                    && ((hexprefixlen >= octprefixlen && hexprefixlen >= binprefixlen)
                        || (uchar32_casecmp(&lines[at].data[i], config.current_syntax->number_prefix[1].name, octprefixlen)
                            && uchar32_casecmp(&lines[at].data[i], config.current_syntax->number_prefix[2].name, binprefixlen)))) {
                    prefixlen = hexprefixlen;
                    numbers = config.current_syntax->number_strings[0];

                } else if (octprefixlen != 0 && lines[at].length - i >= octprefixlen
                           && !uchar32_casecmp(&lines[at].data[i], config.current_syntax->number_prefix[1].name, octprefixlen)
                           && ((octprefixlen >= binprefixlen)
                               || uchar32_casecmp(&lines[at].data[i], config.current_syntax->number_prefix[2].name, binprefixlen))) {
                    prefixlen = octprefixlen;
                    numbers = config.current_syntax->number_strings[1];

                } else if (binprefixlen != 0 && lines[at].length - i >= binprefixlen
                           && !uchar32_casecmp(&lines[at].data[i], config.current_syntax->number_prefix[2].name, binprefixlen)) {
                    prefixlen = binprefixlen;
                    numbers = config.current_syntax->number_strings[2];
                }

                numlen = prefixlen;
                while ((i + numlen) < lines[at].length && strchr(numbers, lines[at].data[i + numlen])) numlen++;

                if (numlen != 0) {
                    while ((i + numlen + suffixlen) < lines[at].length
                    && strchr(config.current_syntax->number_suffixes, lines[at].data[i + numlen + suffixlen])) suffixlen++;

                    if ((i + numlen + suffixlen) == lines[at].length
                        || strchr(config.current_syntax->word_separators, lines[at].data[i + numlen + suffixlen])) {
                        unsigned int start = 0;

                        if (numlen == prefixlen) {
                            for (unsigned int j = 0; j < prefixlen; j++)
                                if (!strchr(numbers, lines[at].data[i + j])) {
                                    start = prefixlen;
                                    break;
                                }
                        } else
                            memset(&lines[at].color[i], config.current_syntax->number_prefix_color, prefixlen);

                        for (unsigned int j = start; j < numlen; j++)
                            if (!lines[at].color[i + j])
                                lines[at].color[i + j] = config.current_syntax->number_color;

                        memset(&lines[at].color[i + numlen], config.current_syntax->number_suffix_color, suffixlen);
                        i += numlen + suffixlen;
                    }
                }
            }
            for (unsigned int k = 0; k < config.current_syntax->kwdlen; k++) {
                unsigned int stringlen = config.current_syntax->keywords[k].length;
                if (lines[at].length - i < stringlen) continue;

                if (!config.current_syntax->keywords[k].operator) {
                    if ((i != 0 && !strchr(config.current_syntax->word_separators, lines[at].data[i - 1]))
                        || !strchr(config.current_syntax->word_separators, lines[at].data[i + stringlen]))
                        continue;
                }
                if (uchar32_cmp(&lines[at].data[i], config.current_syntax->keywords[k].string, stringlen))
                    continue;

                for (unsigned int j = 0; j < stringlen; j++)
                    if (!lines[at].color[i + j])
                        lines[at].color[i + j] = config.current_syntax->keywords[k].color;
                i += stringlen - 1;
                break;
            }
        }
        
        if (syntax_update_fast && (at > config.selected_buf.syntax_at && at > 0) && //highlight at least one line
            (lines[at].state.backslash == lines[at + ((at + 1) < num_lines)].state.backslash &&
            lines[at].state.multi_line_comment == lines[at + ((at + 1) < num_lines)].state.multi_line_comment &&
            lines[at].state.string == lines[at + ((at + 1) < num_lines)].state.string &&
            lines[at].state.waiting_to_close == lines[at + ((at + 1) < num_lines)].state.waiting_to_close)) {
            syntax_update_fast = 0;
            goto END;
        }
    }
END:
    setitimer(ITIMER_REAL, &zeroed, NULL);
    syntax_yield = 0;
    return SYNTAX_END;
}

void readColor(unsigned int at, unsigned int at1, unsigned char *fg, unsigned char *bg) {
    *fg = (lines[at].color[at1] & 0xF0) >> 4;
    *bg = lines[at].color[at1] & 0x0F;
}
