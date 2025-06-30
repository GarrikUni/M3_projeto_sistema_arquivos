#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"
#include "btree.h"

static void free_btree_nodes_recursive(BTreeNode* node);

static void free_btree_nodes_recursive(BTreeNode* node) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < node->n; i++) {
        if (!node->leaf) {
            free_btree_nodes_recursive(node->children[i]);
        }

        TreeNode* item = node->keys[i];
        if (item != NULL) {
            if (item->type == FILE_TYPE) {
                free(item->data.file->content);
                free(item->data.file->name);
                free(item->data.file);
            } else {
                free_directory_contents_recursive(item->data.directory);
                free(item->data.directory->tree->root->keys);
                free(item->data.directory->tree->root->children);
                free(item->data.directory->tree->root);
                free(item->data.directory->tree);
                free(item->data.directory);
            }
            free(item->name);
            free(item);
            node->keys[i] = NULL;
        }
    }

    if (!node->leaf) {
        if (node->children[node->n] != NULL) {
            free_btree_nodes_recursive(node->children[node->n]);
        }
    }

    free(node->keys);
    free(node->children);
    free(node);
}

void free_directory_contents_recursive(Directory* dir) {
    if (dir == NULL || dir->tree == NULL || dir->tree->root == NULL) {
        return;
    }
    free_btree_nodes_recursive(dir->tree->root);
}


Directory* get_root_directory() {
    Directory* dir = malloc(sizeof(Directory));
    dir->tree = btree_create(2);
    return dir;
}

TreeNode* create_txt_file(const char* name, const char* content) {
    File* file = malloc(sizeof(File));
    file->name = strdup(name);
    file->content = strdup(content);
    file->size = strlen(content);

    TreeNode* node = malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->type = FILE_TYPE;
    node->data.file = file;

    return node;
}

TreeNode* create_directory(const char* name) {
    Directory* dir = malloc(sizeof(Directory));
    dir->tree = btree_create(2);

    TreeNode* node = malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->type = DIRECTORY_TYPE;
    node->data.directory = dir;

    return node;
}

void list_directory_contents(Directory* dir) {
    printf("Conteúdo do diretório:\n");
    btree_traverse(dir->tree);
}

void change_directory_path(DirStack* stack, const char* path) {
    char* temp_path = strdup(path);
    if (temp_path == NULL) {
        perror("Memory allocation failed");
        return;
    }

    char* token = strtok(temp_path, "/");
    while (token) {
        if (strcmp(token, "..") == 0) {
            pop(stack);
        } else {
            Directory* current = top(stack);
            TreeNode* node = btree_search(current->tree, token, DIRECTORY_TYPE);
            if (node) {
                push(stack, node->data.directory);
            } else {
                printf("Diretório não encontrado: %s\n", token);
                free(temp_path);
                return;
            }
        }
        token = strtok(NULL, "/");
    }
    printf("Diretório atual alterado.\n");
    free(temp_path);
}

DirStack* create_stack(int capacity) {
    DirStack* s = malloc(sizeof(DirStack));
    s->stack = malloc(sizeof(Directory*) * capacity);
    s->size = 0;
    s->capacity = capacity;
    return s;
}

void push(DirStack* s, Directory* dir) {
    if (s->size < s->capacity) {
        s->stack[s->size++] = dir;
    }
}

Directory* pop(DirStack* s) {
    if (s->size == 0) return NULL;
    return s->stack[--s->size];
}

Directory* top(DirStack* s) {
    if (s->size == 0) return NULL;
    return s->stack[s->size - 1];
}

void free_stack(DirStack* s) {
    free(s->stack);
    free(s);
}

int exists_node_of_type(Directory* dir, const char* name, NodeType type) {
    TreeNode* node = btree_search(dir->tree, name, type);
    return node != NULL;
}

void delete_txt_file(Directory* current_dir, const char* name) {
    TreeNode* file_node = btree_search(current_dir->tree, name, FILE_TYPE);
    if (file_node) {
        free(file_node->data.file->content);
        free(file_node->data.file->name);
        free(file_node->data.file);

        btree_delete(current_dir->tree, name, FILE_TYPE);
        printf("Arquivo '%s' excluído com sucesso.\n", name);
    } else {
        printf("Erro: arquivo '%s' não encontrado.\n", name);
    }
}

void delete_directory(Directory* current_dir, const char* name) {
    TreeNode* dir_node = btree_search(current_dir->tree, name, DIRECTORY_TYPE);
    if (dir_node) {
        if (dir_node->data.directory->tree->root->n == 0) {
            free(dir_node->data.directory->tree->root->keys);
            free(dir_node->data.directory->tree->root->children);
            free(dir_node->data.directory->tree->root);
            free(dir_node->data.directory->tree);
            free(dir_node->data.directory);

            btree_delete(current_dir->tree, name, DIRECTORY_TYPE);
            printf("Diretório '%s' excluído com sucesso.\n", name);
        } else {
            printf("Erro: diretório '%s' não está vazio.\n", name);
        }
    } else {
        printf("Erro: diretório '%s' não encontrado.\n", name);
    }
}

void write_to_txt_file(Directory* current_dir, const char* name, const char* content) {
    TreeNode* file_node = btree_search(current_dir->tree, name, FILE_TYPE);
    if (file_node) {
        if (file_node->data.file->content) {
            free(file_node->data.file->content);
        }
        file_node->data.file->content = strdup(content);
        if (file_node->data.file->content == NULL && strlen(content) > 0) {
            perror("Failed to allocate memory for file content");
            return;
        }
        file_node->data.file->size = strlen(content);
        printf("Conteúdo gravado em '%s'.\n", name);
    } else {
        printf("Erro: arquivo '%s' não encontrado para escrita.\n", name);
    }
}

void read_txt_file(Directory* current_dir, const char* name) {
    TreeNode* file_node = btree_search(current_dir->tree, name, FILE_TYPE);
    if (file_node) {
        printf("Conteúdo de '%s':\n%s\n", name, file_node->data.file->content);
    } else {
        printf("Erro: arquivo '%s' não encontrado para leitura.\n", name);
    }
}