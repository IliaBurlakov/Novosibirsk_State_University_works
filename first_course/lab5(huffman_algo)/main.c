#define BUFF_SIZE 128000u


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct st_Data Data;
struct st_Data {
    FILE* file;
    unsigned char buffer[BUFF_SIZE];
    size_t pos;
    size_t size;
    unsigned int bitpos;
};


typedef struct st_Node Node;
struct st_Node {
    int sym;
    size_t freq;
    Node* left;
    Node* right;
};


int compare_function(const void* a, const void* b) {
    return (int)((*(Node**)b)->freq) - (int)((*(Node**)a)->freq);
}


void freq_count(size_t* freq, Data* in) {
    do {
        for (size_t i = in->pos; i < in->size; i++)
            freq[in->buffer[i]]++;
        in->pos = 0;
        in->size = fread(in->buffer, 1, BUFF_SIZE, in->file);
    } while (in->size > 0);
}


Node* mk_tree(size_t* freq) {
    Node* slice[256];
    slice[0] = NULL;
    size_t n = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            Node* newNode = (Node*)malloc(sizeof(Node));
            newNode->sym = i;
            newNode->freq = freq[i];
            newNode->left = NULL;
            newNode->right = NULL;
            slice[n++] = newNode;
        }
    }
    while (n > 1) {
        qsort(slice, n, sizeof(Node*), compare_function);//sort from bigger freq to smaller, 2 last nodes are being connected into 1
        Node* node = (Node*)malloc(sizeof(Node));
        node->sym = 0;
        node->freq = slice[n - 2]->freq + slice[n - 1]->freq;
        node->left = slice[n - 2];
        node->right = slice[n - 1];
        slice[n - 2] = node;
        n--;
    }
    return slice[0];
}


void free_tree(Node* root) {
    if (root == NULL) {
        return;
    }
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}


void fill_table(char* code_table[256], Node* root, char path[256], size_t depth) {
    if (root->left != NULL) {
        path[depth++] = '0';
        fill_table(code_table, root->left, path, depth);
        depth--;
        path[depth++] = '1';
        fill_table(code_table, root->right, path, depth);
    }
    else {
        path[depth] = 0;
        code_table[root->sym] = (char*)memcpy(malloc(depth + 1), path, depth + 1);
    }
}


void free_code_table(char* code_table[256]) {
    for (int i = 0; i < 256; i++) {
        if (code_table[i] != NULL) {
            free(code_table[i]);
        }
    }
}


void write_sym(Data* out, unsigned char sym) {
    if (out->bitpos == 0) {
        out->buffer[out->pos] = (unsigned char)sym;
    }
    else {
        out->buffer[out->pos] |= (unsigned char)sym >> out->bitpos;
        out->buffer[out->pos + 1] = (unsigned char)sym << (8 - out->bitpos);
    }
    ++out->pos;
}


void write_tree(Node* root, Data* out) {
    if (root->left == NULL) {
        ++(out->bitpos);
        if (out->bitpos == 8) {
            out->bitpos = 0;
            ++(out->pos);
        }
        write_sym(out, root->sym);
    }
    else {
        out->buffer[out->pos] |= 128 >> out->bitpos;
        if (++out->bitpos == 8) {
            out->bitpos = 0;
            ++(out->pos);
        }
        write_tree(root->left, out);
        write_tree(root->right, out);
    }

}


void encode_huffman(Data* in, Data* out, char* code_table[256]) {
    do {
        for (size_t i = in->pos; i < in->size; i++) {
            int k = 0;
            int bit = code_table[in->buffer[i]][k];
            while (bit != 0) {
                if (bit == '1')
                    out->buffer[out->pos] |= 128u >> (out->bitpos);
                if (++out->bitpos >= 8) {
                    out->bitpos = 0;
                    ++out->pos;
                    if (out->pos >= BUFF_SIZE) {
                        fwrite(out->buffer, 1, out->pos, out->file);
                        memset(out->buffer, 0, BUFF_SIZE);
                        out->pos = 0;
                    }
                }
                k++;
                bit = code_table[in->buffer[i]][k];
            }
        }
        in->pos = 0;
        in->size = fread(in->buffer, 1, BUFF_SIZE, in->file);
    } while (in->size > 0);
    size_t extra_byte = (out->bitpos == 0) ? 0 : 1;
    fwrite(out->buffer, 1, out->pos + extra_byte, out->file);
}


