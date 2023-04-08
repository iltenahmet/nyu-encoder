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
#include <string.h>
#include <pthread.h>
#include "nyuenc.h"

#define INITIAL_INPUT_SIZE 100
#define CHUNK_SIZE 4096

// some ideas about this struct is from https://nachtimwald.com/2019/04/12/thread-pool-in-c/
typedef struct
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int count;
	char *task;
	size_t task_size;
	int terminate;
} ThreadPool;

void down(ThreadPool *pool)
{
	pthread_mutex_lock(&pool->mutex);
	while (pool->count == 0 && !pool->terminate)
		pthread_cond_wait(&pool->cond, &pool->mutex);
	if (!pool->terminate)
	{
		pool->count--;
	}
	pthread_mutex_unlock(&pool->mutex);
}

void up(ThreadPool *pool, const char *task, const size_t task_size)
{
	pthread_mutex_lock(&pool->mutex);
	pool->count++;
	pool->task = malloc(task_size);
	memcpy(pool->task, task, task_size);
	pool->task_size = task_size;
	pthread_cond_signal(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);
}

void *worker(void* arg)
{
	ThreadPool *pool = (ThreadPool*) arg;
	for (;;)
	{
		down(pool);
		if (pool->terminate)
		{
			pthread_exit(NULL);
		}

		//RLE - Run Length Encoding
		char *output = malloc(2 * pool->task_size * sizeof(char));

		char current = pool->task[0];
		int count = 0;
		int output_index = 0;
		for (size_t i = 0; i < pool->task_size; i++)
		{
			// Check if the value is the same as the previous one
			if (pool->task[i] == current)
			{
				count++;
				continue;
			}

			// Add the value and it's count to the output string
			output[output_index++] = current;
			output[output_index++] = (char) count;

			// Increment current, reset count
			current = pool->task[i];
			count = 1;
		}
		output[output_index++] = current;
		output[output_index++] = (char) count;

		write(STDOUT_FILENO, output, output_index);

		free(output);
		free(pool->task);
	}
}

int main(int argc, char *argv[])
{
	char *input = malloc(INITIAL_INPUT_SIZE * sizeof(char));
	size_t size_allocated = INITIAL_INPUT_SIZE;
	size_t size_needed = 0;
	int input_last_char_idx = 0;
	int thread_count = 1;

	for (int i = 1; i < argc; i++)
	{
		// Check if thread count is specified
		if (strcmp(argv[i], "-j") == 0)
		{
			i++;
			thread_count = atoi(argv[i++]);
		}

		// Open the file
		int fd = open(argv[i], O_RDONLY);
		if (fd == -1)
		{
			fprintf(stderr, "Error opening the file.");
			return 1;
		}

		// Get file info
		struct stat sb;
		if (fstat(fd, &sb) == -1)
		{
			fprintf(stderr, "Error getting file information.");
			return 1;
		}

		// Allocate size
		size_needed += sb.st_size;
		if (size_allocated < size_needed)
		{
			input = realloc(input, size_needed * sizeof(char));
			size_allocated = size_needed;
		}

		// Map file into memory
		char *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (addr == MAP_FAILED)
		{
			fprintf(stderr, "Error mapping the file into memory");
			return 1;
		}

		close(fd);

		// add addr to input
		for (int j = 0; j < sb.st_size; j++)
		{
			input[input_last_char_idx + j] = addr[j];
		}
		input_last_char_idx += (int) sb.st_size;
	}

	//Create a thread pool
	ThreadPool pool = {
			.mutex = PTHREAD_MUTEX_INITIALIZER,
			.cond = PTHREAD_COND_INITIALIZER,
			.count = 0,
			.terminate = 0
	};

	pthread_t threads[thread_count];
	for (int i = 0; i < thread_count; i++)
	{
		pthread_create(&threads[i], NULL, worker, &pool);
	}

	//Divide tasks into 4kb chunks
	size_t offset = 0;
	while (offset < size_needed)
	{
		size_t task_size = (size_needed - offset) > CHUNK_SIZE ? CHUNK_SIZE : (size_needed - offset);
		up(&pool, input + offset, task_size);
		offset += task_size;
	}

	pthread_mutex_lock(&pool.mutex);
	pool.terminate = 1;
	pthread_cond_broadcast(&pool.cond);
	pthread_mutex_unlock(&pool.mutex);

	for (int i = 0; i < thread_count; i++)
	{
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&pool.mutex);
	pthread_cond_destroy(&pool.cond);

	free(input);
	return 0;
}
