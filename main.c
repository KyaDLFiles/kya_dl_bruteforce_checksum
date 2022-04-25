#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

int get_argument_value(char arg[]) {
	char * endptr;
	int ret;

	if (arg[0] == 'h') {
		ret = strtol(arg + 1, &endptr, 16);
	}
	else {
		ret = strtol(arg, &endptr, 10);
	}

	//In case of invalid values
	if (endptr == arg || *endptr != '\0')
		ret = -1;

	return ret;
}

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
	//Variables definition
	FILE * fp;
	uint8_t *buf;
	uint32_t csum, known_csum, fsize, i, j;
	int start_offset = 0, stop_offset, tot_read;
	_Bool found = 0;

	//Check file path and hash argument
	if (argc < 3) {
		fprintf(stderr, "Missing argument(s)!\n"
						"bfcsum(.exe) /path/to/save/file.dat <h>known_checksum <--start=<h>start_offset>> <--stop=<h>stop_offset>\n"
						"Default offsets are the start and the end of the file\n"
						"Values are read as decimal by default, prepend lowercase \"h\" to have one read as hexadecimal\n");
		return 1;
	}

	//Open file
	if ((fp = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "Error opening file!\n");
		return 2;
	}

	//Get file size and assign it to stop_offset as default
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	stop_offset = (int)fsize;

	//Reset errno and get checksum
	known_csum = get_argument_value(argv[2]);

	//Read other arguments
	for (i = 3; i < argc; i++) {
		if (strncmp("--start=", argv[i], 8) == 0)
			start_offset = get_argument_value(argv[i] + 8);
		else if (strncmp("--stop=", argv[i], 7) == 0)
			stop_offset = get_argument_value(argv[i] + 7);
		else {
			fprintf(stderr, "Error: invalid argument(s)!\n");
			return 5;
		}
	}

	//Exit if values are invalid
	tot_read = stop_offset - start_offset;
	if (start_offset > fsize || stop_offset > fsize || start_offset < 0 || stop_offset < 0 || tot_read <= 0 || tot_read > fsize) {
		fprintf(stderr, "Error: invalid value(s)! Check the arguments\n");
		if (start_offset > fsize || stop_offset > fsize)
			fprintf(stderr, "Offset(s) larger than file size\n");
		return 4;
	}

	//Allocate buffer and read file
	buf = (uint8_t*)malloc(tot_read);
	fseek(fp, (long)start_offset, SEEK_SET);
	fread(buf, tot_read, 1, fp);

	//Start bruteforcing
	for (i = 0; i < tot_read; i++) { //Start from specified value and increase each time
		if (found) //Exit if previously found
			break;
		printf("h%X\n", i + start_offset); //Print current start value to let the user keep track
		#pragma omp parallel for shared(found)
		for (j = tot_read - i; j > 0; j--) { //Start from right after i and increase each time
			if (found) //Skip if already found
				continue;
			csum = compute_checksum(buf + i, tot_read - i - j); //Calculate checksum buffer[i:-j]
			if (csum == known_csum) { //Print and skip all the rest if found (omp doesn't like break)
				printf("Match found for %08X!\n"
					   "Start: h%02X; stop: h%02X\n",
					   csum, i + start_offset, stop_offset - j);
				found = 1;
			}
		}
	}

	//Return 3 if not found
	if (found)
		return 0;
	else {
		printf("No match found :(\n");
		return 3;
	}
}
