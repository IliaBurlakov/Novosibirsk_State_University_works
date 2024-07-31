#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void swap(int* arr, int i, int j) {
	int temp = arr[i];
	arr[i] = arr[j];
	arr[j] = temp;
}


void siftdown(int* arr, int i, int upper) {
	while (1) {
		int left = i * 2 + 1;
		int right = i * 2 + 2;
		if (left < upper && right < upper) {
			if (arr[i] >= arr[left] && arr[i] >= arr[right])
				break;
			else if (arr[left] > arr[right]) {
				swap(arr, i, left);
				i = left;
			}
			else {
				swap(arr, i, right);
				i = right;
			}
		}
		else if (left < upper) {
			if (arr[left] > arr[i]) {
				swap(arr, i, left);
				i = left;
			}
			else break;
		}
		else break;
	}
}


void heapsort(int* arr, int n) {
	int j = (n - 2) / 2;
	for (; j > -1; j--)
		siftdown(arr, j, n);
	int end = n - 1;
	for (; end > 0; end--) {
		swap(arr, 0, end);
		siftdown(arr, 0, end);
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
	heapsort(arr, n);
	print(arr, n);
	free(arr);
	return 0;
}
