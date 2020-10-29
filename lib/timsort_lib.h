#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef RUN_LENGTH
#define RUN_LENGTH 64
#endif

size_t min(size_t a, size_t b);

bool isSorted(int arr[], size_t length);
void insertionSort(int arr[], size_t lowerBound, size_t upperBound);
void merge(int arr[], size_t lowerBound, size_t midPoint, size_t upperBound);
void timSort_classic(int arr[], size_t arr_length);
