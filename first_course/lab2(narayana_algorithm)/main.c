#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


void bad_input() {
	printf("bad input");
	exit(0);
}


void reverse(char* stringIn, unsigned int index1, unsigned int len) {
	for (unsigned int i = (index1 + 1), j = 0; j < ((len - index1 - 1) / 2); i++, j++) {
		stringIn[j] = stringIn[i - index1 - 1];
		char temp = stringIn[i];
		stringIn[i] = stringIn[len - 1 - j];
		stringIn[len - 1 - j] = temp;
	}
}


int permutation(char* stringIn, unsigned int len) {
	int index1 = 0, index2 = 0, i;
	char c;
	for (i = len - 2; i >= 0; i--) {
		if (stringIn[i] < stringIn[i + 1]) {
			index1 = i;
			break;
		}
	}
	if (i < 0)
		return 0;
	for (i = len - 1; i > 0; i--) {
		if ((stringIn[i]) > (stringIn[index1])) {
			index2 = i;
			break;
		}
	}
	c = stringIn[index1];
	stringIn[index1] = stringIn[index2];
	stringIn[index2] = c;
	reverse(stringIn, index1, len);
	return 1;
}


int main(void) {
	unsigned int n = 0;
	char stringIn[64];
	if (fgets(stringIn, sizeof stringIn, stdin) == NULL)
		bad_input();
	if (scanf("%u", &n) != 1)
		bad_input();
	if (strlen(stringIn) == 2)
		return 0;
	else {
		char nums[10] = { 0 };
		for (unsigned int i = 0; stringIn[i] != 0 && stringIn[i] != '\n'; i++) {
			if (!isdigit(stringIn[i]))
				bad_input();
			else {
				nums[stringIn[i] - '0']++;
			}
		}
		for (unsigned int i = 0; i < 10; i++) {
			if (nums[i] > 1)
				bad_input();
		}
			unsigned int len = strlen(stringIn) - 1;
			for (unsigned int i = 0; i < n; i++) {
				if (permutation(stringIn, len) == 1)
					printf("%s", stringIn);
				else
					return 0;
			}
		}
		return 0;
	}
