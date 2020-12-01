
#ifndef _FILEREADER_H_
#define _FILEREADER_H_

#endif



typedef struct pid_node {
     int pid; // index of pid
     int vpn;
}pid_node;

typedef struct 
{
     int pid;
     int vpn;
}filetype;

typedef struct
{
    int *array;
    size_t used;
    size_t size;
} VPN_Array;

void initArray(VPN_Array *a, size_t initialSize);
void insertArray(VPN_Array *a, int element);
void freeArray(VPN_Array *a);
char *filereader(FILE *file);
char *get_next_token(const char *line, const char *delimiters, size_t *current_position);
int findDistinctpId(int *arr, int size);
void delete_pidnode(void **root, int key);
void free_pidnode(void *ptr);
void add_pidnode(void **root, struct pid_node *node);
void exit_with_message(char *message);
struct pid_node *make_node(unsigned int key, unsigned int pid);
int compare(const void *a, const void *b);
//void fileProcessor(FILE *file);