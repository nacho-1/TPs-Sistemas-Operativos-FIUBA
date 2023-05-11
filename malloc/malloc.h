#ifndef _MALLOC_H_
#define _MALLOC_H_

#define MAX_BLOCKS 32  // maximun ammount of blocks this library will support
#define MIN_SIZE 64    // minimun size of a region
#define SMALL_BLOCK_SIZE 16384
#define MEDIUM_BLOCK_SIZE 1048576
#define LARGE_BLOCK_SIZE 33554432

struct malloc_stats {
	int mallocs;
	int frees;
	int requested_memory;
	int blocks;
};

void *malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);

void get_stats(struct malloc_stats *stats);

#endif  // _MALLOC_H_
