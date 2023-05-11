#include <stdio.h>
#include <string.h>

#include "testlib.h"
#include "malloc.h"

//-------------------------------------------------
// MALLOC FREE
//-------------------------------------------------
static void
successful_malloc_returns_non_null_pointer(void)
{
	char *var = malloc(100);

	ASSERT_TRUE(
	        "[MALLOC - success] successful malloc returns non null pointer",
	        var != NULL);

	free(var);
}

static void
successful_malloc_split(void)
{
	char *reg1 = malloc(100);
	char *reg2 = malloc(100);

	ASSERT_TRUE("[MALLOC - success] successful split block", reg2 != NULL);

	free(reg1);
	free(reg2);
}

static void
correct_copied_value(void)
{
	char *test_string = "FISOP malloc is working!";

	char *var = malloc(100);

	strcpy(var, test_string);

	ASSERT_TRUE("[MALLOC - copied value] allocated memory should contain "
	            "the copied value",
	            strcmp(var, test_string) == 0);

	free(var);
}

static void
correct_amount_of_mallocs(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - stats] amount of mallocs should be one",
	            stats.mallocs == 1);
}

static void
correct_amount_of_mallocs_multiple_mallocs(void)
{
	struct malloc_stats stats;
	int malloc_requests = 10;
	char *var;

	for (int i = 0; i < malloc_requests; i++) {
		var = malloc(100);
		free(var);
	}

	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - stats] amount of mallocs should be 10",
	            stats.mallocs == 10);
}


static void
correct_amount_of_requested_memory(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - stats] amount of requested memory should be 100",
	            stats.requested_memory == 100);
}


static void
correct_amount_of_requested_memory_multiple_mallocs(void)
{
	struct malloc_stats stats;
	int malloc_requests = 10;
	char *var;

	for (int i = 0; i < malloc_requests; i++) {
		var = malloc(100);
		free(var);
	}

	get_stats(&stats);

	ASSERT_TRUE(
	        "[MALLOC - stats] amount of requested memory should be 1000",
	        stats.requested_memory == 1000);
}


static void
correct_amount_of_requested_memory_if_requested_less_than_min(void)
{
	struct malloc_stats stats;

	char *var = malloc(1);

	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - stats] amount of requested memory should be min "
	            "size",
	            stats.requested_memory == MIN_SIZE);

	free(var);
}

static void
fail_if_requested_zero(void)
{
	struct malloc_stats stats;

	char *var = malloc(0);
	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - requested 0] should return NULL pointer",
	            var == NULL);
	free(var);
}


static void
fail_if_malloc_exceeds_memory_limit(void)
{
	struct malloc_stats stats;

	char *var = malloc(LARGE_BLOCK_SIZE);
	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - limit] should return NULL pointer", var == NULL);
	free(var);
}

static void
malloc_small_block_size(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - small] amount of requested memory should be "
	            "less than medium block size",
	            stats.requested_memory <= SMALL_BLOCK_SIZE);
	ASSERT_TRUE("[MALLOC - small] amount of blocks should be 1",
	            stats.blocks == 1);

	free(var);
}

static void
malloc_medium_block_size(void)
{
	struct malloc_stats stats;

	char *var = malloc(SMALL_BLOCK_SIZE + 1);

	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - medium] amount of requested memory should be "
	            "greater than small block size",
	            stats.requested_memory >= SMALL_BLOCK_SIZE + 1);
	ASSERT_TRUE("[MALLOC - medium] amount of blocks should be 1",
	            stats.blocks == 1);

	free(var);
}

static void
malloc_large_block_size(void)
{
	struct malloc_stats stats;

	char *var = malloc(MEDIUM_BLOCK_SIZE + 1);

	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - large] amount of requested memory should be "
	            "greater than medium block size",
	            stats.requested_memory >= MEDIUM_BLOCK_SIZE);
	ASSERT_TRUE("[MALLOC - large] amount of blocks should be 1",
	            stats.blocks == 1);

	free(var);
}

static void
malloc_multiple_blocks(void)
{
	struct malloc_stats stats;

	char *var = malloc(16000);
	char *var2 = malloc(16000);

	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - multiple blocks] amount of blocks should be 2",
	            stats.blocks == 2);

	free(var);
	free(var2);
}

