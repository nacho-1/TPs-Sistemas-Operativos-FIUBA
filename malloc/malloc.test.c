#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
	int total_size_requested = 1000;
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
fail_if_requests_negative_amount(void)
{
	struct malloc_stats stats;

	size_t neg_size = -1;

	char *var = malloc(neg_size);
	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - negative] should return NULL pointer", var == NULL);
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

	char *var = malloc(SMALL_BLOCK_SIZE + 1);  // TODO: AJUSTAR A MAS BLOQUES
	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - limit] should return NULL pointer", var == NULL);
	free(var);
}

static void
fail_if_malloc_exceeds_block_free_space(void)
{
	struct malloc_stats stats;


	char *reg1 = malloc(SMALL_BLOCK_SIZE / 2);
	char *reg2 = malloc(SMALL_BLOCK_SIZE);

	ASSERT_TRUE("[MALLOC - limit] should return NULL pointer", reg2 == NULL);
	free(reg1);
	free(reg2);
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
	ASSERT_TRUE("[FREE - stats] amount of mallocs should be 10",
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
	run_test(fail_if_requests_negative_amount);
	run_test(fail_if_requested_zero);
	run_test(fail_if_malloc_exceeds_memory_limit);
	run_test(fail_if_malloc_exceeds_block_free_space);  // TODO - rm when multiple block allowed


	// TESTS FREE
	run_test(correct_amount_of_frees);
	run_test(correct_amount_of_frees_multiple_mallocs);
	run_test(correct_amount_of_frees_if_free_null_pointer);
	run_test(alloc_space_free);
	run_test(free_with_coalesce);
	run_test(fail_if_double_free);   // TODO - implement check
	run_test(fail_if_invalid_free);  // TODO - implement check

	return 0;
}
