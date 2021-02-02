#include "ted.h"

unsigned int calculate_real_cx(unsigned int *last_one_size)
{
    unsigned int real_cx = 0;
    unsigned int offset = 0;
    *last_one_size = 1;
    for (unsigned int i = 0; i < cursor.x; i++)
    {
        if (lines[cy].data[i + offset] >= 0xC0 && lines[cy].data[i + offset] <= 0xDF)
        {
            offset++;
            real_cx++;
            *last_one_size = 2;
        }
        else if (lines[cy].data[i + offset] >= 0xE0 && lines[cy].data[i + offset] <= 0xEF)
        {
            offset += 2;
            real_cx += 2;
            *last_one_size = 3;
        }
        else if (lines[cy].data[i + offset] >= 0xF0 && lines[cy].data[i + offset] <= 0xF7)
        {
            offset += 3;
            real_cx += 3;
            *last_one_size = 4;
        }
        else {
            *last_one_size = 1;
        }
        real_cx++;
    }
    return real_cx;
}
