// Copyright Avram Cristian - Stefan 2022 stefanavram93@gmail.com
// Copyright Dumitrescu Rares - Matei 2022 mateidum828@gmail.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#define TREE_CMD_INDENT_SIZE 4
#define NO_ARG ""
#define PARENT_DIR ".."
#define TAB_SIZE 4


/*
* Function used to create FileTree, with root pointer initialized to
* a FolderNode with "root" name. RootNode has a NULL parent, as it is
* the first directory in the system.
*/
FileTree createFileTree(char* rootFolderName) {
    TreeNode *current_dir = malloc(sizeof(TreeNode));
    current_dir->parent = NULL;
    current_dir->name = malloc(strlen(rootFolderName) + 1);
    memcpy(current_dir->name, rootFolderName, strlen(rootFolderName) + 1);
    current_dir->type = FOLDER_NODE;
    current_dir->content = NULL;

    FileTree root_dir;
    root_dir.root = current_dir;

    return root_dir;
}

/*
* This function is used for freeing the given node, as all of its children.
*
* It works as a recursively function, and every FolderNode is firstly being used
* as a FileTree root pointer, so it can be used as a parameter of the freeTree func.
*
* The node is freed after its content is freed, so if the node is a FolderNode,
* its children will be freed firstly.
*/
void freeTree(FileTree fileTree) {
    TreeNode *current_root = fileTree.root;
    free(current_root->name);

    if (current_root->type == FILE_NODE) {
        FileContent *file_content = (FileContent *)current_root->content;
        free(file_content->text);
        free(file_content);
    } else {
        FolderContent *dir_content = (FolderContent *)current_root->content;

        if (dir_content) {
            ListNode *current_node = dir_content->children->head, *prev;

            while (current_node) {
                prev = current_node;
                current_node = current_node->next;
                FileTree new_root;
                new_root.root = prev->info;
                freeTree(new_root);
                free(prev);
            }
            free(dir_content->children);
            free(dir_content);
        }
    }

    free(current_root);
}

/*
* This is a recursive function. It is first called from "ls" function.
*
* It is used for printing the children of a given FolderNode.
*
* It is printing in reverse, from the last added node to the first one.
*
* The data from the current node is being pushed on stack as the
* traversing of the parent node's children continues.
*/
void print_ls(ListNode *content_node, int do_not_print_newline) {
    if (!content_node)
        return;

    print_ls(content_node->next, 0);
    TreeNode *info = content_node->info;
    printf("%s", info->name);
    if (do_not_print_newline)
        return;
    printf("\n");
}


/*
* If the current node is a directory, this function will proceed
* to print the elements that this folder contains.
*
* In case of being a text file, this function will print its content.
*/
void ls(TreeNode* currentNode, char* arg) {
    if (currentNode->content == NULL)
        return;

    if (strlen(arg) == 0) {
        FolderContent *directory_content =
        (FolderContent *)currentNode->content;

        ListNode *content_node = directory_content->children->head;

        print_ls(content_node, 1);
    } else {
        FolderContent *directory_content =
        (FolderContent *)currentNode->content;
        ListNode *content_node = directory_content->children->head;

        while (content_node) {
            TreeNode *info = content_node->info;
            if (strcmp(arg, info->name) == 0) {
                if (info->type == FOLDER_NODE) {
                    ls(info, "\0");
                } else {
                    FileContent *file_content = (FileContent *)info->content;
                    printf("%s: %s\n", info->name, file_content->text);
                }
                return;
            }
            content_node = content_node->next;
        }

        printf("ls: cannot access '%s': No such file or directory", arg);
    }
}

