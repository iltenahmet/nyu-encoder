/*
#nyush

To use valgrind to check for memory leaks and track origins:

valgrind --leak-check=full --track-origins=yes ./nyush

To run a Docker container on WSL:

docker run -i --name cs202 --privileged --rm -t -v /mnt/c/users/ailte/os:/cs202 -w /cs202 ytang/os bash

To run a Docker container on Mac:

docker run -i --name cs202 --privileged --rm -t -v /Users/ahmetilten/OS/labs:/cs202 -w /cs202 ytang/os bash

To zip specific files:

zip nyuenc.zip Makefile *.h *.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "nyuenc.h"

#define INITIAL_INPUT_SIZE 100

int main(int argc, char *argv[]) {

	char* input = malloc(INITIAL_INPUT_SIZE * sizeof(char));
	size_t size_needed = 0;

	for (int i = 1; i <= argc; i++)
	{
		int fd = open(argv[1], O_RDONLY);
		if (fd == -1)
		{
			fprintf(stderr, "Error opening the file.");
			return 1;
		}

		struct stat sb;
		if (fstat(fd, &sb) == -1)
		{
			fprintf(stderr, "Error getting file information.");
			return 1;
		}

		size_needed += sb.st_size;
		if (sizeof(input) < size_needed)
		{
			input = realloc(input, size_needed * sizeof(char));
		}

		char *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (addr == MAP_FAILED)
		{
			fprintf(stderr, "Error mapping the file into memory");
			return 1;
		}

		close(fd);

		//input += addr;
		// add addr to input in a for loop
	}

	char* output = malloc(2 * size_needed * sizeof(char));

	//Run Length Encoding
	char current = input[0];
	int count = 0;
	int output_index = 0;
	for (size_t i = 0; i < size_needed; i++)
	{
		if (input[i] == current)
		{
			count++;
			continue;
		}
		output_index += sprintf(&output[output_index], "%c%c", current, (char) count);
		current = input[i];
		count = 1;
	}
	output_index += sprintf(&output[output_index], "%c%c", current, (char) count);
	write(STDOUT_FILENO, output, output_index);

	free(input);
	free(output);
	return 0;
}
