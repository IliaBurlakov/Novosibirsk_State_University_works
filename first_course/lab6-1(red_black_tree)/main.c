#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>


enum en_Color { RED, BLACK };
typedef enum en_Color Color;


struct st_Node {
    struct st_Node* left;
    struct st_Node* right;
    int data;
    Color color;
};
typedef struct st_Node Node;


Node* RotateLeft(Node* x) {
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    y->color = x->color;
    x->color = RED;
    return y;
}


Node* RotateRight(Node* x) {
    Node* y = x->left;
    x->left = y->right;
    y->right = x;
    y->color = x->color;
    x->color = RED;
    return y;
}


int IsRed(Node* x) {
    return !x ? 0 : (x->color == RED ? 1 : 0);
}


Node* FixInsertion(Node* node, short link) { //"node" is grandparent
    if (!link) { //parent is a left child
        if ((IsRed(node->left)) && (IsRed(node->right))) { //red parent and red uncle
            if ((IsRed(node->left->left)) || (IsRed(node->left->right))) { //red parent has red child
                node->left->color = BLACK;
                node->right->color = BLACK;
                node->color = RED;
            }
        }
        else { //uncle is black
            if ((IsRed(node->left)) && (IsRed(node->left->left)))
                node = RotateRight(node);
            else if (IsRed(node->left) && IsRed(node->left->right)) {
                node->left = RotateLeft(node->left);
                node = RotateRight(node);
            }
        }
    }
    else { //parent is a right child
        if ((IsRed(node->left)) && (IsRed(node->right))) { //red parent and red uncle
            if ((IsRed(node->right->right)) || (IsRed(node->right->left))) { //red parent has red child
                node->left->color = BLACK;
                node->right->color = BLACK;
                node->color = RED;
            }
        }
        else { //uncle is black
            if ((IsRed(node->right)) && (IsRed(node->right->right)))
                node = RotateLeft(node);
            else if ((IsRed(node->right)) && (IsRed(node->right->left))) {
                node->right = RotateRight(node->right);
                node = RotateLeft(node);
            }
        }
    }
    return node;
}


Node* InsertData(Node* root, int data, Node* node) {
    if (root == NULL) {
        node->data = data;
        node->color = RED;
        node->left = node->right = NULL;
        root = node;
    }
    else {
        short link;
        if (data < root->data) {
            link = 0;
            root->left = InsertData(root->left, data, node);
        }
        else {
            link = 1;
            root->right = InsertData(root->right, data, node);
        }
        return FixInsertion(root, link);
    }
    return root;
}


int ComputeBlackHeight(Node* node) {
    int count = 0;
    while (node) {
        if (node->color == BLACK) {
            count++;
        }
        node = node->left;
    }
    return count + 1;
}


int main() {
    int n;
    if (!scanf("%d", &n) || n <= 0) {
        printf("0");
        return 0;
    }

    Node* tree = malloc(sizeof(Node) * n);
    Node* root = NULL;

    int data;
    for (int i = 0; i < n; i++) {
        if (!scanf("%d", &data)) {
            free(tree);
            return 0;
        }
        root = InsertData(root, data, &tree[i]);
    }
    root->color = BLACK;
    printf("%d", ComputeBlackHeight(root));
    free(tree);
    return 0;
}
