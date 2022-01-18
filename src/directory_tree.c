#include "directory_tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const size_t PERMISSION = 0777;

void init_node(node_t *node, char *name, node_type_t type) {
    if (name == NULL) {
        name = strdup("ROOT");
        assert(name != NULL);
    }
    node->name = name;
    node->type = type;
}

file_node_t *init_file_node(char *name, size_t size, uint8_t *contents) {
    file_node_t *node = malloc(sizeof(file_node_t));
    assert(node != NULL);
    init_node((node_t *) node, name, FILE_TYPE);
    node->size = size;
    node->contents = contents;
    return node;
}

directory_node_t *init_directory_node(char *name) {
    directory_node_t *node = malloc(sizeof(directory_node_t));
    assert(node != NULL);
    init_node((node_t *) node, name, DIRECTORY_TYPE);
    node->num_children = 0;
    node->children = NULL;
    return node;
}

void add_child_directory_tree(directory_node_t *dnode, node_t *child) {
    size_t increased_children = dnode->num_children + 1;
    dnode->children = realloc(dnode->children, sizeof(node_t *) * increased_children);
    dnode->num_children = increased_children;
    dnode->children[increased_children - 1] = child;
    size_t i = increased_children - 1;
    while (i >= 1) {
        if (strcmp(dnode->children[i]->name, dnode->children[i - 1]->name) < 0) {
            dnode->children[i] = dnode->children[i - 1];
            dnode->children[i - 1] = child;
        }
        else {
            break;
        }
        i--;
    }
}

void print_spaces(size_t count) {
    for (size_t i = 0; i < count; i++) {
        printf("    ");
    }
}

void print_helper(node_t *node, size_t count) {
    print_spaces(count);
    printf("%s\n", node->name);
    if (node->type == DIRECTORY_TYPE) {
        directory_node_t *dnode = (directory_node_t *) node;
        for (size_t i = 0; i < dnode->num_children; i++) {
            print_helper(dnode->children[i], count + 1);
        }
    }
}

void print_directory_tree(node_t *node) {
    print_helper(node, 0);
}

void create_helper(node_t *node, char *str) {
    char *new_str = malloc(sizeof(char) * (strlen(node->name) + strlen(str) + 2));
    strcpy(new_str, str);
    strcat(new_str, "/");
    strcat(new_str, node->name);

    if (node->type == FILE_TYPE) {
        file_node_t *fnode = (file_node_t *) node;
        FILE *fp = fopen(new_str, "w");
        assert(fwrite(fnode->contents, sizeof(uint8_t), fnode->size, fp) == fnode->size);
        assert(fclose(fp) == 0);
    }
    else {
        assert(node->type == DIRECTORY_TYPE);
        mkdir(new_str, PERMISSION);
        directory_node_t *dnode = (directory_node_t *) node;
        for (size_t i = 0; i < dnode->num_children; i++) {
            create_helper(dnode->children[i], new_str);
        }
    }
    free(new_str);
}

void create_directory_tree(node_t *node) {
    create_helper(node, ".");
}

void free_directory_tree(node_t *node) {
    if (node->type == FILE_TYPE) {
        file_node_t *fnode = (file_node_t *) node;
        free(fnode->contents);
    }
    else {
        assert(node->type == DIRECTORY_TYPE);
        directory_node_t *dnode = (directory_node_t *) node;
        for (size_t i = 0; i < dnode->num_children; i++) {
            free_directory_tree(dnode->children[i]);
        }
        free(dnode->children);
    }
    free(node->name);
    free(node);
}
