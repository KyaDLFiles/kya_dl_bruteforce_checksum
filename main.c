#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

long long get_argument_value(char arg[]) {
	char * endptr;
	long long ret;

	if (arg[0] == 'h') {
		ret = strtoll(arg + 1, &endptr, 16);
	}
	else {
		ret = strtoll(arg, &endptr, 10);
	}

	//In case of invalid values
	if (endptr == arg || *endptr != '\0')
		ret = -1;

	return ret;
}

uint64_t compute_checksum(const void *buffer, int64_t size) {
	uint64_t result;
	int64_t remaining = (int)size - 1, flag;

	result = 0xFFFFFFFFFFFFFFFFLL;
	if (size) {
		do {
			flag = remaining;
			remaining = (int32_t)remaining - 1;
			result = ((uint32_t)result >> 8) ^ (uint64_t)crc32_tab[(uint8_t)(result ^ *(uint8_t *)buffer)];
			buffer = (uint8_t *)buffer + 1;
		} while ( flag );
	}
	return result;
}

int main(int argc, char *argv[]) {
	//Variables definition
	FILE *fp;
	uint8_t *buf;
	uint32_t csum, fsize;
	int64_t known_csum;
	long long start_offset = 0, stop_offset, tot_read, i, j;
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

	//Get checksum
	known_csum = get_argument_value(argv[2]);

	if (known_csum > 0xFFFFFFFF || known_csum < 0) {
		fprintf(stderr, "Error: invalid checksum!\n"
						"Must me a positive int, at most hFFFFFFFF/4294967295");
		return 5;
	}

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
	buf = (uint8_t *)malloc(tot_read);
	fseek(fp, (long)start_offset, SEEK_SET);
	fread(buf, tot_read, 1, fp);

	//Start bruteforcing
	for (i = 0; i < tot_read; i++) { //Start from specified value and increase each time
		if (found) //Exit if previously found
			break;
		printf("h%llX\n", i + start_offset); //Print current start value to let the user keep track
		#pragma omp parallel for shared(found)
		for (j = 1; j < tot_read - i; j++) { //Start from right after i and increase each time
			if (found) //Skip if already found
				continue;
			csum = compute_checksum(buf + i, j - i); //Calculate checksum buffer[i:-j]
			if (csum == known_csum) { //Print and skip all the rest if found (omp doesn't like break)
				printf("Match found for %08I32X!\n"
					   "Start: h%02llX; stop: h%02llX\n",
					   csum, i + start_offset, start_offset + j);
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
