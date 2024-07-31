#define BUFF_LEN 999999


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void make_shift_table(size_t* shift_table, const char* sample, size_t sample_len) {
	size_t i = 1;
	size_t j = 0;
	shift_table[0] = 0;
	while (i < sample_len) {
		if (sample[i] == sample[j]) {
			shift_table[i] = j + 1;
			i++;
			j++;
		}
		else {
			if (j == 0) {
				shift_table[i] = 0;
				i++;
			}
			else
				j = shift_table[j - 1];
		}
	}
}


size_t kmp_search(const char* string, size_t string_len, const char* sample, size_t sample_len, const size_t* shift_table, size_t global_pos) {
	size_t i = 0;
	size_t j = 0;
	size_t temp = global_pos + 1 - sample_len;
	while (i < string_len) {
		if (string[i] == sample[j]) {
			j++;
			if (j == sample_len) {
				printf("%zu %zu ", i + temp, sample_len);
				j = shift_table[j - 1];
			}
			i++;
		}
		else {
			if (j == 0) {
				i++;
				if (i == string_len)
					break;
			}
			else {
				if (i - j + sample_len <= string_len) {
					printf("%zu %zu ", i - j + global_pos, j);
					j = shift_table[j - 1];
				}
				else
					break;
			}
		}
	}
	if (j != sample_len)
		return string_len - i + j;
	else
		return 0;
}


int main(void) {
	FILE* file = fopen("in.txt", "r");
	if (file == NULL)
		return 0;
	char sample[128];
	if (fgets(sample, sizeof(sample), file) == NULL) {
		fclose(file);
		return 0;
	}
	size_t sample_len = strlen(sample) - 1;
	if (sample_len == 0) {
		fclose(file);
		return 0;
	}
	size_t* shift_table = malloc(sizeof(size_t) * sample_len);
	make_shift_table(shift_table, sample, sample_len);
	for (size_t i = 0; i < sample_len; i++)
		printf("%zu ", shift_table[i]);
	printf("\n");
	char string[BUFF_LEN];
	size_t read_len;
	size_t tail_len = 0;
	size_t global_pos = 1;
	while ((read_len = fread(string + tail_len, 1, BUFF_LEN - tail_len, file)) > 0) {
		size_t string_len = read_len + tail_len;
		tail_len = kmp_search(string, string_len, sample, sample_len, shift_table, global_pos);
		size_t used_len = string_len - tail_len;
		memmove(string, string + used_len, tail_len);
		global_pos += used_len;
	}
	fclose(file);
	free(shift_table);
}
