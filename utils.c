#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <search.h>
#include <sys/queue.h>
#include <limits.h>
#include "utils.h"

//define global fields
typedef enum { F, T } _DEBUG;
_DEBUG debug  = F;


//file reader methods
char *filereader(FILE *file)
{
    const int BUFFER_LENGTH = 4096;
    char buffer[BUFFER_LENGTH];
    char *result = 0;
    int length = 0;
    int len = 0;
    size_t result_len = 0;

    while (!feof(file))
    {
        if (!fgets(buffer, BUFFER_LENGTH, file))
        {
            return result;
        }

        len = strlen(buffer);

        length += len;
        char *tmp = (char *)malloc((length + 1) * sizeof(char));

        if (!tmp)
        {
            printf("error while trying to allocate %ld bytes. \n",
                   (length + 1) * sizeof(char));
            return result;
        }

        tmp[0] = '\0';

        if (result)
        {
            strcpy(tmp, result);
            free(result);
            result = tmp;
        }
        else
        {
            result = tmp;
        }

        strcat(result, buffer);

        if (strstr(buffer, "\n"))
            break;
    }

    if (result)
    {
        result_len = strlen(result);

        if (result_len > 1 && result[result_len - 2] == '\r')
        {
            result[result_len - 2] = '\0';
        }
        else if (result_len > 0)
        {
            if (result[result_len - 1] == '\n' || result[result_len - 1] == '\r')
            {
                result[result_len - 1] = '\0';
            }
        }
    }

    return result;
}

char *get_next_token(const char *line,
                     const char *delimiters,
                     size_t *current_position)
{
    char *token = NULL;
    const char *start_token = NULL;
    const char *end_token = NULL;
    size_t len = 0;

    if (!line)
        return token;

    if (*current_position >= strlen(line))
        return token;

    if (*current_position > 0)
    {
        start_token = line + *current_position;
    }
    else
    {
        start_token = line;
    }

    if (!delimiters)
    {
        token = (char *)malloc(strlen(start_token) * sizeof(char));
        strcpy(token, start_token);
        *current_position += strlen(token) + 1;
        return token;
    }

    end_token = strpbrk(start_token, delimiters);

    if (!end_token)
    {
        token = (char *)malloc(strlen(start_token) * sizeof(char));
        strcpy(token, start_token);
        *current_position += strlen(token) + 1;
        return token;
    }
    else
    {
        len = end_token - start_token;
        token = (char *)malloc((len + 1) * sizeof(char));
        strncpy(token, start_token, len);
        token[len] = '\0';
        *current_position += strlen(token) + 1;
    }

    return token;
}





/* pid in tree nodes */

//Implemented pid in three nodes method
// This is the compare function for the binary tree
// Returns -1 if a <  b
// Returns 1  if a >  b
// Returns 0  if a == b
int compare(const void *a, const void *b)
{
    struct pid_node *node_a = (struct pid_node *)a;
    struct pid_node *node_b = (struct pid_node *)b;

    if (node_a->key < node_b->key)
        return -1;
    else if (node_a->key > node_b->key)
        return 1;
    else
        return 0;
}


// Find a node in the binary tree
// Returns NULL if no node is found
struct pid_node *find_pidnode(void **root, int key)
{
    void *result;
    struct pid_node *node;
    struct pid_node search_node;

    search_node.key = key;

    if ((result = tfind(&search_node, root, compare)) == NULL) {
        // No node found
        if(debug == T){
            printf("No node found. key: %d\n", key);
        }
        
        node = NULL;
    } else {
        // Node found
        node = *(struct pid_node**)result;
        if(debug == T){
            printf("Found node. key: %d, value: %d\n", node->key, node->pid);
        }
    }

    return node;
}

// Delete a node in the binary tree
// If it is not found, do nothing
void delete_pidnode(void **root, int key)
{
    struct pid_node *node;

    if ((node = find_pidnode(root, key)) == NULL) {
        // Nothing to delete
    } else {
        tdelete(node, root, compare);
        printf("Deleted node. key: %d, value: %d\n", node->key, node->pid);
        // It's important to free the only after deleting it
        free(node);
    }
}


// Free node when destroying the tree
void free_pidnode(void *ptr)
{
    struct pid_node *node = ptr;

    printf("Freeing node during tree destroy. key: %d, value: %d\n",
            node->key, node->pid);

    free(node);
}

//reorder file here, if miss
// Add the node to the binary tree
void add_pidnode(void **root, struct pid_node *node)
{
    void *result;
    struct pid_node *existing;// if miss, keep the vpn in the node,

    if ((result = tsearch(node, root, compare)) == NULL) {
        // Failed to add the node
        exit_with_message("Insufficient memory");
    } else {
        // Check if an node with the same key already existed
        existing = *(struct pid_node **)result;

        if (existing != node) {
            if(debug == T){
                printf("Node with key already exists. ");
                printf("key: %d, value: %d\n", existing->key, existing->pid);
            }
           
            free(node);
        } else {
            if(debug == T){
                printf("Added node. key: %d, value: %d\n", node->key, node->pid);
            }
            
        }
    }
}



// This function prints an error message and exits
void exit_with_message(char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

// Make node for binary tree
struct pid_node *make_pidnode( int key,  int pid)
{
    struct pid_node *node;

    if ((node = malloc(sizeof(struct pid_node))) == NULL)
        exit_with_message("Failed to malloc");

    node->key = key;
    node->pid = pid;

    return node;
}




/* dynamic array */
//dynamic array methods
void initArray(DArray *a, size_t initialSize)
{
    a->array = malloc(initialSize * sizeof(int));
    a->used = 0;
    a->size = initialSize;
}

void insertArray(DArray *a, int element)
{
    // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
    // Therefore a->used can go up to a->size
    if (a->used == a->size)
    {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(int));
    }
    a->array[a->used++] = element;
}

void freeArray(DArray *a)
{
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}




