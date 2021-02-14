#include "ted.h"

void syntaxHighlight(unsigned int at) {
    memset(lines[at].color, 0, lines[at].length);
}