/*
* This function prints the path from root to the current directory.
*
* It is traversing the nodes through the parents and it is storing
* their names in an array of char* (dir_names).
*/
void pwd(TreeNode* treeNode) {
    if (treeNode->parent == NULL) {
        printf("root\n");
        return;
    }

    char **dir_names;  // used to store the names of all directories
    TreeNode *current_dir = treeNode;
    int index = 0, size = 0;
    while (current_dir != NULL) {
        if (!size) {
            dir_names = malloc(sizeof(char *));
            size = 1;
        } else if (index == size) {
            size *= 2;
            dir_names = realloc(dir_names, size * sizeof(char *));
        }
        dir_names[index] = malloc(strlen(current_dir->name) + 1);
        memcpy(dir_names[index], current_dir->name,
               strlen(current_dir->name) + 1);
        index++;
        current_dir = current_dir->parent;
    }
    for (int i = index - 1; i >= 0; i--) {
        printf("%s", dir_names[i]);  // printing names to create path
        free(dir_names[i]);

        if (i > 0)
            printf("/");  // delimiter for path
    }
    free(dir_names);
}

/*
* A function used to change path from the current node, depending on the
* argument.
*
* The char* path is splitted by "/" delimiter, using strtok.
* The children are being traversed by verifying the corespondency
* between their names and the current token.
*
* This function may get more options (1, 2, 3).
* Option 1 -> main functionality on *cd* command;
* Option 2 -> called in *cp* for verifying the destination node;
*          -> also called in *mv* for both source and destination nodes;
*          -> if path isn't correct, it is going to return NULL;
* Option 3 -> used in *cp* for source node.
*/
TreeNode* cd(TreeNode* currentNode, char* path, int option) {
    TreeNode *initial_copy_current = currentNode, *copy = currentNode;

    char *token = strtok(path, "/");

    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            copy = copy->parent;
        } else {
            FolderContent *content = (FolderContent *)copy->content;
            if (!content) {
                if (option == 1) {
                    printf("cd: no such file or directory: %s", path);
                    return initial_copy_current;
                } else if (option == 2) {
                    return NULL;
                }
            }

            List *my_list = content->children;
            ListNode *current_file = my_list->head;
            while (current_file != NULL) {
                if (strcmp(token, current_file->info->name) == 0 &&
                    current_file->info->type == FOLDER_NODE && option != 3) {
                    copy = current_file->info;
                    break;
                } else if (strcmp(token, current_file->info->name) == 0 &&
                    current_file->info->type != FOLDER_NODE) {
                    if (option == 1) {
                        printf("cd: no such file or directory: %s", path);
                    } else {
                        copy = current_file->info;
                        break;
                    }
                    return initial_copy_current;
                }
                current_file = current_file->next;
            }

            if (current_file == NULL) {
                if (option == 1) {
                    printf("cd: no such file or directory: %s", path);
                } else if (option == 2) {
                    return NULL;
                }
                return initial_copy_current;
            }
        }
        token = strtok(NULL, "/");
    }
    return copy;
}

/*
* This is a recursive function that takes every directory
* and prints its elements in reverse order.
*
* It actually pushes the current data on stack and goes
* forward to the next element, so the data is printed
* in reverse order.
*
* The "distance" variable represents the distance
* between the initial parent node and the current node and
* it is used for assign the number of tabs that needs to be printed. 
*/
void print_tree(ListNode *content_node, int distance,
                int *nr_of_dir, int *nr_of_files) {
    if (!content_node)
        return;

    char *distance_string;
    char *pula = "\t";
    if (!distance) {
        distance_string = "";
    } else {
        distance_string = malloc(TAB_SIZE * distance);
        for (int i = 0; i < distance; i++) {
            memcpy(distance_string + i, "\t", TAB_SIZE);
        }
    }

    print_tree(content_node->next, distance, nr_of_dir, nr_of_files);
    if (content_node->info->type == FOLDER_NODE) {
        (*nr_of_dir)++;
        printf("%s%s\n", distance_string, content_node->info->name);

        FolderContent *dir_content = content_node->info->content;
        if (dir_content == NULL) {
            if (distance)
                free(distance_string);
            return;
        }

        ListNode *current_node = dir_content->children->head;
        print_tree(current_node, distance + 1, nr_of_dir, nr_of_files);
    } else {
        (*nr_of_files)++;
        printf("%s%s\n", distance_string, content_node->info->name);
    }
    if (distance)
        free(distance_string);
}

