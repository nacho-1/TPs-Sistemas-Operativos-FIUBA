#include "malloc.h"

int amount_of_mallocs = 0;
int amount_of_frees = 0;
int requested_memory = 0;


size_t blocks_mapped = 0;
struct block *small_blocks = NULL;
struct block *medium_blocks = NULL;
struct block *large_blocks = NULL;


static struct region *
find_free_region(size_t size)
{
	if (size > LARGE_BLOCK_SIZE - sizeof(struct block) - sizeof(struct region))
		return NULL;

	struct region *region = NULL;

	// search in small blocks list
	struct block *curr_block = small_blocks;
	while (curr_block != NULL) {
		region = find_in_block((struct region *) BLOCK2REGION(curr_block),
		                       size);
		if (region != NULL)
			return region;
		else
			curr_block = curr_block->next;
	}

	// search in medium blocks list
	curr_block = medium_blocks;
	while (curr_block != NULL) {
		region = find_in_block((struct region *) BLOCK2REGION(curr_block),
		                       size);
		if (region != NULL)
			return region;
		else
			curr_block = curr_block->next;
	}

	curr_block = large_blocks;
	while (curr_block != NULL) {
		region = find_in_block((struct region *) BLOCK2REGION(curr_block),
		                       size);
		if (region != NULL)
			return region;
		else
			curr_block = curr_block->next;
	}

	return NULL;  // couldn't find any free region for size in any block
}

/*
 * Find free region in block that fits size,
 * where first is the first region of the block.
 * Return NULL if there's none.
 */
static struct region *
find_in_block(struct region *first, size_t size)
{
	struct region *region = first;
#ifdef FIRST_FIT
	while (region != NULL) {
		if (region->free && region->size >= size)
			break;
		else
			region = region->next;
	}
#endif
#ifdef BEST_FIT
	struct region *best_region = NULL;
	while (region != NULL) {
		if (region->free && region->size >= size) {
			if (best_region == NULL) {
				best_region = region;
			} else if (region->size < best_region->size) {
				best_region = region;
			}
			region = region->next;
		} else
			region = region->next;
	}
	region = best_region;
#endif
	return region;
}

/*
 * Map a new block that fits size.
 * Return the first region of the block.
 */
static struct region *
grow_heap(size_t size)
{
	if (blocks_mapped == MAX_BLOCKS)
		return NULL;

	size = size_that_fits(size);
	if (size == 0)  // can't fit
		return NULL;

	void *mapping = mmap(
	        NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mapping == MAP_FAILED)
		return NULL;

	struct block *new_block = (struct block *) mapping;
	new_block->prev = NULL;

	if (size == SMALL_BLOCK_SIZE) {
		new_block->next = small_blocks;
		small_blocks = new_block;
	} else if (size == MEDIUM_BLOCK_SIZE) {
		new_block->next = medium_blocks;
		medium_blocks = new_block;
	} else {
		new_block->next = large_blocks;
		large_blocks = new_block;
	}
	if (new_block->next != NULL)
		new_block->next->prev = new_block;
	blocks_mapped++;

	// Initialize the first region of the block.
	struct region *region = (struct region *) BLOCK2REGION(new_block);
	region->magic_number = MAGIC_NUMBER;
	region->free = true;
	region->size = size - sizeof(struct block) - sizeof(struct region);
	region->next = NULL;
	region->prev = NULL;
	return region;
}

static size_t
size_that_fits(size_t size)
{
	size_t header_size = sizeof(struct block) + sizeof(struct region);
	if (size <= SMALL_BLOCK_SIZE - header_size)
		return SMALL_BLOCK_SIZE;
	else if (size <= MEDIUM_BLOCK_SIZE - header_size)
		return MEDIUM_BLOCK_SIZE;
	else if (size <= LARGE_BLOCK_SIZE - header_size)
		return LARGE_BLOCK_SIZE;
	else
		return 0;  // too big
}

/// Public API of malloc library ///

