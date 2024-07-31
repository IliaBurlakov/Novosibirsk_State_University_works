#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>


typedef struct st_Node Node;
struct st_Node {
    int key;
    int height;
    struct st_Node* left;
    struct st_Node* right;
};


int GetHeight(Node* node) {
    return node ? node->height : 0;
}


void FixHeight(Node* node) {
    int height_left = GetHeight(node->left);
    int height_right = GetHeight(node->right);
    node->height = (height_left > height_right ? height_left : height_right) + 1;
}


int Balance_factor(Node* p)
{
    return GetHeight(p->right) - GetHeight(p->left);
}


Node* RightRotation(Node* root) {
    Node* new_root = root->left;
    root->left = new_root->right;
    new_root->right = root;
    FixHeight(root);
    FixHeight(new_root);
    return new_root;
}


Node* LeftRotation(Node* root) {
    Node* new_root = root->right;
    root->right = new_root->left;
    new_root->left = root;
    FixHeight(root);
    FixHeight(new_root);
    return new_root;
}


Node* Balance(Node* node)
{
    FixHeight(node);
    if (Balance_factor(node) == 2)
    {
        if (Balance_factor(node->right) < 0)
            node->right = RightRotation(node->right);
        return LeftRotation(node);
    }
    if (Balance_factor(node) == -2)
    {
        if (Balance_factor(node->left) > 0)
            node->left = LeftRotation(node->left);
        return RightRotation(node);
    }
    return node;
}


Node* Insert(Node* root, Node* tree, int k, int i) {
    if (!root) {
        tree[i].key = k;
        tree[i].height = 1;
        tree[i].left = NULL;
        tree[i].right = NULL;
        root = &tree[i];
        return root;
    }
    if (k < root->key)
        root->left = Insert(root->left, tree, k, i);
    else
        root->right = Insert(root->right, tree, k, i);
    return Balance(root);
}


int main(void) {
    int n;
    if (!scanf("%d", &n))
        return 0;

    Node* tree = malloc(sizeof(Node) * n);
    Node* root = NULL;

    int key;
    for (int i = 0; i < n; i++) {
        if (!scanf("%d", &key))
            return 0;
        root = Insert(root, tree, key, i);
    }
    printf("%d", GetHeight(root));
    free(tree);
    return 0;
}
