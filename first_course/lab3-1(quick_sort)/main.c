#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>


void swap(int* array, int a, int b) {
	int temp = array[a];
	array[a] = array[b];
	array[b] = temp;
}


void partition(int* array, int first, int last, int* left, int* right) {
	int pivot = array[(first + last) / 2];
	int i = first, j = first, k = last + 1;
	while (j < k) {
		if (array[j] < pivot)
		{
			swap(array, i, j);
			i++;
			j++;
		}
		else if (array[j] > pivot) {
			k--;
			swap(array, j, k);
		}
		else
			j++;
	}
	*left = i;
	*right = j;
}


void QuickSort(int* array, int first, int last) {
	int left = 0, right = 0;
	while (first < last) {
		partition(array, first, last, &left, &right);
		if (left - 1 - first < last - right) {
			QuickSort(array, first, left - 1);
			first = right;
			left = right;
		}
		else {
			QuickSort(array, right, last);
			last = left - 1;
		}
	}
}


void print(int* arr, int n) {
	for (int i = 0; i < n; i++)
		printf("%d ", arr[i]);
}


int main(void) {
	int n;
	if (scanf("%d", &n) != 1)
		return 0;
	int* arr = malloc(sizeof(int) * n);
	for (int i = 0; i < n; i++) {
		if (scanf("%d", &arr[i]) != 1)
			return 0;
	}
	QuickSort(arr, 0, n - 1);
	print(arr, n);
	free(arr);
	return 0;
}