static void
malloc_only_one_block_when_enough_memory(void)
{
	struct malloc_stats stats;

	char *var = malloc(10);
	char *var2 = malloc(15);

	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - one block] amount of blocks should be 1",
	            stats.blocks == 1);

	free(var);
	free(var2);
}

static void
malloc_max_blocks_limit(void)
{
	struct malloc_stats stats;
	char *matrix[50];
	for (int i = 0; i < 50; i++) {
		matrix[i] = malloc(LARGE_BLOCK_SIZE * 2 / 3);
	}

	get_stats(&stats);
	ASSERT_TRUE(
	        "[MALLOC - max blocks limit] max amount of blocks should be 32",
	        stats.blocks == 32);
	for (int i = 0; i < 50; i++) {
		free(matrix[i]);
	}
}

static void
malloc_min_request_size(void)
{
	struct malloc_stats stats;
	char *var = malloc(10);
	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - min request size] requesting less than 64 "
	            "should allocate 64",
	            stats.requested_memory == 64 && stats.blocks == 1);
	free(var);
}

static void
malloc_of_multiple_vars_that_create_two_blocks(void)
{
	char *small_var = malloc(SMALL_BLOCK_SIZE / 3);
	char *small_var2 = malloc(SMALL_BLOCK_SIZE / 3);
	char *medium_var = malloc(SMALL_BLOCK_SIZE + 1);

	struct malloc_stats stats;
	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - splitting] 3 allocs should create two blocks "
	            "because of splitting",
	            stats.blocks == 2);
}

static void
malloc_multiple_blocks_splitting(void)
{
	char *small_var = malloc(3000);
	char *medium_var = malloc(SMALL_BLOCK_SIZE * 2);
	char *large_var = malloc(MEDIUM_BLOCK_SIZE * 3);

	char *small_var2 = malloc(3000);
	char *medium_var2 = malloc(SMALL_BLOCK_SIZE * 2);
	char *large_var2 = malloc(MEDIUM_BLOCK_SIZE * 3);

	struct malloc_stats stats;
	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - splitting] allocs that produces multiple blocks "
	            "types should reuse this blocks",
	            stats.blocks == 3);

	free(small_var);
	free(small_var2);
	free(medium_var);
	free(medium_var2);
	free(large_var);
	free(large_var2);
}

//-------------------------------------------------
// TESTS FREE
//-------------------------------------------------
static void
correct_amount_of_frees(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);

	ASSERT_TRUE("[FREE - stats] amount of frees should be one",
	            stats.frees == 1);
}

static void
correct_amount_of_frees_multiple_mallocs(void)
{
	struct malloc_stats stats;
	int expected_free_amount = 10;
	char *var;

	for (int i = 0; i < expected_free_amount; i++) {
		var = malloc(100);
		free(var);
	}

	get_stats(&stats);
	ASSERT_TRUE("[FREE - stats] amount of frees should be 10",
	            stats.frees == 10);
}

static void
correct_amount_of_frees_if_free_null_pointer(void)
{
	struct malloc_stats stats;

	char *var = NULL;

	free(var);

	get_stats(&stats);

	ASSERT_TRUE("[FREE - null] amount of frees should be 0", stats.frees == 0);
}

static void
alloc_space_free(void)
{
	char *reg1 = malloc(SMALL_BLOCK_SIZE * 2 / 3);
	free(reg1);
	char *reg2 = malloc(SMALL_BLOCK_SIZE * 2 / 3);

	ASSERT_TRUE("[FREE - success] allow to alloc free space", reg2 != NULL);

	free(reg2);
}


static void
free_with_coalesce(void)
{
	char *reg1 = malloc(SMALL_BLOCK_SIZE * 2 / 7);
	char *reg2 = malloc(SMALL_BLOCK_SIZE * 2 / 7);
	char *reg3 = malloc(SMALL_BLOCK_SIZE * 2 / 7);
	free(reg2);
	free(reg3);
	char *reg4 = malloc(SMALL_BLOCK_SIZE * 4 / 7);

	ASSERT_TRUE("[FREE - coalesce] alloc space after coalescing",
	            reg4 != NULL);

	free(reg1);
	free(reg4);
}

