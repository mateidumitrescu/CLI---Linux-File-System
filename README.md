**Name: Dumitrescu Rares Matei and Avram Cristian Stefan**\
**Group: 311CA**

# Homework Nr. 3 - Simple Linux File System

## Description

This homework is oriented to simulate a Linux file system, by using linked lists and the concept of trees, which is including a list of children, that represents the files and directories that a folder may contain. Practically, this homework gives us an idea about how real life things are working and, even if they are much more complex than we think, it is still pushing us a step forward in understanding the concepts of operating systems and the programming that lays behind.

For different commands that the users could give, there were implemented different functions.
>* **CREATE COMMANDS**
>>* **TOUCH** --> This command creates a text file inside the current directory. It is being added as the final child of its parent node and may contain text or not. Its *void\* content* pointer redirect the program to a *FileContent* structure.
>>* **MKDIR** --> This command creates a folder inside the current directory. It is being added as the final child of its parent node. Its *void\* content* pointer redirect the program to a *FolderContent* structure, that is redirecting to a *List* structure and then to a *ListNode* structure.

>* **PRINTING COMMANDS**
>>* **LS** --> As *ls* comes from *List files and directories*, its main attribution is to print the content of the current directory. This is happening by traversing every single child of this folder. Still, in Linux file system, *ls* is used just for listing the existing files and directories, but the currently implemented *ls* is accepting one more option. If an argument is given and it represents the path to a file, then this *ls* will behave like the command *cat* and will print the text from the given file. If the argument is a directory, then it will act as usual and will print the elements from the given directory. The function *print_ls* is a recursive function that is used for printing the files in a reversed order, from the last added to the first one.
>>* **TREE** --> The *tree* command works a lot like ls command, because of the fact that it is printing every single element from a directory. The only difference is represented by the capability of listing every directory that the current node includes. It was implemented by using a recursive function, *print_tree*, which takes every directory and, for a brief moment, it is designated to be the "parent" node of the following nodes. The number of tabs that are printed before printing a file represents the distance from the main node, that was given as an initial parent.

>* **HANDLING PATHS COMMANDS**
>>* **CD** --> This command takes the path that is given as an argument and traverses every child node until the nearest directory from the path, then the current node actualizes itself. The function accepts more options, as it is also used in the **CP** and **MV** commands, for returning the source and destination nodes. So, for its main purpose, it will be needed the option 1.
>>* **PWD** --> *Print working directory*, a simple command that is traversing through the parent nodes and is printing their names in the following format: dir1/dir2/dir3/dir4.

>* **DELETE COMMANDS**
>>* **RMDIR** --> This command is used for deleting empty directories. The function is firstly verifying if the folder has any files and if that's true, then it proceeds to wipe it out. The memory that has been allocated is freed and the links between its parent's nodes are modified.
>>* **RM** --> This command works exactly like **RMDIR**, but it is only for files, not directories. Another difference may be represented by the fact that it can delete an empty file, with no text in it, in contrast to the *RMDIR* command.
>>* **RMREC** --> RMREC comes from *remove recursively*, so, as said, it is used for removing the every single file or dir that a directory contains. It uses the **freeTree** function, to remove recursively every single node.

>* **HANDLING FILES / DIRECTORIES**
>>* **CP** --> This command is used for copying files from the source to the destination. To access the source and destination nodes, it uses **CD** function with option 3, respectively option 2. This options are used for returning different nodes or messages. For example, if the destination node (option 2) does not represent a correct file or directory, as specified, then it is going to return a NULL pointer, which will trigger the **CP** function to stop. The fundamental concept of this function is not about handling pointers, but about handling memory, as by using *copy_node* function, it just copying the data from source to destination, so if something happens to the source node, it won't affect its copy from destination.
>>* **MV** --> This command may be similar to **CP**, but is not duplicating the source node, it is just changing its parent through the concepts of pointers. So, the source have to be deleted from its initial parent's list of children and it has to be added to destination. Some of the rules that are applied to *CP* function are still valid here.