#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"
#include "btree.h"

int main() {
    Directory* root = get_root_directory();
    DirStack* stack = create_stack(100);
    push(stack, root);

    char line_buffer[256];

    while (1) {
        printf(">>> ");
        if (fgets(line_buffer, sizeof(line_buffer), stdin) == NULL) {
            break;
        }

        line_buffer[strcspn(line_buffer, "\n")] = 0;

        if (strncmp(line_buffer, "exit", 4) == 0) {
            break;
        }

        char* command_name;
        char* arg1 = NULL;
        char* arg2 = NULL;

        char* temp_line = strdup(line_buffer);
        if (temp_line == NULL) {
            perror("Memory allocation failed");
            continue;
        }

        command_name = strtok(temp_line, " ");

        if (command_name == NULL) {
            printf("Comando inválido.\n");
            free(temp_line);
            continue;
        }

        if (strcmp(command_name, "mkdir") == 0 ||
            strcmp(command_name, "touch") == 0 ||
            strcmp(command_name, "cd") == 0 ||
            strcmp(command_name, "rm") == 0 ||
            strcmp(command_name, "rmdir") == 0 ||
            strcmp(command_name, "cat") == 0) {
            arg1 = strtok(NULL, "");
        } else if (strcmp(command_name, "write") == 0) {
            arg1 = strtok(NULL, " ");
            arg2 = strtok(NULL, "");
        }


        if (strcmp(command_name, "mkdir") == 0) {
            if (arg1 == NULL || *arg1 == '\0') {
                printf("Uso: mkdir <nome_do_diretorio>\n");
            } else if (exists_node_of_type(top(stack), arg1, DIRECTORY_TYPE)) {
                printf("Erro: já existe um diretório com esse nome.\n");
            } else {
                TreeNode* dir = create_directory(arg1);
                btree_insert(top(stack)->tree, dir);
            }
        } else if (strcmp(command_name, "touch") == 0) {
            if (arg1 == NULL || *arg1 == '\0') {
                printf("Uso: touch <nome_do_arquivo>\n");
            } else if (exists_node_of_type(top(stack), arg1, FILE_TYPE)) {
                printf("Erro: já existe um arquivo com esse nome.\n");
            } else {
                TreeNode* file = create_txt_file(arg1, "");
                btree_insert(top(stack)->tree, file);
            }
        } else if (strcmp(command_name, "ls") == 0) {
            list_directory_contents(top(stack));
        } else if (strcmp(command_name, "cd") == 0) {
            if (arg1 == NULL || *arg1 == '\0') {
                printf("Uso: cd <caminho>\n");
            } else {
                change_directory_path(stack, arg1);
            }
        } else if (strcmp(command_name, "rm") == 0) {
            if (arg1 == NULL || *arg1 == '\0') {
                printf("Uso: rm <nome_do_arquivo>\n");
            } else {
                delete_txt_file(top(stack), arg1);
            }
        } else if (strcmp(command_name, "rmdir") == 0) {
            if (arg1 == NULL || *arg1 == '\0') {
                printf("Uso: rmdir <nome_do_diretorio>\n");
            } else {
                delete_directory(top(stack), arg1);
            }
        } else if (strcmp(command_name, "write") == 0) {
            if (arg1 == NULL || *arg1 == '\0') {
                printf("Uso: write <nome_do_arquivo> [conteúdo]\n");
            } else {
                write_to_txt_file(top(stack), arg1, (arg2 != NULL ? arg2 : ""));
            }
        } else if (strcmp(command_name, "cat") == 0) {
            if (arg1 == NULL || *arg1 == '\0') {
                printf("Uso: cat <nome_do_arquivo>\n");
            } else {
                read_txt_file(top(stack), arg1);
            }
        }
        else {
            printf("Comando inválido.\n");
        }
        free(temp_line);
    }

    while(stack->size > 1) {
        pop(stack);
    }
    
    free_directory_contents_recursive(top(stack));

    free_stack(stack);
    return 0;
}