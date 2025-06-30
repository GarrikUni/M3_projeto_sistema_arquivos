#ifndef BTREE_H
#define BTREE_H

#include "filesystem.h"

typedef struct BTreeNode {
    TreeNode** keys;
    int t;
    struct BTreeNode** children;
    int n;
    int leaf;
} BTreeNode;

struct BTree {
    BTreeNode* root;
    int t;
};

BTree* btree_create(int t);
void btree_insert(BTree* tree, TreeNode* node);
TreeNode* btree_search(BTree* tree, const char* name, NodeType type);
void btree_traverse(BTree* tree);
void btree_delete(BTree* tree, const char* name, NodeType type);

#endif