/*
* This function works somehow like *ls*.
*
* The char* path is splitted by "/" delimiter, using strtok.
* The children are being traversed by verifying the corespondency
* between their names and the current token.
*/
void tree(TreeNode* currentNode, char* arg) {
    int nr_of_dir = 0, nr_of_files = 0;

    if (strlen(arg) == 0) {
        FolderContent *directory_content =
        (FolderContent *)currentNode->content;
        ListNode *content_node = directory_content->children->head;

        print_tree(content_node, 0, &nr_of_dir, &nr_of_files);
    } else {
        char *token = strtok(arg, "/");
        TreeNode *copy = currentNode;

        while (token != NULL) {
            if (strcmp(token, "..") == 0) {
                copy = copy->parent;
            } else {
                FolderContent *content = (FolderContent *)copy->content;
                if (content == NULL) {
                    printf("%s [error opening dir]\n\n0 directories, 0 files\n",
                           arg);
                    return;
                }

                List *my_list = content->children;
                ListNode *current_file = my_list->head;
                while (current_file != NULL) {
                    if (strcmp(token, current_file->info->name) == 0 &&
                        current_file->info->type == FOLDER_NODE) {
                        currentNode = current_file->info;
                        break;
                    } else if (strcmp(token, current_file->info->name) == 0 &&
                        current_file->info->type != FOLDER_NODE) {
                        printf(
                        "%s [error opening dir]\n\n0 directories, 0 files\n",
                        arg);
                        return;
                    }
                    current_file = current_file->next;
                }

                if (current_file == NULL) {
                    printf(
                        "%s [error opening dir]\n\n0 directories, 0 files\n",
                        arg);
                    return;
                }
                copy = current_file->info;
            }
        token = strtok(NULL, "/");
        }
        FolderContent *directory_content = (FolderContent *)copy->content;
        ListNode *content_node = directory_content->children->head;

        print_tree(content_node, 0, &nr_of_dir, &nr_of_files);
    }

    printf("%d directories, %d files\n", nr_of_dir, nr_of_files);
}

/*
* This function creates a directory that is added to the tail
* of the currentNode's children list.
*
* It creates a TreeNode* that is a type of FOLDER_NODE, and its
* void* content pointer is redirecting to FolderContent.
*/
void mkdir(TreeNode* currentNode, char* folderName) {
    ListNode *new_content_node;  // content of new directory

    if (currentNode->content == NULL) {
        FolderContent *directory_content = malloc(sizeof(FolderContent));
        currentNode->content = directory_content;

        List *dir_content_head = malloc(sizeof(List));
        directory_content->children = dir_content_head;

        new_content_node = malloc(sizeof(ListNode));
        dir_content_head->head = new_content_node;
    } else {
        FolderContent *directory_content =
        (FolderContent *)currentNode->content;
        ListNode *content_node = directory_content->children->head;
        ListNode *prev;

        while (content_node != NULL) {
            if (!strcmp(content_node->info->name, folderName)) {
                printf(
                "mkdir: cannot create directory '%s': File exists",
                folderName);
                return;
            }

            prev = content_node;
            content_node = content_node->next;
        }

        new_content_node = malloc(sizeof(ListNode));
        prev->next = new_content_node;
    }

    new_content_node->next = NULL;

    TreeNode *info = malloc(sizeof(TreeNode));

    info->parent = currentNode;
    info->name = malloc(strlen(folderName) + 1);
    memcpy(info->name, folderName, strlen(folderName) + 1);
    info->type = FOLDER_NODE;

    info->content = NULL;
    new_content_node->info = info;
}

