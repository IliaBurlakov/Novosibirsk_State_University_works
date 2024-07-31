#define BUFF_SIZE 999999


#include <string.h>
#include <stdio.h>


unsigned int calculate_hash(const char* str, size_t sample_len, unsigned int* pow3) {
    unsigned int power = 1;
    unsigned int hash = 0;
    for (size_t i = 0; i < sample_len; i++) {
        hash += ((unsigned char)str[i] % 3) * power;
        power *= 3;
    }
    *pow3 = power;
    return hash;
}


void rk_search(const char* str, size_t str_len, const char* sample, size_t sample_len, unsigned int sample_hash, size_t index) {
    if (str_len < sample_len)
        return;
    unsigned int pow3 = 0;
    unsigned int str_hash = calculate_hash(str, sample_len, &pow3);
    size_t i = 0;
    while (i <= str_len - sample_len) {
        if (str_hash == sample_hash) {
            for (size_t j = 0; j < sample_len; j++) {
                printf("%zu ", index + i + j);
                if (sample[j] != str[i + j])
                    break;
            }
        }
        str_hash = ((str_hash - ((unsigned char)str[i] % 3)) + ((unsigned char)str[i + sample_len] % 3) * pow3) / 3;
        i++;
    }
}


void rk_file(FILE* file, const char* sample, size_t sample_len) {
    unsigned int pow3 = 0;
    size_t index = 1, offset = 0;
    unsigned int sample_hash = calculate_hash(sample, sample_len, &pow3);
    printf("%u ", sample_hash);
    size_t read_len;
    char str[BUFF_SIZE + 1];
    while ((read_len = fread(str + offset, 1, BUFF_SIZE - offset, file)) > 0) {
        size_t str_len = read_len + offset;
        rk_search(str, str_len, sample, sample_len, sample_hash, index);
        offset = sample_len - 1;
        if (str_len > offset) {
            memmove(str, str + str_len - offset, offset);
        }
        index += str_len - offset;
    }
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
    size_t sample_len = strlen(sample) - 1;
    rk_file(file, sample, sample_len);
    fclose(file);
    return 0;
}
