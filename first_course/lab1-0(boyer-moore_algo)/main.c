#define _CRT_SECURE_NO_WARNINGS
#define BUFF_LEN 999999


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void make_shift_table(size_t* shift_table, const char* sample, size_t sample_len) {
	size_t i = 0;
	while (i < 256)
		shift_table[i++] = sample_len;
	i = 0;
	while (i < sample_len - 1) {
		shift_table[(unsigned char)sample[i]] = sample_len - i - 1;
		i++;
	}
}


size_t bm_search(const char* string, size_t string_len, const char* sample, size_t sample_len, size_t* shift_table, size_t global_pos) {
	size_t i = sample_len;
	while (i <= string_len) {
		size_t r = i;
		size_t k = sample_len;
		do {
			--k;
			--r;
			printf("%zu ", global_pos + r);
		} while (string[r] == sample[k] && k > 0);
		i += shift_table[(unsigned char)string[i - 1]];
	}
	return string_len + sample_len - i;
}


int main(void) {
	FILE* file = fopen("in.txt", "r");
	if (file == NULL)
		return 0;
	char sample[18];
	if (fgets(sample, 18, file) == NULL) {
		fclose(file);
		return 0;
	}
	size_t shift_table[256];
	char string[BUFF_LEN];
	size_t read_len;
	size_t tail_len = 0;
	size_t global_pos = 1;
	size_t sample_len = strlen(sample) - 1;
	make_shift_table(shift_table, sample, sample_len);
	while ((read_len = fread(string + tail_len, 1, BUFF_LEN - tail_len, file)) > 0) {
		size_t string_len = read_len + tail_len;
		tail_len = bm_search(string, string_len, sample, sample_len, shift_table, global_pos);
		size_t used_len = string_len - tail_len;
		memmove(string, string + used_len, tail_len);
		global_pos += used_len;
	}
	fclose(file);
	return 0;
}