/*
* Function that deletes recursively both directories and files.
* If the current node is going to be a FolderNode, then this function
* is going to be called again, to delete this new folder's elements and so on.
*/
void rmrec(TreeNode* currentNode, char* resourceName) {
    FolderContent *content = (FolderContent *)currentNode->content;
    if (!content)
        return;

    List *file_list = content->children;
    // getting the list of files in the directory
    ListNode *current_file = file_list->head;
    ListNode *prev_file = current_file;

    while (current_file != NULL) {
        if (strcmp(current_file->info->name, resourceName) == 0) {
            // found the directory or file to delete
            break;
        }
        prev_file = current_file;
        current_file = current_file->next;
    }

    if (current_file == NULL) {
        printf("rmrec: failed to remove '%s': No such file or directory\n",
               resourceName);
        return;
    }
    // if directory or file was found list of children must change
    // and get the file out of it

    // there are several possible options to check first:
    if (current_file == file_list->head) {
        if (current_file->info->type == FILE_NODE) {
            file_list->head = current_file->next;

            FileContent *file_content = file_list->head->info->content;
            if (file_content)
                free(file_content->text);

            free(current_file->info->name);
            free(current_file->info);
            free(current_file);
            return;
        } else {
            file_list->head = current_file->next;
            FileTree root;
            root.root = current_file->info;
            freeTree(root);
            free(current_file);
            return;
        }
    }

    prev_file->next = current_file->next;
    if (current_file->info->type == FILE_NODE) {
        FileContent *file_content = current_file->info->content;
        if (file_content)
            free(file_content->text);

        free(current_file->info->name);
        free(current_file->info);
        free(current_file);
        return;
    }
    FileTree root;
    root.root = current_file->info;
    freeTree(root);
    free(current_file);
}

/*
* Function that is used to delete a file.
* As a fileis one of its parent's children, the linkings
* between the previous and next children have to be modified.
*/
void rm(TreeNode* currentNode, char* fileName) {
    FolderContent *content = (FolderContent *)currentNode->content;

    List *file_list = content->children;  // list of files in the directory
    ListNode *current_file = file_list->head;
    ListNode *prev_file = current_file;

    while (current_file != NULL) {
        if (strcmp(current_file->info->name, fileName) == 0) {
            // found the file to delete
            break;
        }
        prev_file = current_file;
        current_file = current_file->next;
    }

    if (current_file == NULL) {
        printf("rm: failed to remove '%s': No such file or directory\n",
               fileName);
        return;
    }

    if (current_file->info->type != FILE_NODE) {
        printf("rm: cannot remove '%s': Is a directory\n",
                fileName);
        return;
    }
    // if it is a file to delete and not a directory it will be deleted

    if (current_file == file_list->head) {
        file_list->head = current_file->next;
    } else {
        prev_file->next = current_file->next;
    }

    FileContent *file_content = file_list->head->info->content;
    if (file_content)
        free(file_content->text);

    free(current_file->info->name);
    free(current_file->info);
    free(current_file);
}

/*
* Function that deletes an empty directory.
* As a directory is one of its parent's children, the linkings
* between the previous and next children have to be modified.
*/
void rmdir(TreeNode* currentNode, char* folderName) {
    FolderContent *content = (FolderContent *)currentNode->content;
    List *file_list = content->children;  // list of files in the directory
    ListNode *current_file = file_list->head;
    ListNode *prev_file = current_file;
    while (current_file != NULL) {
        if (strcmp(folderName, current_file->info->name) == 0) {
            // found the directory to delete
            break;
        }
        prev_file = current_file;
        current_file = current_file->next;
    }

    if (current_file == NULL) {
        printf("rmdir: failed to remove '%s': No such file or directory\n",
               folderName);
        return;
    }

    if (current_file->info->type != FOLDER_NODE) {
        printf("rmdir: failed to remove '%s': Not a directory\n",
               folderName);
        return;
    }
    // if it was found and it is a directory it will be deleted

    if (current_file->info->content != NULL) {
        printf("rmdir: failed to remove '%s': Directory not empty\n",
               folderName);
        return;
    }

    if (current_file == file_list->head) {
        file_list->head = current_file->next;
        free(current_file->info->name);
        free(current_file->info);
        free(current_file);
    } else {
        prev_file->next = current_file->next;
        free(current_file->info->name);
        free(current_file->info);
        free(current_file);
    }
}

