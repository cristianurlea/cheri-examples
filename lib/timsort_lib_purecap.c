#include "timsort_lib_purecap.h"
#include "../include/common.h"
#include <assert.h>
#include <cheri/cheric.h>
#include <cheriintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// this is needed to fall-black when the array length can not be represented exactly as CHERI bounds
void timSort_classic(int arr[], size_t length);

/**
 * Strict capability pointer equality check that includes the `offset` and `bounds` fields.
 * @param a first capability pointer to be checked.
 * @param b second capability pointer to be checked
 * @returns True if pointers have equal: bases, address offsets and lengths.
 */
bool offsetBoundsEq(int *a, int *b)
{
	return (a == b && cheri_getlen(a) == cheri_getlen(b) &&
			cheri_getoffset(a) == cheri_getoffset(b));
}

/**
 * Attempts to encode exact upper and lower bounds through CHERI fields.
 * @param arr is the source capability. If successful `offsetBoundsEq(arr, <return value> )` will be
 * false.
 * @param lowerBound is encoded as CHERI `offset` ( if successful )
 * @param arrayLength is encoded as CHERI `length` ( if successful )
 * @returns new capbility pointer with exact `offset` and `length` on success, `arr` otherwise.
 */
int *local_setBounds(int *arr, const size_t lowerBound, const size_t arrayLength)
{
	int *upperSet = cheri_bounds_set(arr, arrayLength * sizeof(int));
	int *lowerSet = cheri_offset_set(upperSet, lowerBound * sizeof(int));

	if (lowerBound == cheri_getoffset(lowerSet) / sizeof(int) &&
		arrayLength == cheri_getlen(lowerSet) / sizeof(int))
	{
		return lowerSet;
	}

	return arr;
}

/**
 * Sorts the input array, in place, using `insertion sort`.
 * @param arr array to sort
 * Capability implicit paramters:
 * - uses offset to indicate lower bound (unit: bytes)
 * - uses length of memory allocation chunk as upper bound (unit: bytes)
 * WARNING: the array length (encoded as bounds) is not always exactly representable
 */
void insertionSort_unsafe(int *arr)
{
	size_t lowerBound = cheri_getoffset(arr) / sizeof(int);
	size_t arrayLength = cheri_getlen(arr) / sizeof(int);

	// reset offset otherwise arr[x] is actually arr[x+offset]
	arr = cheri_offset_set(arr, 0);

	int ix, ixValue, ixP;
	for (ix = lowerBound + 1; ix < arrayLength; ix++)
	{
		ixValue = arr[ix];
		ixP = ix - 1;

		while (ixP >= 0 && arr[ixP] > ixValue)
		{
			arr[ixP + 1] = arr[ixP];
			ixP = ixP - 1;
		}
		arr[ixP + 1] = ixValue;
	}
}

/**
 * Merges two runs of an array.
 * @param arr super-array to merge
 * Capability implicit paramters:
 * - uses offset to indicate the end of the first leg to merge (unit: bytes)
 * - uses length of memory allocation as end of sencond leg to merge (unit: bytes)
 * WARNING: the array length (encoded as bounds) is not always exactly representable
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
 * This is function is not exported via `timsort_lib_purecap.h` and should only
 * be used through `timSort_purecap()`
 */
void timSort_unsafe(int *arr)
{
	size_t length = cheri_getlen(arr) / sizeof(int);

	// insertion sort on `RUN_LENGTH` segments
	for (size_t ix = 0; ix < length; ix += RUN_LENGTH)
	{
		size_t max_ix = min((ix + RUN_LENGTH) + 1, length);
		int *maybeBounded = local_setBounds(arr, ix, max_ix);

		if (offsetBoundsEq(arr, maybeBounded))
		{
			insertionSort(arr, ix, max_ix);
		}
		else
		{
			insertionSort_unsafe(maybeBounded);
		}
	}
	// Merge window doubles every iteration
	for (size_t size = RUN_LENGTH; size < length; size *= 2)
	{
		// Merge
		for (size_t left = 0; left + size < length - 1; left += 2 * size)
		{
			size_t mid = left + size;
			size_t right = min((left + 2 * size), (length - 1));

			int *maybeBounded = local_setBounds(arr, mid, right);

			if (offsetBoundsEq(arr, maybeBounded))
			{
				merge(arr, left, mid, right);
			}
			else
			{
				merge_unsafe(maybeBounded);
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
	if (length <= 1)
	{
		return;
	}

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