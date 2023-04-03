/*
#nyush

To use valgrind to check for memory leaks and track origins:

valgrind --leak-check=full --track-origins=yes ./nyush

To run a Docker container on WSL:

docker run -i --name cs202 --privileged --rm -t -v /mnt/c/users/ailte/os:/cs202 -w /cs202 ytang/os bash

To run a Docker container on Mac:

docker run -i --name cs202 --privileged --rm -t -v /users/ahmetilten/OS/labs:/cs202 -w /cs202 ytang/os bash

To zip specific files:

zip nyush.zip Makefile *.h *.c
 */

#include <stdio.h>
#include <string.h>
#include "nyuenc.h"

#define MAX_FILE_COUNT 100
//#define MAX_sda

int main(int argc, char* argv[])
{
//    char* input = "aaaaaabbbbbbbbba";
//
//	FILE *file_ptr;
//	char ch;
//
//	errno_t err_code = fopen_s(argv[1], "r");
//	if (err_code != 0)
//	{
//		printf("Error reading the file");
//		return 1;
//	}

    //RLE
	char prev = input[0];
	int count = 0;
	size_t len = strlen(input);
    for (size_t i = 0; i < len; i++)
	{
		char current = input[i];
		if (current == prev)
		{
			count++;
		} else
		{
			printf("%c%d", prev, count);
			prev = current;
			count = 1;
		}
	}
	printf("%c%d", prev, count);

	fclose(fiel_ptr);
	return 0;
}