int encode(Data* in) {
    size_t freq[256] = { 0 };
    size_t start_pos = in->pos;
    freq_count(freq, in);
    size_t input_size = (size_t)ftell(in->file) - start_pos;
    fseek(in->file, (long)start_pos, SEEK_SET);
    Node* root = mk_tree(freq);
    char* code_table[256] = { NULL };
    char path[256];
    fill_table(code_table, root, path, 0);

    FILE* file2 = fopen("out.txt", "wb");
    if (file2 == NULL) {
        free_tree(root);
        free_code_table(code_table);
        printf("cannot open file");
        return 0;
    }
    Data* out = (Data*)malloc(sizeof(Data));
    if (out == NULL) {
        ;
        free_tree(root);
        free_code_table(code_table);
        fclose(file2);
        return 0;
    }
    out->file = file2;
    memset(out->buffer, 0, BUFF_SIZE);
    out->buffer[0] = (unsigned char)(input_size >> 24);
    out->buffer[1] = (unsigned char)(input_size >> 16);
    out->buffer[2] = (unsigned char)(input_size >> 8);
    out->buffer[3] = (unsigned char)input_size;
    out->pos = 4;
    out->bitpos = 0;
    write_tree(root, out);
    encode_huffman(in, out, code_table);
    free(out);
    free_tree(root);
    free_code_table(code_table);
    fclose(file2);
    return 1;
}


int getbyte(Data* in) {
    int byte;
    if (in->bitpos == 0) {
        byte = in->buffer[in->pos++];
        if (in->pos == in->size) {
            in->size = fread(in->buffer, 1, BUFF_SIZE, in->file);
            in->pos = 0;
        }
    }
    else {
        byte = (unsigned char)(in->buffer[in->pos++] << in->bitpos);
        if (in->pos == in->size) {
            in->size = fread(in->buffer, 1, BUFF_SIZE, in->file);
            in->pos = 0;
        }
        byte |= (unsigned char)in->buffer[in->pos] >> (8 - in->bitpos);
    }
    return byte;
}


int getbit(Data* in) {
    int byte = in->buffer[in->pos];
    int bit = (byte >> (7 - in->bitpos)) & 1;
    if (++in->bitpos >= 8) {
        if (++in->pos >= in->size) {
            in->size = fread(in->buffer, 1, BUFF_SIZE, in->file);
            in->pos = 0;
        }
        in->bitpos = 0;
    }
    return bit;
}


Node* restore_tree(Data* in) {
    Node* nd = (Node*)malloc(sizeof(Node));
    if (nd == NULL) {
        printf("Unable to allocate memory for nd!");
        exit(0);
    }
    int bit = getbit(in);
    if (!bit) {
        nd->left = nd->right = NULL;
        nd->sym = getbyte(in);
    }
    else {
        nd->left = restore_tree(in);
        nd->right = restore_tree(in);
    }
    return nd;
}


int decode(Data* in) {
    size_t out_count;
    out_count = ((size_t)(in->buffer[1] << 24)) | ((size_t)(in->buffer[2] << 16)) | ((size_t)(in->buffer[3] << 8)) | ((size_t)in->buffer[4]);
    FILE* file2 = fopen("out.txt", "wb");
    if (file2 == NULL) {
        printf("cannot open file");
        return 0;
    }
    Data* out = (Data*)malloc(sizeof(Data));
    if (out == NULL) {
        fclose(file2);
        return 0;
    }
    out->pos = 0;
    out->bitpos = 0;
    out->file = file2;
    in->pos = 5;
    in->bitpos = 0;
    Node* tree = restore_tree(in);
    size_t i = 0;
    while (i < out_count) {
        Node* cur = tree;
        while (cur->left != NULL) {
            if (getbit(in))
                cur = cur->right;
            else
                cur = cur->left;
        }
        out->buffer[out->pos++] = (unsigned char)cur->sym;
        if (out->pos >= BUFF_SIZE) {
            fwrite(out->buffer, 1, out->pos, out->file);
            out->pos = 0;
        }
        i++;
    }
    fwrite(out->buffer, 1, out->pos, out->file);
    fclose(file2);
    free(out);
    return 1;
}


int main(void) {
    FILE* file1 = fopen("in.txt", "rb");
    if (file1 == NULL)
        return 0;
    Data* in = (Data*)malloc(sizeof(Data));
    if (in == NULL) {
        fclose(file1);
        return 0;
    }
    in->file = file1;
    in->pos = 0;

    if ((in->size = fread(in->buffer, sizeof(unsigned char), BUFF_SIZE, file1)) < 2) {
        free(in);
        fclose(file1);
        return 0;
    }
    if (in->buffer[0] == 'c') {
        in->pos = 1;
        if (!encode(in)) {
            printf("An error occurred while encoding!\n");
            fclose(file1);
            free(in);
            return 0;
        }
    }
    else {
        in->pos = 1;
        if (!decode(in)) {
            printf("An error occurred while decoding!\n");
            fclose(file1);
            free(in);
            return 0;
        }
    }
    fclose(file1);
    free(in);
    return 0;
}