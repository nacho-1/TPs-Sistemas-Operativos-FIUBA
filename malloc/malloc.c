#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "malloc.h"
#include "printfmt.h"

#define ALIGN4(s) (((((s) -1) >> 2) << 2) + 4)
#define REGION2PTR(r) ((r) + 1)
#define PTR2REGION(ptr) ((struct region *) (ptr) -1)

#define MIN_SIZE 64

#define SMALL_BLOCK_SIZE 16384

struct region {
	bool free;
	size_t size;
	struct region *next;
	struct region *prev;
};

// List of free regions kept in order of memory address.
struct region *free_regions_head = NULL;

int amount_of_mallocs = 0;
int amount_of_frees = 0;
int requested_memory = 0;

void split_region(struct region *region, size_t size);

/*
 * Insert region in the list of free regions.
 * Coalesce if possible.
 */
void free_region(struct region *region);

void coalesce_region(struct region *region);

// finds the next free region
// that holds the requested size
//
static struct region *
find_free_region(size_t size)
{
	struct region *next = free_regions_head;

#ifdef FIRST_FIT
	while (next != NULL) {
		if (next->size >= size)
			break;
		else
			next = next->next;
	}
#endif

#ifdef BEST_FIT
	// Your code here for "best fit"
#endif

	return next;
}

static struct region *
grow_heap(size_t size)
{
	struct region *addr;

	addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (addr == MAP_FAILED)
		return NULL;

	addr->free = true;
	addr->size = size - sizeof(struct region);
	addr->next = NULL;
	addr->prev = NULL;

	free_region(addr);

	return addr;
}

/// Public API of malloc library ///

void *
malloc(size_t size)
{
	// aligns to multiple of 4 bytes
	size = ALIGN4(size);

	// minimun size of region is MIN_SIZE
	size = size > MIN_SIZE ? size : MIN_SIZE;

	// for now this is max size
	if (size >= SMALL_BLOCK_SIZE - sizeof(struct region))
		return NULL;

	struct region *region = find_free_region(size);

	if (region == NULL) {
		// for now we only work with 1 block
		if (amount_of_mallocs > 0)
			return NULL;

		region = grow_heap(SMALL_BLOCK_SIZE);
		if (region == NULL)
			return NULL;
	}

	split_region(region, size);

	// remove the region from the list of free regions
	if (region->prev != NULL)
		region->prev->next = region->next;
	if (region->next != NULL)
		region->next->prev = region->prev;

	region->free = false;

	// updates statistics
	amount_of_mallocs++;
	requested_memory += size;

	return REGION2PTR(region);
}

void
free(void *ptr)
{
	if (ptr == NULL)
		return;

	struct region *curr = PTR2REGION(ptr);
	assert(!curr->free);

	free_region(curr);

	// updates statistics
	amount_of_frees++;
}

void *
calloc(size_t nmemb, size_t size)
{
	// Your code here

	return NULL;
}

void *
realloc(void *ptr, size_t size)
{
	// Your code here

	return NULL;
}

void
get_stats(struct malloc_stats *stats)
{
	stats->mallocs = amount_of_mallocs;
	stats->frees = amount_of_frees;
	stats->requested_memory = requested_memory;
}

void split_region(struct region *region, size_t size)
{
	if (region->size < size + sizeof(struct region) + MIN_SIZE)
		return;

	char *addr = ((char *) (region + 1)) + size;

	struct region *new_region = (struct region *) addr;
	new_region->free = true;
	new_region->size = region->size - size - sizeof(struct region);
	new_region->next = region->next;
	new_region->next->prev = new_region;
	new_region->prev = region;

	region->next = new_region;
}

void free_region(struct region *region)
{
	region->free = true;

	if (free_regions_head == NULL) {
		free_regions_head = region;
		return;
	}

	struct region *curr = free_regions_head;

	while (curr < region) {
		if (curr->next == NULL)
			break;

		curr = curr->next;
	}

	if (curr < region) { // region has the highest mem addr of the list
		region->prev = curr;
		region->next = NULL;
	} else {
		region->prev = curr->prev;
		region->next = curr;
	}

	coalesce_region(region);
}

void coalesce_region(struct region *region)
{
	if (region->prev != NULL) {
		char *addr = ((char *) (region->prev + 1)) + region->prev->size;
		if (addr == (char *) region) { // they are contiguous
			region->prev->next = region->next;
			region->prev->size += sizeof(struct region) + region->size;
			region = region->prev;
			if (region->next != NULL)
				region->next->prev = region;
		}
	}
	if (region->next != NULL) {
		char *addr = ((char *) (region + 1)) + region->size;
		if (addr == (char *) region->next) { // they are contiguous
			region->next = region->next->next;
			region->size += sizeof(struct region) + region->next->size;
			if (region->next != NULL)
				region->next->prev = region;
		}
	}
}
