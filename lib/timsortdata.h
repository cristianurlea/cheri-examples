#include <cheriintrin.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int MAX_ARRAY_SZ = 2048;

int *random_chunk(size_t arr_length)
{

	srand(time(NULL));

	int *arr = malloc(arr_length * sizeof(int));
	for (size_t ix = 0; ix < arr_length; ix++)
	{
		arr[ix] = rand();
	}

	return arr;
}

int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

bool arrEq(int arr_a[], int arr_b[], size_t lowerBound, size_t upperBound)
{
	if (lowerBound == upperBound || 0 == upperBound)
	{
		return true;
	}

	for (size_t ix = lowerBound; ix <= upperBound; ix++)
	{
		if (arr_a[ix] != arr_b[ix])
		{
			printf("neq %d %d %zu \n", arr_a[ix], arr_b[ix], ix);

			for (size_t ix = lowerBound; ix <= upperBound; ix++)
			{
				printf(" %d . %d ; ", arr_a[ix], arr_b[ix]);
			}

			printf("\n");
			return false;
		}
	}
	return true;
}
