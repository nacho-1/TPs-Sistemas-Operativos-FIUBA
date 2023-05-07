#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "testlib.h"
#include "malloc.h"

static void
successful_malloc_returns_non_null_pointer(void)
{
	char *var = malloc(100);

	ASSERT_TRUE(
	        "[MALLOC - success]successful malloc returns non null pointer",
	        var != NULL);

	free(var);
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
correct_amount_of_frees(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - stats] amount of frees should be one",
	            stats.frees == 1);
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
fail_if_malloc_exceeds_memory_limit(void)
{
	struct malloc_stats stats;
	size_t SMALL_BLOCK_SIZE = 16384;

	char *var = malloc(SMALL_BLOCK_SIZE + 1);  // TODO: AJUSTAR A MAS BLOQUES
	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - limit] should return NULL pointer", var == NULL);
	free(var);
}

static void
fail_if_requests_negative_amount(void)
{
	struct malloc_stats stats;

	char *var = malloc(-1);
	get_stats(&stats);

	ASSERT_TRUE("[MALLOC - negative] should return NULL pointer", var == NULL);
	free(var);
}

static void
alloc_min_size_if_requested_less(void)
{
	struct malloc_stats stats;

	char *var = malloc(1);

	get_stats(&stats);
	ASSERT_TRUE("[MALLOC - min] amount of requested memory should be 64 "
	            "(min size)",
	            var == NULL);

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

int
main(void)
{
	// TESTS MALLOC
	run_test(successful_malloc_returns_non_null_pointer);
	run_test(correct_copied_value);
	run_test(correct_amount_of_mallocs);
	run_test(correct_amount_of_frees);
	run_test(correct_amount_of_requested_memory);
	run_test(fail_if_malloc_exceeds_memory_limit);
	run_test(fail_if_requests_negative_amount);
	run_test(fail_if_requested_zero);
	run_test(alloc_min_size_if_requested_less);

	return 0;
}
