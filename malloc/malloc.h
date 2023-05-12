#ifndef _MALLOC_H_
#define _MALLOC_H_

#define MAX_BLOCKS 32  // maximun ammount of blocks this library will support
#define MIN_SIZE 64    // minimun size of a region
#define SMALL_BLOCK_SIZE 16384
#define MEDIUM_BLOCK_SIZE 1048576
#define LARGE_BLOCK_SIZE 33554432

#define _DEFAULT_SOURCE
#define ALIGN4(s) (((((s) -1) >> 2) << 2) + 4)
#define REGION2PTR(r) ((r) + 1)
#define PTR2REGION(ptr) ((struct region *) (ptr) -1)
#define REGION2BLOCK(r) ((struct block *) (r) -1)
#define BLOCK2REGION(b) ((b) + 1)

#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <stddef.h>

struct malloc_stats {
	int mallocs;
	int frees;
	int requested_memory;
	int blocks;
};

struct region {
	bool free;
	size_t size;
	struct region *next;
	struct region *prev;
};

struct block {
	struct block *next;
	struct block *prev;
};

void *malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);

void get_stats(struct malloc_stats *stats);

static size_t size_that_fits(size_t size);

static void split(struct region *region, size_t size);

static struct region *merge(struct region *region);

static struct region *coalesce(struct region *region);

static void unmap(struct block *block, size_t size);

static struct region *find_in_block(struct region *first, size_t size);

#endif  // _MALLOC_H_
