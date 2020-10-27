#include "timsort_lib_purecap.h"
#include "../include/common.h"
#include <assert.h>
#include <cheri/cheric.h>
#include <cheriintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// this is needed to fall-black when the array length can not be represented exactly as CHERI bounds
void timSort_classic(int arr[], size_t length);

/**
 * Sorts the input array, in place, using `insertion sort`.
 * @param arr array to sort
 * Capability implicit paramters:
 * - uses offset to indicate lower bound (unit: bytes)
 * - uses length of memory allocation chunk as upper bound (unit: bytes)
 * WARNING: the array length (encoded as bounds) is not always exactly representable
 * With -march=rv64imafdcxcheri -mabi=l64pc128d any value <= 1024 is exact
 */
void insertionSort_unsafe(int *arr)
{
	assert(cheri_is_valid(arr));
	size_t lowerBound = cheri_getoffset(arr) / sizeof(int);
	size_t upperBound = (cheri_getlen(arr) / sizeof(int)) - lowerBound;

	// reset offset otherwise arr[x] is actually arr[x+offset]
	arr = cheri_offset_set(arr, 0);

	for (size_t ix = lowerBound + 1; ix < upperBound; ix++)
	{
		int ix_value = arr[ix];
		size_t ixp = ix - 1;
		bool hitBottom = false;

		while ((ixp >= lowerBound) && (arr[ixp] > ix_value))
		{
			arr[ixp + 1] = arr[ixp];

			if (ixp > 0)
			{
				ixp--;
			}
			else
			{
				arr[ixp] = ix_value;
				hitBottom = true;
				break;
			}
		}

		if (!hitBottom)
		{
			arr[ixp + 1] = ix_value;
		}
	}
}

/**
 * Merges two runs of an array.
 * @param arr super-array to merge
 * Capability implicit paramters:
 * - uses offset to indicate the end of the first leg to merge (unit: bytes)
 * - uses length of memory allocation as end of sencond leg to merge (unit: bytes)
 * WARNING: the array length (encoded as bounds) is not always exactly representable
 * With -march=rv64imafdcxcheri -mabi=l64pc128d any value <= 1024 is exact
 */
void merge_unsafe(int *arr)
{

	// allocations
	size_t lengthFirstHalf = cheri_getoffset(arr) / sizeof(int);
	size_t lengthSecondHalf = (cheri_getlen(arr) / sizeof(int)) - lengthFirstHalf;

	// reset offset otherwise arr[x] is actually arr[x+offset]
	arr = cheri_offset_set(arr, 0);

	int firstHalf[lengthFirstHalf];
	int secondHalf[lengthSecondHalf];

	// copy to intermediate storage
	memcpy(firstHalf, &arr[0], lengthFirstHalf * sizeof(int));
	memcpy(secondHalf, &arr[lengthFirstHalf], lengthSecondHalf * sizeof(int));

	// merge intermediate back to output
	size_t ix_fst = 0;
	size_t ix_snd = 0;
	size_t ix_out = 0;
	while ((ix_fst < lengthFirstHalf) && (ix_snd < lengthSecondHalf))
	{
		if (firstHalf[ix_fst] <= secondHalf[ix_snd])
		{
			arr[ix_out++] = firstHalf[ix_fst++];
		}
		else
		{
			arr[ix_out++] = secondHalf[ix_snd++];
		}
	}

	// copy stragglers
	if (ix_fst < lengthFirstHalf)
	{
		size_t delta = lengthFirstHalf - ix_fst;
		memcpy(&arr[ix_out], &firstHalf[ix_fst], delta * sizeof(int));
		ix_out += delta;
		ix_fst += delta;
	}

	if (ix_snd < lengthSecondHalf)
	{
		size_t delta = lengthSecondHalf - ix_snd;
		memcpy(&arr[ix_out], &secondHalf[ix_snd], delta * sizeof(int));
		ix_out += delta;
		ix_snd += delta;
	}

	return;
}

/**
 * Timsort routine for an array of `int`.
 * @param arr Array to sort
 * Capability implicit paramters:
 * - uses length of memory allocation chunk: n = cheri_getlen(arr) / sizeof(int)
 * WARNING: the array length (encoded as bounds) is not always exactly representable
 * With -march=rv64imafdcxcheri -mabi=l64pc128d any value <= 1024 is exact
 *
 * WARNING: This is function is not exported via `timsort_lib_purecap.h` and should only
 * be used through `timSort_purecap()`
 */
void timSort_unsafe(int *arr)
{
	size_t length = cheri_getlen(arr) / sizeof(int);

	// insertion sort on `RUN_LENGTH` segments
	for (size_t ix = 0; ix < length; ix += RUN_LENGTH)
	{
		size_t min_offset = min((ix + RUN_LENGTH), (length - 1));

		// The array length (encoded as bounds) is not always exactly representable.
		// With -march=rv64imafdcxcheri -mabi=l64pc128d any value <= 1024 is exact.
		// For lengths <=1024 we can use CHERI bounds field to carry length.
		if (RUN_LENGTH * sizeof(int) <= 1024)
		{
			int *arr_base_length_set = cheri_bounds_set(&arr[ix], (min_offset - ix) * sizeof(int));
			arr_base_length_set = cheri_offset_set(arr, ix * sizeof(int));
			insertionSort_unsafe(arr_base_length_set);
		}
		else
		{
			insertionSort(arr, ix, min_offset);
		}
	}

	// Merge window doubles every iteration
	for (size_t size = RUN_LENGTH; size < length; size *= 2)
	{
		// Merge
		for (size_t left = 0; left + size < length; left += 2 * size)
		{
			size_t mid = left + size;
			size_t right = min((left + 2 * size), (length - 1));

			int *arr_base_length_set = cheri_bounds_set(&arr[left], (right - left) * sizeof(int));
			arr_base_length_set = cheri_offset_set(arr_base_length_set, (mid - left) * sizeof(int));

			// as represented bounds may be inaccurate we check before dispatching
			const size_t represented_fstHalfLength = cheri_getoffset(arr) / sizeof(int);
			const size_t represented_sndHalfLength =
				(cheri_getlen(arr) / sizeof(int)) - represented_fstHalfLength;

			if (represented_fstHalfLength == (mid - left) &&
				represented_sndHalfLength == (right - mid))
			{
				merge_unsafe(arr_base_length_set);
			}
			else
			{
				merge(arr, left, mid, right);
			}
		}
	}
}

/**
 * Timsort routine for an array of `int`.
 * @param arr Array to sort
 * @param length The legth of `arr`
 */
void timSort_purecap(int arr[], size_t length)
{
	size_t bounds_length = cheri_getlen(arr) / sizeof(int);

	// as represented bounds may be inaccurate we check before dispatching
	if (length == bounds_length)
	{
		timSort_unsafe(arr);
	}
	else
	{
		timSort_classic(arr, length);
	}
}