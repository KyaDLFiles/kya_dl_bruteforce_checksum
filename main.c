#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

uint64_t compute_checksum(const void *buffer, int64_t size) {
    uint64_t result;
    int64_t v4;
    uint8_t v5;
    int64_t v6;

    result = 0xFFFFFFFFFFFFFFFFLL;
    v4 = (int)size - 1;
    if (size) {
        do {
            v5 = *(uint8_t *)buffer;
            v6 = v4;
            v4 = (int32_t)v4 - 1;
            buffer = (uint8_t *)buffer + 1;
            result = ((uint32_t)result >> 8) ^ (uint64_t)crc32_tab[(uint8_t)(result ^ v5)];
        } while ( v6 );
    }
    return result;
}

int main(int argc, char *argv[]) {
    //Variable definitions
    FILE * fp;
    uint8_t *buf;
    uint32_t csum, known_csum;
    int i, j, fsize, start_value;

    //Check file path argument
    if (argc < 5) {
        printf("Missing argument!\n"
               "Usage: kdl_bf(.exe) /path/to/save/file.dat filesize(hex) known_csum(dec) start_value(hex)\n"
               "filesize can be set to \"auto\" to make it automatic");
        return 1;
    }

    //Open file
    if ((fp = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening file!\n");
        return 2;
    }

    //Get file size
    if (strcmp(argv[2], "auto") == 0) {
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
    } else if (argv[2][0] == 'h')
        fsize = strtol(argv[2] + 1, NULL, 16);
    else
        fsize = strtol(argv[2], NULL, 10);

    //Get known checksum
    if (argv[3][0] == 'h')
        known_csum = strtoul(argv[3] + 1, NULL, 16);
    else
        known_csum = strtoul(argv[3], NULL, 10);

    //Get starting position
    if (argv[4][0] == 'h')
        start_value = strtol(argv[4] + 1, NULL, 16);
    else
        start_value = strtol(argv[4], NULL, 10);

    //Allocate buffer and read file
    buf = (uint8_t*)malloc(fsize);
    fread(buf, fsize, 1, fp);

    //Start bruteforcing
    for (i = start_value; i < fsize; i++) { //Start from specified value and increase each time
        printf("%X\n", i); //Print current start value to let the user keep track
        for (j = fsize - i; j > 0; j--) { //Start from right after i and increase each time
            csum = compute_checksum(buf + i, fsize - i - j); //Calculate checksum buffer[i:-j]
            if (csum == known_csum) { //Print and return 0 if checksum is found
                printf("%08X! %02X:%02X", csum, i, fsize - j);
                return 0;
            }
        }
    }

    //Return 3 if not found
    return 3;
}
