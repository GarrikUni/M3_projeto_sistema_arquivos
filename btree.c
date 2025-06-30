#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"
#include "filesystem.h"

static BTreeNode* btree_create_node(int t, int leaf);
static void btree_split_child(BTreeNode* x, int i, BTreeNode* y);
static void btree_insert_non_full(BTreeNode* x, TreeNode* k);
static void btree_delete_from_node_at_index(BTreeNode* x, int idx);
static void btree_delete_recursive(BTreeNode* x, const char* name, NodeType type);
static void btree_traverse_recursive(BTreeNode* x);

static BTreeNode* btree_create_node(int t, int leaf) {
    BTreeNode* node = malloc(sizeof(BTreeNode));
    node->t = t;
    node->leaf = leaf;
    node->n = 0;
    node->keys = malloc(sizeof(TreeNode*) * (2 * t - 1));
    node->children = malloc(sizeof(BTreeNode*) * (2 * t));
    return node;
}

BTree* btree_create(int t) {
    BTree* tree = malloc(sizeof(BTree));
    tree->t = t;
    tree->root = btree_create_node(t, 1);
    return tree;
}

static void btree_split_child(BTreeNode* x, int i, BTreeNode* y) {
    BTreeNode* z = btree_create_node(y->t, y->leaf);
    z->n = y->t - 1;

    for (int j = 0; j < y->t - 1; j++) {
        z->keys[j] = y->keys[j + y->t];
    }

    if (!y->leaf) {
        for (int j = 0; j < y->t; j++) {
            z->children[j] = y->children[j + y->t];
        }
    }

    y->n = y->t - 1;

    for (int j = x->n; j >= i + 1; j--) {
        x->children[j + 1] = x->children[j];
    }
    x->children[i + 1] = z;

    for (int j = x->n - 1; j >= i; j--) {
        x->keys[j + 1] = x->keys[j];
    }
    x->keys[i] = y->keys[y->t - 1];
    x->n++;
}

static void btree_insert_non_full(BTreeNode* x, TreeNode* k) {
    int i = x->n - 1;

    if (x->leaf) {
        while (i >= 0 && strcmp(k->name, x->keys[i]->name) < 0) {
            x->keys[i + 1] = x->keys[i];
            i--;
        }
        x->keys[i + 1] = k;
        x->n++;
    } else {
        while (i >= 0 && strcmp(k->name, x->keys[i]->name) < 0) {
            i--;
        }
        i++;

        if (x->children[i]->n == (2 * x->t - 1)) {
            btree_split_child(x, i, x->children[i]);
            if (strcmp(k->name, x->keys[i]->name) > 0) {
                i++;
            }
        }
        btree_insert_non_full(x->children[i], k);
    }
}

void btree_insert(BTree* tree, TreeNode* k) {
    BTreeNode* r = tree->root;
    if (r->n == (2 * r->t - 1)) {
        BTreeNode* s = btree_create_node(r->t, 0);
        tree->root = s;
        s->children[0] = r;
        btree_split_child(s, 0, r);
        btree_insert_non_full(s, k);
    } else {
        btree_insert_non_full(r, k);
    }
}

// MODIFIED btree_search
TreeNode* btree_search(BTree* tree, const char* name, NodeType type) {
    BTreeNode* current = tree->root;
    while (current) {
        int i = 0;
        // Find the first key that is >= name
        while (i < current->n && strcmp(name, current->keys[i]->name) > 0) {
            i++;
        }

        // Iterate through all keys in the current node that match 'name'
        // This is crucial for handling items with same name but different types
        int search_idx = i;
        while (search_idx < current->n && strcmp(name, current->keys[search_idx]->name) == 0) {
            if (current->keys[search_idx]->type == type) {
                return current->keys[search_idx]; // Found exact match by name AND type
            }
            search_idx++;
        }

        // If not found in current node, descend to child
        if (current->leaf) {
            return NULL; // Not found and reached a leaf node
        } else {
            // Descend into the child corresponding to the initial 'i'
            current = current->children[i];
        }
    }
    return NULL;
}


static void btree_delete_from_node_at_index(BTreeNode* x, int idx) {
    for (int j = idx; j < x->n - 1; j++) {
        x->keys[j] = x->keys[j + 1];
    }
    x->n--;
}

static void btree_delete_recursive(BTreeNode* x, const char* name, NodeType type) {
    if (x == NULL) {
        return;
    }

    int i = 0;
    while (i < x->n && strcmp(name, x->keys[i]->name) > 0) {
        i++;
    }

    // Iterate through all potential keys with the exact name to find the right type
    int delete_idx = i;
    while (delete_idx < x->n && strcmp(name, x->keys[delete_idx]->name) == 0) {
        if (x->keys[delete_idx]->type == type) {
            btree_delete_from_node_at_index(x, delete_idx);
            return; // Item deleted
        }
        delete_idx++;
    }


    if (x->leaf) {
        return;
    } else {
        // If not found in current node, recurse into appropriate child
        btree_delete_recursive(x->children[i], name, type);
    }
}

void btree_delete(BTree* tree, const char* name, NodeType type) {
    if (tree == NULL || tree->root == NULL) {
        return;
    }
    btree_delete_recursive(tree->root, name, type);
}

static void btree_traverse_recursive(BTreeNode* x) {
    if (x == NULL) {
        return;
    }

    int i;
    for (i = 0; i < x->n; i++) {
        if (!x->leaf) {
            btree_traverse_recursive(x->children[i]);
        }
        TreeNode* item = x->keys[i];
        printf("[%s] %s\n", item->type == FILE_TYPE ? "ARQ" : "DIR", item->name);
    }

    if (!x->leaf) {
        btree_traverse_recursive(x->children[i]);
    }
}

void btree_traverse(BTree* tree) {
    if (tree && tree->root) {
        btree_traverse_recursive(tree->root);
    }
}