/*
* This function creates a file that is added to the tail
* of the currentNode's children list.
*
* It creates a TreeNode* that is a type of FILE_NODE, and its
* void* content pointer is redirecting to FileContent.
*/
void touch(TreeNode* currentNode, char* fileName, char* fileContent) {
    ListNode *new_content_node;

    if (currentNode->content == NULL) {
        FolderContent *directory_content = malloc(sizeof(FolderContent));
        currentNode->content = directory_content;

        List *dir_content_head = malloc(sizeof(List));
        directory_content->children = dir_content_head;

        new_content_node = malloc(sizeof(ListNode));
        dir_content_head->head = new_content_node;
    } else {
        FolderContent *directory_content =
        (FolderContent *)currentNode->content;
        ListNode *content_node = directory_content->children->head;
        ListNode *prev;

        while (content_node != NULL) {
            if (!strcmp(content_node->info->name, fileName))
                return;

            prev = content_node;
            content_node = content_node->next;
        }

        new_content_node = malloc(sizeof(ListNode));
        prev->next = new_content_node;
    }

    new_content_node->next = NULL;

    TreeNode *info = malloc(sizeof(TreeNode));

    info->parent = currentNode;
    info->name = malloc(strlen(fileName) + 1);
    memcpy(info->name, fileName, strlen(fileName) + 1);
    info->type = FILE_NODE;

    FileContent *file_node_content = malloc(sizeof(FileContent));
    file_node_content->text = malloc(strlen(fileContent) + 1);
    memcpy(file_node_content->text, fileContent, strlen(fileContent) + 1);

    info->content = file_node_content;
    new_content_node->info = info;
}

/*
* This function is used to copy the effective data from the source node
* to the destination node.
*
* One of the given parameters is represented by dest node, which is used
* for linking nodes in the children list, as the source node may already
* exist and it needs to be updated. There is also a need of determining the
* parent of the new node, which is why there is dest used.
*
* There are 3 options that this function may get:
* Option 1 -> dest represents the last node in the list of children
*          -> it creates a new file at the end if there was not a file with
*             the exact same name
* Option 2 -> dest is ---- CURRENT - NODE ----
*          -> in the given path, there is already a file with the same
*          -> name, so its content just needs to be changed
* Option 3 -> dest is ---- CURRENT - NODE ----
*          -> if the destination directory is empty, it creates a new file
*             and initializes it to be the head
*/
void copy_node(ListNode *dest, TreeNode *source, int option) {
    TreeNode *current;

    if (option == 1) {
        dest->next = malloc(sizeof(ListNode));
        dest->next->next = NULL;
        dest->next->info = malloc(sizeof(TreeNode));
        current = dest->next->info;
        current->parent = dest->info->parent;
    } else {
        current = dest->info;
    }

    if (option == 1 || option == 3) {
        current->name = malloc(strlen(source->name) + 1);
        memcpy(current->name, source->name, strlen(source->name) + 1);
    }
    current->type = source->type;

    current->content = malloc(sizeof(FileContent));
    FileContent *dest_file_cont = current->content;
    FileContent *src_file_cont = source->content;

    dest_file_cont->text = malloc(strlen(src_file_cont->text) + 1);
    memcpy(dest_file_cont->text, src_file_cont->text,
           strlen(src_file_cont->text) + 1);
}

// Freeing the copies of the source and destination paths
static inline void free_copies(char *copy_source, char *copy_dest) {
    free(copy_source);
    free(copy_dest);
}