void *
malloc(size_t size)
{
	if (size == 0)
		return NULL;

	// aligns to multiple of 4 bytes
	size = ALIGN4(size);

	// minimun size of region is MIN_SIZE
	size = size > MIN_SIZE ? size : MIN_SIZE;

	struct region *region = find_free_region(size);

	if (region == NULL) {  // Couldn't find in any block
		region = grow_heap(size);

		if (region == NULL) {  // Couldn't create new block
			errno = ENOMEM;
			return NULL;
		}
	}

	split(region, size);

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

	if (curr->magic_number != MAGIC_NUMBER)
		return;

	if (curr->free)  // stdlib no define ningun error
		return;

	// stdlib define que free de un puntero no devuelto por la libreria (no
	// fue devuelto por un malloc, realloc, etc) causa undefined-behaviour.
	// Por ahora nuestro free es igual.

	curr->free = true;

	curr = coalesce(curr);

	// updates statistics
	amount_of_frees++;

	if (curr->next == NULL && curr->prev == NULL) {  // block is empty
		size_t block_size = curr->size + sizeof(struct block) +
		                    sizeof(struct region);
		unmap((struct block *) REGION2BLOCK(curr), block_size);
	}
}

void *
calloc(size_t nmemb, size_t size)
{
	void *ptr = malloc(nmemb * size);

	if (ptr == NULL) {
		return NULL;
	}

	memset(ptr, 0, nmemb * size);
	return ptr;
}

void *
realloc(void *ptr, size_t size)
{
	if (size == 0) {
		free(ptr);
		return ptr;
	}

	if (ptr == NULL)
		return malloc(size);

	struct region *curr = PTR2REGION(ptr);
	struct region *new_reg;
	void *new_ptr;

	if (size < curr->size) {
		split(curr, size);
		return ptr;
	} else {
		size_t prev_next_free_space = 0;
		if (curr->prev->free) {
			if (curr->size + curr->prev->size > size) {
				new_reg = merge(curr->prev);
				new_ptr = REGION2PTR(new_reg);
				memcpy(new_ptr, ptr, curr->size);
				return new_ptr;
			}
			prev_next_free_space += curr->prev->size;
		}
		if (curr->next->free) {
			if (curr->size + curr->prev->size > size) {
				new_reg = merge(curr);
				return REGION2PTR(new_reg);
			}
			prev_next_free_space += curr->next->size;
		}

		if (curr->size + prev_next_free_space >= size) {
			new_reg = merge(curr->prev);
			new_reg = merge(new_reg);
			new_ptr = REGION2PTR(new_reg);
			memcpy(new_ptr, ptr, curr->size);
			return new_ptr;
		}

		new_ptr = malloc(size);
		if (new_ptr == NULL)
			return NULL;

		memcpy(new_ptr, ptr, curr->size);
		free(ptr);
		return new_ptr;
	}
}

void
get_stats(struct malloc_stats *stats)
{
	stats->mallocs = amount_of_mallocs;
	stats->frees = amount_of_frees;
	stats->requested_memory = requested_memory;
	stats->blocks = blocks_mapped;
}

void
split(struct region *region, size_t size)
{
	if (region == NULL)
		return;

	if (region->size < size + sizeof(struct region) + MIN_SIZE)
		return;

	char *addr = ((char *) (region + 1)) + size;

	struct region *new_region = (struct region *) addr;

	new_region->magic_number = MAGIC_NUMBER;
	new_region->free = true;
	new_region->size = region->size - size - sizeof(struct region);
	new_region->next = region->next;
	if (new_region->next != NULL) {
		new_region->next->prev = new_region;
	}
	new_region->prev = region;
	region->size = size;
	region->next = new_region;
}

/*
 * Merge region with its next region into one.
 */
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


/*
 * Coalesce region with its adjacent ones.
 * Return a pointer to the region that contains
 * the parameter region.
 */
struct region *
coalesce(struct region *region)
{
	if (region->prev != NULL && region->prev->free)
		region = merge(region->prev);

	if (region->next != NULL && region->next->free)
		region = merge(region);

	return region;
}

void
unmap(struct block *block, size_t size)
{
	if (block->next != NULL)
		block->next->prev = block->prev;

	if (block->prev != NULL) {
		block->prev->next = block->next;
	} else {  // block list points to me
		if (size == SMALL_BLOCK_SIZE)
			small_blocks = block->next;
		else if (size == MEDIUM_BLOCK_SIZE)
			medium_blocks = block->next;
		else
			large_blocks = block->next;
	}

	if (munmap(block, size) == 0)
		blocks_mapped--;
}
