// file reader

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <search.h>
//#include <sys/queue.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "filereader.h"

//define global fields
VPN_Array va;
typedef enum { F, T } _DEBUG;
_DEBUG debug  = F;

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


//////////////////////
//
//defined count array
void initArray(VPN_Array *a, size_t initialSize)
{
    a->array = malloc(initialSize * sizeof(int));
    a->used = 0;
    a->size = initialSize;
}

void insertArray(VPN_Array *a, int element)
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

void freeArray(VPN_Array *a)
{
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

int findDistinctpId(int *arr, int size)
{
    int numberofPid = 0;
    //int duplicate =0;
    int pindex = 0;
    struct pid_node pidnode;
    void *root;
    root = NULL;

    // Pick all elements one by one
    for (int i = 0; i < size; i++)
    {
        int j = 0;
        for (j = 0; j < i; j++)
        {
            if (arr[i] == arr[j])
            {
                //duplicate number
                //duplicate++;

                break;
            }
        }

        // If not printed earlier, then print it
        if (i == j)
        {  
            //將arr[i] 丟到 一個rbtree裡面,<index, pid>
            add_pidnode(&root, make_pidnode(pindex++,arr[i]));
            printf("pid = %d, \t which index = %d \n", arr[i], pindex);    
            //duplicate = 0;
            numberofPid++;
        }
    }

    //size_t sizen = sizeof(parr)/sizeof(parr[0]);

    return numberofPid;

}


//queue
typedef struct node{

	int data;
	struct node* previous;

}*node_ptr;


node_ptr front = NULL;
node_ptr rear = NULL;

bool is_empty(){

	return (front == NULL);
}

void display_front(){

	if(is_empty()){

		printf("\nThe queue is empty!\n");
		return;
	}

	printf("\n[%d]\n", front->data);
}

void display_rear(){

	if(is_empty()){

		printf("\nThe queue is empty!\n");
		return;
	}
	
	printf("\n[%d]\n", rear->data);
}

bool enqueue(int value){

	node_ptr item = (node_ptr) malloc(sizeof(struct node));

	if (item == NULL)
		return false;

	item->data = value;
	item->previous = NULL;

	if(rear == NULL)
		front = rear = item;
	else{
		rear->previous = item;
		rear = item;
	}

	return true;
}

bool dequeue(){

	if(is_empty()){

		printf("\nThe queue is empty!\n");
		return false;
	}

	node_ptr temp = front;
	front = front->previous;
	free(temp);

	return true;
}

void display(){


	if(is_empty()){

		printf("\nThe queue is empty!\n");
		return;
	}

	node_ptr temp = front;

	printf("\n[front -> ");

	while(temp != NULL){
		printf("[%d]", temp->data);
		temp = temp->previous;
	}

	printf(" <- rear]\n");

}

bool clear(){

	if(is_empty()){

		printf("\nThe queue is empty!\n");
		return false;
	}

	node_ptr current = front;
	node_ptr previous = NULL;

	while(current != NULL){

		previous = current->previous;
		free(current);
		current = previous;
	}

	front = NULL;

	return is_empty();
}



int main(int argc, char **argv)
{
    char *line = NULL;

    //char   *token = NULL;
    int line_no = 0;
    //size_t  cur_pos = 0;
    FILE *f;
    void *root;
    root = NULL;
    int pindex,num_vpn = 0;

    if (argc != 2)
    {
        printf("Wrong argumets. Usage: ReadLines.exe <filename>.\n");
        return 1;
    }

    f = fopen(argv[1], "r");

    if (!f)
    {
        printf("Can not open file: %s. %s", argv[1], strerror(errno));
        return 1;
    }

    
    filetype ft;
    
    initArray(&va, 10);
    
    long int total_process_number;
    while ((line = filereader(f)) != NULL)
    {

        int i = 0;
        int parr[2];
        total_process_number = line_no + 1;
        // printf("line #%d: \"%s\"\n", line_no + 1, line);

        char *token = strtok(line, " ");
        // loop through the string to extract all other tokens
        while (token != NULL)
        {
            parr[i++] = atoi(token);

            token = strtok(NULL, " ");
        }

        ft.pid = parr[0];
        ft.vpn = parr[1];
        // printf("pid is %d \n", ft.pid);
        // printf("page reference(VPN) is %d \n", ft.vpn);
        
        
        
        
        if(ft.vpn != find_pidnode(&root, ft.pid)->vpn){ //if miss
            add_pidnode(&root, make_pidnode(ft.pid,ft.vpn));
            printf("line_no = %d", line_no);
            enqueue(line_no);
        }else{ //if hit
            //if hit then continue to process
            delete_pidnode(&root,ft.pid);
            add_pidnode(&root, make_pidnode(ft.pid,ft.vpn));
        }


        
        //1 pid create 1 page, 
        
        
        
        

        
        //add_pidnode(&root, make_pidnode(pindex++,ft.pid));
        //insertArray(&va, find_pidnode(&root, pindex)->pid);

        //load this number into a tsearch tree

        //pid = atoi(line);

        //printf("pID is %d \n", pid);
        //cur_pos = 0;

        // while ((token = get_next_token (line, "\t", &cur_pos)) != NULL) {
        //     printf ("token \"%s\"\n", token);
        //     free (token);
        //     token = NULL;
        // }

        free(line);
        line = NULL;
        line_no++;
    }
    for(int i = 0; i <= pindex; i++){
        insertArray(&va, find_pidnode(&root, i)->pid);
    }
    
    //va.size = total_process_number;
    //int size = sizeof(va);

    // for (int i; i < total_process_number; i++)
    // {
    //     int a = va.array[i];
    //     printf("pid is %d\n", a);
    // }
    printf("total vpn numbers are : %ld \n", total_process_number);
    int countpids = findDistinctpId(va.array, pindex);
    
    //initialized and create page here

    printf("distinct pids are : %d \n", countpids);


    // Example
    // Array a;
    // int i;
    // initArray(&a, 5);  // initially 5 elements
    // for (i = 0; i < 100; i++)
    // insertArray(&a, i);  // automatically resizes as necessary
    // printf("%d\n", a.array[9]);  // print 10th element
    // printf("%d\n", a.used);  // print number of elements
    // freeArray(&a);

    fclose(f);

    return 0;
}