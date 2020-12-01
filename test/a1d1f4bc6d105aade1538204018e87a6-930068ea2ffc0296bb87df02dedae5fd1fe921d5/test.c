

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <search.h>
//#include <sys/queue.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>


typedef struct pid_node {
     int pid; // index of pid
     int vpn;
}pid_node;




//Implemented pid node method
// This is the compare function for the binary tree
// Returns -1 if a <  b
// Returns 1  if a >  b
// Returns 0  if a == b
int compare(const void *a, const void *b)
{
    struct pid_node *node_a = (struct pid_node *)a;
    struct pid_node *node_b = (struct pid_node *)b;

    if (node_a->pid < node_b->pid)
        return -1;
    else if (node_a->pid > node_b->pid)
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

    search_node.pid = key;

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
            printf("Found node. key: %d, value: %d\n", node->pid, node->vpn);
        }
    }

    return node;
}

// Delete a node in the binary tree
// If it is not found, do nothing
void delete_pidnode(void **root, int pid)
{
    struct pid_node *node;

    if ((node = find_pidnode(root, pid)) == NULL) {
        // Nothing to delete
    } else {
        tdelete(node, root, compare);
        printf("Deleted node. key: %d, value: %d\n", node->pid, node->vpn);
        // It's important to free the only after deleting it
        free(node);
    }
}


// Free node when destroying the tree
void free_pidnode(void *ptr)
{
    struct pid_node *node = ptr;

    printf("Freeing node during tree destroy. key: %d, value: %d\n",
            node->pid, node->vpn);

    free(node);
}


// Add the node to the binary tree
void add_pidnode(void **root, struct pid_node *node)
{
    void *result;
    struct pid_node *existing;

    if ((result = tsearch(node, root, compare)) == NULL) {
        // Failed to add the node
        exit_with_message("Insufficient memory");
    } else {
        // Check if an node with the same key already existed
        existing = *(struct pid_node **)result;

        if (existing != node) {
            if(debug == T){
                printf("Node with key already exists. ");
                printf("key: %d, value: %d\n", existing->pid, existing->vpn);
            }
           
            free(node);
        } else {
            if(debug == T){
                printf("Added node. key: %d, value: %d\n", node->pid, node->vpn);
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
struct pid_node *make_pidnode( int pid,  int vpn)
{
    struct pid_node *node;

    if ((node = malloc(sizeof(struct pid_node))) == NULL)
        exit_with_message("Failed to malloc");

    node->pid = pid;
    node->vpn = vpn;

    return node;
}


int main(int argc, char **argv){
    
    File *fp;
    unsigned int pid, vaddr;
    fp = fopen(argv[1], "r");
    
    fseek(fp, 0, SEEK_SET); 
    
    while (fscanf(fp, "%d %x\n", &pid, &vaddr) == 2){
    
        printf("line #%d: \"%s\"\n", line_no + 1, line);

    
    }
}



