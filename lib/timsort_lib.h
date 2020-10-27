#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef RUN_LENGTH
// WARNING: this must be a value that is *exactly* representable as a CHERI bounds field
// With -march=rv64imafdcxcheri -mabi=l64pc128d any value <= 1024 is exact
#define RUN_LENGTH 32
#endif

size_t min(size_t a, size_t b);

bool isSorted(int arr[], size_t length);
void insertionSort(int arr[], size_t lowerBound, size_t upperBound);
void merge(int arr[], size_t lowerBound, size_t midPoint, size_t upperBound);
void timSort_classic(int arr[], size_t arr_length);
