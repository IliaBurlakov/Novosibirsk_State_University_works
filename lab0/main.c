#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


unsigned long long power(int x, int n) {
	unsigned long long s = 1;
	for (int i = 1; i <= n; i++)
		s = s * x;
	return s;
}


int char_to_number(char sym) {
	int result;
	if (isdigit(sym) != 0)
		result = sym - '0';
	else {
		if (tolower(sym) >= 'a' && tolower(sym) <= 'f')
			result = (tolower(sym) - 'a' + 10);
		else
			result = 16;
	}
	return result;
}


char number_to_char(char x) {
	if (x >= 0 && x <= 9)
		return x + '0';
	else if (x >= 10 && x <= 16)
		return x - 10 + 'a';
	return 0;
}


void reverse(char* x) {
	int length = strlen(x) - 1;
	for (int i = 0, j = length, c; i < j; i++, j--) {
		c = x[i];
		x[i] = x[j];
		x[j] = c;
	}
}


void bad_input(void) {
	printf("bad input");
	exit(0);
}


int convert(char* stringIn, char* result, int b1, int b2) {
	unsigned long long mainpart_temp = 0;
	unsigned int i = 0;
	unsigned int stringIn_len = strlen(stringIn);
	if ((stringIn_len > 13) || stringIn[i] == '.')
		return 0;
	if ((stringIn_len == 1) && stringIn[i] == '0') {
		result[0] = '0';
		return 1;
	}
	unsigned long long numerator = 0, denominator = 1;
	for (; stringIn[i] != 0 && stringIn[i] != '.'; i++) {
		mainpart_temp *= b1;
		if (char_to_number(stringIn[i]) < b1)
			mainpart_temp += char_to_number(stringIn[i]);
		else
			return 0;
	}
	if ((stringIn[i] == '.' && i > 11) || (stringIn[i] == '.' && (char_to_number(stringIn[i + 1]) == 16)))
		return 0;
	int isfrac = 0;
	if (stringIn[i] == '.')
		isfrac = 1;
	if (isfrac == 1) {
		unsigned int afterdotpartb1_len = stringIn_len - i - 1;
		unsigned long long degree = afterdotpartb1_len;
		for (i = 0; i < afterdotpartb1_len; i++) {
			char c = char_to_number(stringIn[stringIn_len - afterdotpartb1_len + i]);
			if (stringIn[stringIn_len - afterdotpartb1_len + i] == '.')
				return 0;
			degree = degree - 1;
			denominator = denominator * b1;
			numerator += c * power(b1, degree);
		}
	}
	i = 0;
	while (mainpart_temp > 0) {
		result[i] = number_to_char(mainpart_temp % b2);
		mainpart_temp /= b2;
		i++;
	}
	reverse(result);
	if (isfrac == 1) {
		result[i] = '.';
		i++;
		unsigned int j = 0;
		while (numerator > 0 || j < 13) {
			int p = numerator * b2 / denominator;
			numerator = numerator * b2 - p * denominator;
			result[i + j] = number_to_char(p);
			j++;
			if (j > 11)
				break;
		}
	}
	return 1;
}


int main(void) {
	char stringIn[16], result[128] = "";
	int b1, b2;
	if (scanf("%d %d", &b1, &b2) != 2)
		bad_input();
	if (scanf("%14s", stringIn) != 1)
		bad_input();
	if (b1 < 2 || b1 > 16 || b2 < 2 || b2 > 16)
		bad_input();
	if (convert(stringIn, result, b1, b2) == 1) {
		if (result[0] == '.')
			printf("0%s\n", result);
		else
			printf("%s\n", result);
		return 0;
	}
	else
		bad_input();
}
