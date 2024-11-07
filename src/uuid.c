#include "ted.h"

void encode_uuid(const unsigned char bit[16], char uuid[37]) {
    snprintf(uuid, 37,
        "%02X%02X%02X%02X"
        "-" "%02X%02X"
        "-" "%02X%02X"
        "-" "%02X%02X"
        "-" "%02X%02X%02X%02X%02X%02X",
        bit[0], bit[1], bit[2], bit[3], bit[4], bit[5], bit[6], bit[7],
        bit[8], bit[9], bit[10], bit[11], bit[12], bit[13], bit[14], bit[15]);
}

void uuid_version4(char uuid[37]) {
    srandom(time(NULL));

    unsigned char r[16];
    for (size_t i = 0; i < sizeof(r); i++) {
        long rl = random();
        char *rc = (char *)&rl;
        r[i] = rc[3];
    }
    r[6] |= 0x40;
    r[6] &= 0x4F;
    r[8] |= 0x80;
    r[8] &= 0xBF;
    
    encode_uuid(r, uuid);
}

