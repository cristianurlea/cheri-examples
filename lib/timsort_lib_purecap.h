#include "timsort_lib.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void insertionSort_unsafe(int *arr);
void merge_unsafe(int *arr);
void timSort_purecap(int arr[], size_t length);
