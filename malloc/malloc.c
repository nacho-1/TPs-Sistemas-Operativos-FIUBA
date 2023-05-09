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

struct region {
	bool free;
	size_t size;
	struct region *next;
	struct region *prev;
};

struct block {
	size_t size;
	struct region *first;
	struct block *next;
	void *ptr;
};

size_t blocks_mapped = 0;
struct block *small_blocks = NULL;
struct block *medium_blocks = NULL;
struct block *large_blocks = NULL;

int amount_of_mallocs = 0;
int amount_of_frees = 0;
int requested_memory = 0;

void split(struct region *region, size_t size);

/*
 * Merge region with its next region into one.
 */
struct region *merge(struct region *region);

/*
 * Coalesce region with its adjacent ones.
 */
struct region *coalesce(struct region *region);

static struct region *
find_free_region(size_t size)
{
	struct block *block_ptr = small_blocks;
	struct region *region_ptr = block_ptr->first;

#ifdef FIRST_FIT
	//search in small blocks list
	while (block_ptr != NULL) {
		while (region_ptr != NULL) {
			if (region_ptr->free && region_ptr->size >= size)
				break;
			else
				region_ptr = region_ptr->next;
		}
		if (region_ptr != NULL) break;
		region_ptr = block_ptr->next;
	}

	if (region_ptr != NULL) {
		//search in medium blocks list
		block_ptr = medium_blocks;
		region_ptr = block_ptr->first;
		while (block_ptr != NULL) {
			while (region_ptr != NULL) {
				if (region_ptr->free && region_ptr->size >= size)
					break;
				else
					region_ptr = region_ptr->next;
			}
			if (region_ptr != NULL) break;
			region_ptr = block_ptr->next;
		}
	}

	if (region_ptr != NULL) {
		//search in large blocks list
		block_ptr = large_blocks;
		region_ptr = block_ptr->first;
		while (block_ptr != NULL) {
			while (region_ptr != NULL) {
				if (region_ptr->free && region_ptr->size >= size)
					break;
				else
					region_ptr = region_ptr->next;
			}
			if (region_ptr != NULL) break;
			region_ptr = block_ptr->next;
		}
	}
#endif

#ifdef BEST_FIT
	// Your code here for "best fit"
#endif

	return region_ptr;
}

static struct block *
grow_heap(size_t size)
{
	if (blocks_mapped == MAX_BLOCKS) return NULL;
	void *mapping = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mapping == MAP_FAILED) return NULL;

	struct block *new_block = mapping;
	new_block->ptr = (void *) mapping;
	new_block->size = size;
	new_block->first = mapping + sizeof(*new_block);
	new_block->next = NULL;

	if (size == SMALL_BLOCK_SIZE) {
		if (!small_blocks) small_blocks = new_block;
		else {
			new_block->next = small_blocks;
			small_blocks = new_block;
		}
	}
	else if (size == MEDIUM_BLOCK_SIZE) {
		if (!medium_blocks)
			medium_blocks = new_block;
		else {
			new_block->next = medium_blocks;
			medium_blocks = new_block;
		}
	}
	else {
		if (!large_blocks)
			large_blocks = new_block;
		else {
			new_block->next = large_blocks;
			large_blocks = new_block;
		}
	}

	blocks_mapped++;
	return new_block;
}

static size_t size_that_fits(size_t size) {
	if (size > LARGE_BLOCK_SIZE) return 0; //no tengo en cuenta la metadata creo
	if (size < SMALL_BLOCK_SIZE) return SMALL_BLOCK_SIZE;
	if (size < MEDIUM_BLOCK_SIZE) return MEDIUM_BLOCK_SIZE;
	else return LARGE_BLOCK_SIZE;
}

/// Public API of malloc library ///

void *
malloc(size_t size)
{
	if ((int) size <= 0 || (int) size >= LARGE_BLOCK_SIZE)
		return NULL;

	// aligns to multiple of 4 bytes
	size = ALIGN4(size);

	// minimun size of region is MIN_SIZE
	size = size > MIN_SIZE ? size : MIN_SIZE;

	if (size >= SMALL_BLOCK_SIZE - sizeof(struct region))
		return NULL;

	struct region *region = find_free_region(size);

	if (region == NULL) {
		size_t block_size = size_that_fits(size);
		region = grow_heap(block_size)->first;

		if (region == NULL) {
			return NULL;
		}
	}


	if (region->size >= size + sizeof(struct region) + MIN_SIZE) {
		split(region, size);
	}


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
	assert(!curr->free);  // TODO - replace this for error set

	// TODO - check here if it is a region struct allocated by malloc

	curr->free = true;


	coalesce(curr);

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

void
split(struct region *region, size_t size)
{
	assert(region->size > size + sizeof(struct region));

	char *addr = ((char *) (region + 1)) + size;

	struct region *new_region = (struct region *) addr;

	new_region->free = true;
	new_region->size = region->size - size - sizeof(struct region);
	new_region->next = region->next;
	if (new_region->next != NULL) {
		new_region->next->prev = new_region;
	}
	new_region->prev = region;
	region->next = new_region;
}

struct region *
merge(struct region *region)
{
	if (region == NULL)
		return NULL;

	if (region->next == NULL)
		return region;

	region->size += sizeof(struct region) + region->next->size;
	region->next = region->next->next;
	if (region->next != NULL)
		region->next->prev = region;

	return region;
}

struct region *
coalesce(struct region *region)
{
	if (region->prev != NULL && region->prev->free)
		region = merge(region->prev);

	if (region->next != NULL && region->next->free)
		region = merge(region);

	return region;
}