static void
fail_if_double_free(void)
{
	char *var = malloc(100);

	// Assert before free just to print test if failing until fix in free()
	ASSERT_TRUE("[FREE - double] fail if double free requested", 0 == 1);

	free(var);
	free(var);

	// TODO - immplement fail check

	// ASSERT_TRUE("[FREE - double] fail if double free requested",
	//             0==1);
}

static void
fail_if_invalid_free(void)
{
	char *str = "this is a non malloc requested space";

	// Assert before free just to print test if failing until fix in free()
	ASSERT_TRUE("[FREE - invalid] fail if invalid  free requested", 0 == 1);

	free(str);

	// TODO - immplement fail check

	// ASSERT_TRUE("[FREE - invalid] fail if invalid  free requested",
	//             0 == 1);
}

static void
free_one_region_block_should_be_unmapped(void)
{
	char *var = malloc(10);
	struct malloc_stats stats;
	free(var);
	get_stats(&stats);
	ASSERT_TRUE("[FREE - unmap block] a block with a single region being "
	            "freed should be unmapped",
	            stats.blocks == 0);
}

static void
free_multiple_region_of_large_block_should_unmap_it(void)
{
	// Ask for a large block.
	char *var = malloc(MEDIUM_BLOCK_SIZE + 1);
	char *vars[50000];
	for (int i = 0; i < 50000; i++) {
		vars[i] = malloc(64);
	}
	for (int i = 0; i < 50000; i++) {
		free(vars[i]);
	}
	free(var);
	struct malloc_stats stats;
	get_stats(&stats);
	ASSERT_TRUE("[FREE - performance] freeing lots of regions of a large "
	            "block should unmap the block",
	            stats.blocks == 0);
}

static void
free_allocs_of_different_size_should_unmap_blocks(void)
{
	char *small_var = malloc(3000);
	char *medium_var = malloc(SMALL_BLOCK_SIZE * 2);
	char *large_var = malloc(MEDIUM_BLOCK_SIZE * 2);

	free(small_var);
	free(medium_var);
	free(large_var);
	struct malloc_stats stats;
	get_stats(&stats);
	ASSERT_TRUE("[FREE - unmap blocks] freeing all vars of different size "
	            "should unmap blocks",
	            stats.blocks == 0);
}

static void
free_two_allocs_of_different_regions_size_should_unmap_blocks(void)
{
	char *small_var = malloc(1500);
	char *medium_var = malloc(SMALL_BLOCK_SIZE * 2);
	char *large_var = malloc(MEDIUM_BLOCK_SIZE * 2);

	char *small_var2 = malloc(1500);
	char *medium_var2 = malloc(SMALL_BLOCK_SIZE * 3);
	char *large_var2 = malloc(MEDIUM_BLOCK_SIZE * 4);

	free(small_var);
	free(small_var2);
	free(medium_var);
	free(medium_var2);
	free(large_var);
	free(large_var2);

	struct malloc_stats stats;
	get_stats(&stats);
	ASSERT_TRUE("[FREE - unmap blocks] freeing multiple allocs of "
	            "different size should unmap blocks of diff type",
	            stats.blocks == 0);
}

static void
free_all_regions_of_multiple_blocks_of_diff_type_should_unmap_all_blocks(void)
{
	char *small_var = malloc(1500);
	char *small_var2 = malloc(1500);

	char *medium_var = malloc(MEDIUM_BLOCK_SIZE * 2 / 3);
	char *medium_var2 = malloc(SMALL_BLOCK_SIZE * 3);

	char *large_var = malloc(LARGE_BLOCK_SIZE * 2 / 3);
	char *large_var2 = malloc(MEDIUM_BLOCK_SIZE * 4);

	free(small_var);
	free(small_var2);
	free(medium_var);
	free(medium_var2);
	free(large_var);
	free(large_var2);

	struct malloc_stats stats;
	get_stats(&stats);
	ASSERT_TRUE("[FREE - unmap blocks] freeing all mallocs should unmap "
	            "all blocks of diff type",
	            stats.blocks == 0);
}

