#define _CRT_SECURE_NO_WARNINGS 
#define MAX_KEYS (2 * t - 1)
#define MIN_KEYS (t - 1)


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


int t;


struct st_bTree {
    int keys_count;
    int* keys;
    struct st_bTree** child;
    bool leaf;
};
typedef struct st_bTree bTree;


bTree* create_node(bool leaf) {
    bTree* node = (bTree*)malloc(sizeof(bTree));
    node->leaf = leaf;
    node->keys = (int*)malloc(MAX_KEYS * sizeof(int));
    node->child = (bTree**)malloc((MAX_KEYS + 1) * sizeof(bTree*));
    node->keys_count = 0;
    return node;
}


void freeBTree(bTree** root) {
    if (*root == NULL) return;
    if (!(*root)->leaf) {
        for (int i = 0; i <= (*root)->keys_count; ++i) {
            freeBTree(&((*root)->child[i]));
        }
    }
    free((*root)->keys);
    free((*root)->child);
    free(*root);
    *root = NULL;
}


int calculateHeight(bTree* root) {
    int height = 0;
    if (root != NULL) {
        height++;
    }

    while (root != NULL && !root->leaf) {
        root = root->child[0];
        height++;
    }
    return height;
}


void shiftAndInsertKey(bTree* node, int key, int insertPos) {
    for (int i = node->keys_count - 1; i >= insertPos; i--) {
        node->keys[i + 1] = node->keys[i];
    }
    node->keys[insertPos] = key;
    node->keys_count++;
}


int binarySearch(int* keys, int low, int high, int key) {
    while (low <= high) {
        int mid = (low + high) / 2;
        if (keys[mid] < key) {
            low = mid + 1;
        }
        else if (keys[mid] > key) {
            high = mid - 1;
        }
        else {
            return mid;
        }
    }
    return low;
}


void copyKeysAndChildren(bTree* src, bTree* dest, int start, int count) {
    for (int j = 0; j < count; j++) {
        dest->keys[j] = src->keys[start + j];
        if (!src->leaf) {
            dest->child[j] = src->child[start + j];
        }
    }
    if (!src->leaf) {
        dest->child[count] = src->child[start + count];
    }
}


void shiftChildren(bTree* node, int start, int end) {
    for (int j = end; j >= start; j--) {
        node->child[j + 1] = node->child[j];
    }
}


void shiftParentKeys(bTree* node, int start, int end) {
    for (int j = end - 1; j >= start; j--) {
        node->keys[j + 1] = node->keys[j];
    }
}


void splitChild(bTree* parent, int childIndex) {
    bTree* child = parent->child[childIndex];
    bTree* newChild = create_node(child->leaf);
    newChild->keys_count = MIN_KEYS;

    copyKeysAndChildren(child, newChild, t, MIN_KEYS);

    child->keys_count = t - 1;

    shiftChildren(parent, childIndex, parent->keys_count);
    parent->child[childIndex + 1] = newChild;

    shiftParentKeys(parent, childIndex, parent->keys_count);
    parent->keys[childIndex] = child->keys[t - 1];

    parent->keys_count++;
}


void insertNonFull(bTree* node, int key) {
    int insertPos = binarySearch(node->keys, 0, node->keys_count - 1, key);
    if (node->leaf)
        shiftAndInsertKey(node, key, insertPos);
    else {
        if (node->child[insertPos]->keys_count == MAX_KEYS) {
            splitChild(node, insertPos);
            if (key > node->keys[insertPos])
                insertPos++;
        }
        insertNonFull(node->child[insertPos], key);
    }
}


void insert(bTree** root, int key) {
    if ((*root)->keys_count == MAX_KEYS) {
        bTree* newRoot = create_node(false);
        newRoot->child[0] = *root;
        splitChild(newRoot, 0);
        int i = 0;
        if (newRoot->keys[0] < key)
            i++;
        insertNonFull(newRoot->child[i], key);
        *root = newRoot;
    }
    else {
        insertNonFull(*root, key);
    }
}


int main(void) {
    FILE* file = fopen("in.txt", "r");
    if (file == NULL) 
        return 0;
    int n;
    if (fscanf(file, "%d\n%d", &t, &n) != 2) {
        fclose(file);
        return 0;
    }

    if (n == 0) {
        printf("0\n");
        fclose(file);
        return 0;
    }

    bTree* root = create_node(true);

    int key;
    for (int i = 0; i < n; i++) {
        if (fscanf(file, "%d", &key) != 1) {
            fclose(file);
            freeBTree(&root);
            return 0;
        }
        insert(&root, key);
    }

    printf("%d\n", calculateHeight(root));

    freeBTree(&root);
    fclose(file);
    return 0;
}