#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdlib.h>

typedef enum { FILE_TYPE, DIRECTORY_TYPE } NodeType;

typedef struct File {
    char* name;
    char* content;
    size_t size;
} File;

struct Directory;

typedef struct TreeNode {
    char* name;
    NodeType type;
    union {
        File* file;
        struct Directory* directory;
    } data;
} TreeNode;

typedef struct BTree BTree;

typedef struct Directory {
    BTree* tree;
} Directory;

typedef struct DirStack {
    Directory** stack;
    int size;
    int capacity;
} DirStack;

DirStack* create_stack(int capacity);
void push(DirStack* s, Directory* dir);
Directory* pop(DirStack* s);
Directory* top(DirStack* s);
void free_stack(DirStack* s);

TreeNode* create_txt_file(const char* name, const char* content);
TreeNode* create_directory(const char* name);
void list_directory_contents(Directory* dir);
Directory* get_root_directory();
void change_directory_path(DirStack* stack, const char* path);

int exists_node_of_type(Directory* dir, const char* name, NodeType type);

void delete_txt_file(Directory* current_dir, const char* name);
void delete_directory(Directory* current_dir, const char* name);

void free_directory_contents_recursive(Directory* dir);

void write_to_txt_file(Directory* current_dir, const char* name, const char* content);
void read_txt_file(Directory* current_dir, const char* name);

#endif