static void
free_all_regions_of_multiple_blocks_should_unmap_all_blocks(void)
{
	char *small[216];
	char *medium[216];
	char *large[216];
	// small blocks
	for (int i = 0; i < 216; i++) {
		small[i] = malloc(64);
		medium[i] = malloc(SMALL_BLOCK_SIZE + 1);
		large[i] = malloc(MEDIUM_BLOCK_SIZE + 1);
	}

	for (int i = 0; i < 216; i++) {
		free(small[i]);
		free(medium[i]);
		free(large[i]);
	}
	struct malloc_stats stats;
	get_stats(&stats);
	ASSERT_TRUE("[FREE - unmap blocks] freeing all regions of diff size "
	            "from all "
	            "blocks of diff type should unmap all blocks",
	            stats.blocks == 0);
}

//-------------------------------------------------
// TESTS CALLOC
//-------------------------------------------------
static void
calloc_buffer_should_initialize_with_ceros(void)
{
	int buf_size = 5;
	int *buf = (int *) calloc(buf_size, sizeof(int));
	int success = 0;
	for (int i = 0; i < buf_size; i++) {
		if (buf[i] != 0) {
			success = 0;
			break;
		} else {
			success = 1;
		}
	}
	free(buf);
	ASSERT_TRUE("[CALLOC - basic] calloc for a buffer should initialize "
	            "all elements with 0",
	            success);
}

//-------------------------------------------------
// TESTS REALLOC
//-------------------------------------------------

static void
realloc_should_allow_writing_more_elements(void)
{
	char *string1 = "Hola ";
	char *string2 = "Mundo.";
	size_t len_str1 = strlen(string1);
	size_t len_str2 = strlen(string2);

	char *string = malloc(len_str1);
	strncat(string, string1, len_str1);

	char *new_string = (char *) realloc(string, len_str1 + len_str2);
	strncat(new_string, string2, len_str2);

	ASSERT_TRUE("[REALLOC - basic] concat a string, realloc and then "
	            "concat another string",
	            strcmp(new_string, "Hola Mundo.") == 0);
	free(new_string);
}

int
main(void)
{
	// TESTS MALLOC
	// TO DO - check for error flag in fail testcases
	run_test(successful_malloc_returns_non_null_pointer);
	run_test(successful_malloc_split);
	run_test(correct_copied_value);
	run_test(correct_amount_of_mallocs);
	run_test(correct_amount_of_mallocs_multiple_mallocs);
	run_test(correct_amount_of_requested_memory);
	run_test(correct_amount_of_requested_memory_multiple_mallocs);
	run_test(correct_amount_of_requested_memory_if_requested_less_than_min);
	run_test(fail_if_requested_zero);
	run_test(fail_if_malloc_exceeds_memory_limit);
	run_test(malloc_small_block_size);
	run_test(malloc_medium_block_size);
	run_test(malloc_large_block_size);
	run_test(malloc_multiple_blocks);
	run_test(malloc_only_one_block_when_enough_memory);
	run_test(malloc_max_blocks_limit);
	run_test(malloc_min_request_size);
	run_test(malloc_of_multiple_vars_that_create_two_blocks);
	run_test(malloc_multiple_blocks_splitting);

	// TESTS FREE
	run_test(correct_amount_of_frees);
	run_test(correct_amount_of_frees_multiple_mallocs);
	run_test(correct_amount_of_frees_if_free_null_pointer);
	run_test(alloc_space_free);
	run_test(free_with_coalesce);
	run_test(fail_if_double_free);   // TODO - implement check
	run_test(fail_if_invalid_free);  // TODO - implement check
	run_test(free_one_region_block_should_be_unmapped);
	// run_test(free_multiple_region_of_large_block_should_unmap_it);
	run_test(free_allocs_of_different_size_should_unmap_blocks);
	run_test(free_two_allocs_of_different_regions_size_should_unmap_blocks);
	run_test(free_all_regions_of_multiple_blocks_of_diff_type_should_unmap_all_blocks);
	run_test(free_all_regions_of_multiple_blocks_should_unmap_all_blocks);

	// TEST CALLOC
	run_test(calloc_buffer_should_initialize_with_ceros);

	// TEST REALLOC
	run_test(realloc_should_allow_writing_more_elements);

	return 0;
}