/*
* Function used to copy files from src to dest.
*
* It is not copying the files through the concept of pointers,
* but is copying raw data.
*/
void cp(TreeNode* currentNode, char* source, char* destination) {
    char *copy_source = malloc(strlen(source) + 1);
    char *copy_dest = malloc(strlen(destination) + 1);
    memcpy(copy_source, source, strlen(source) + 1);
    memcpy(copy_dest, destination, strlen(destination) + 1);

    TreeNode *source_node = cd(currentNode, source, 3);
    if (source_node->type == FOLDER_NODE) {
        printf("cp: -r not specified; omitting directory '%s'", copy_source);
        free_copies(copy_source, copy_dest);
        return;
    }

    TreeNode *dest_node = cd(currentNode, destination, 2);
    if (!dest_node) {
        printf("cp: failed to access '%s': Not a directory", copy_dest);
        free_copies(copy_source, copy_dest);
        return;
    }

    if (dest_node->type == FOLDER_NODE) {
        FolderContent *dir_content = dest_node->content;
        if (!dir_content) {
            dir_content = malloc(sizeof(FolderContent));
            dir_content->children = malloc(sizeof(List));
            dir_content->children->head = malloc(sizeof(ListNode));
            dir_content->children->head->next = NULL;
            dir_content->children->head->info = malloc(sizeof(TreeNode));
            dir_content->children->head->info->type = FILE_NODE;
            copy_node(dir_content->children->head, source_node, 3);
            dest_node->content = dir_content;
            free_copies(copy_source, copy_dest);
            return;
        }
        ListNode *content_node = dir_content->children->head;

        while (content_node->next) {
            if (!strcmp(content_node->info->name, source_node->name)) {
                FileContent *file_content = content_node->info->content;
                free(file_content->text);
                free(content_node->info->content);
                free(content_node->info->name);
                copy_node(content_node, source_node, 2);
                free_copies(copy_source, copy_dest);
                return;
            }

            content_node = content_node->next;
        }
        copy_node(content_node, source_node, 1);
    } else {
        FileContent *file_content = dest_node->content;
        free(file_content->text);
        free(dest_node->content);
        ListNode *tmp = malloc(sizeof(ListNode));
        tmp->info = dest_node;
        copy_node(tmp, source_node, 2);
        free(tmp);
    }
    free_copies(copy_source, copy_dest);
}

/*
* Function that is used when moving a file into another file, so its
* content needs to be updated.
*/
static inline void move_in_file(TreeNode* dest_node, TreeNode *source_node) {
    TreeNode *dest_parent = dest_node->parent;
    FolderContent *dest_content = dest_parent->content;
    ListNode *dest_nodes = dest_content->children->head;

    while (dest_nodes->next) {
        if (!strcmp(dest_nodes->info->name, dest_node->name))
            break;
        dest_nodes = dest_nodes->next;
    }

    FileContent *file_content = dest_node->content;
    free(file_content->text);
    free(dest_node->content);
    free(source_node->name);
    source_node->name = dest_node->name;
    free(dest_node);
    dest_nodes->info = source_node;
}

/*
* If the *cp* function is copying raw data, this *mv* function
* is just working with pointers, as the node that needs to be
* removed from a directory is going to change its parent to
* the destination.
*/
void mv(TreeNode* currentNode, char* source, char* destination) {
    char *copy_source = malloc(strlen(source) + 1);
    char *copy_dest = malloc(strlen(destination) + 1);
    memcpy(copy_source, source, strlen(source) + 1);
    memcpy(copy_dest, destination, strlen(destination) + 1);

    TreeNode *source_node = cd(currentNode, source, 2);
    TreeNode *dest_node = cd(currentNode, destination, 2);

    if (!source_node) {
        printf("mv: failed to access '%s': Not a directory", copy_source);
        free_copies(copy_source, copy_dest);
        return;
    }

    if (!dest_node) {
        printf("mv: failed to access '%s': Not a directory", copy_dest);
        free_copies(copy_source, copy_dest);
        return;
    }

    TreeNode *source_parent = source_node->parent;

    FolderContent *parent_content = source_parent->content;
    ListNode *children = parent_content->children->head, *prev = NULL;

    while (children) {
        if (!strcmp(copy_source + strlen(copy_source) -
            strlen(children->info->name), children->info->name))
            break;
        prev = children;
        children = children->next;
    }

    if (prev == NULL) {
        parent_content->children->head = children->next;
    } else {
        prev->next = children->next;
    }

    children->next = NULL;

    // FILE CASE
    if (dest_node->type == FILE_NODE) {
        move_in_file(dest_node, source_node);
        free(children);
        free_copies(copy_source, copy_dest);
        return;
    }

    // DIRECTORY CASE
    FolderContent *dest_content = dest_node->content;
    if (!dest_content) {
        dest_content = malloc(sizeof(FolderContent));
        dest_content->children = malloc(sizeof(List));
        dest_content->children->head = children;
        dest_node->content = dest_content;
        free_copies(copy_source, copy_dest);
        return;
    }

    ListNode *dest_nodes = dest_content->children->head;

    while (dest_nodes->next)
        dest_nodes = dest_nodes->next;

    dest_nodes->next = children;
    free_copies(copy_source, copy_dest);
}


