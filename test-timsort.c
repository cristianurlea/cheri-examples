#include "lib/timsort_lib.h"
#include "lib/timsortdata.h"
#include <assert.h>

void test_merge()
{
	int input_arr_control[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	int input_arr_mutate_a_input[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	int input_arr_mutate_a_expected[] = {6, 5, 4, 3, 2, 1, 10, 9, 8, 7};

	int input_arr_mutate_b_input[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	int input_arr_mutate_b_expected[] = {5, 4, 3, 2, 1, 10, 9, 8, 7, 6};

	merge(input_arr_mutate_a_input, 0, 4, 10);
	merge(input_arr_mutate_b_input, 0, 5, 10);

	assert(arrEq(input_arr_mutate_a_input, input_arr_mutate_a_expected, 0, 9));
	assert(arrEq(input_arr_mutate_b_input, input_arr_mutate_b_expected, 0, 9));

	return;
}

void test_isSorted()
{
	// positive cases
	int sorted_array_empty[] = {};
	int sorted_array_singleton[] = {42};

	assert(isSorted(sorted_array_empty, 0));
	assert(isSorted(sorted_array_singleton, 1));

	int sorted_array_small[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	assert(isSorted(sorted_array_small, 10));

	// negative cases
	sorted_array_small[5] = 42;
	assert(!isSorted(sorted_array_small, 10));
}

void test_timsort_classic()
{
	for (size_t sz = 2; sz <= MAX_ARRAY_SZ; sz++)
	{
		// place the chunk of data on the heap
		int *arr = random_chunk(sz);
		int *arr_cpy = malloc(sz * sizeof(int));

		assert(NULL != arr);
		assert(NULL != arr_cpy);

		memcpy(arr_cpy, arr, sz * sizeof(int));

		assert(arrEq(arr, arr_cpy, 0, sz - 1));

		// sort the data
		timSort_classic(arr, sz);

		// stable sort comparison
		qsort(arr_cpy, sz, sizeof(int), cmpfunc);

		// check that have done real work
		assert(isSorted(arr, sz));
		assert(arrEq(arr, arr_cpy, 0, sz - 1));

		// clean up
		free(arr);
		free(arr_cpy);
	}
}

/**
 * Test harness for `lib/timsort_lib.c`.
 * @return EXIT_SUCCESS when all tests pass. Assertion failure otherwise.
 */
int main(int argc, char *argv[])
{
	test_isSorted();

	test_merge();

	test_timsort_classic();

	return EXIT_SUCCESS;
}
