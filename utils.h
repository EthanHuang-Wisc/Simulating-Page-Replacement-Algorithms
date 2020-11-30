
#ifndef _UTILS_H_
#define _UTILS_H_

#endif



typedef struct pid_node {
     int key; // index of pid
     int pid;
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
} DArray;

//filereader methods
char *filereader(FILE *file);
char *get_next_token(const char *line, const char *delimiters, size_t *current_position);

//dynamic array 
void initArray(DArray *a, size_t initialSize);
void insertArray(DArray *a, int element);
void freeArray(DArray *a);

//
int findDistinctpId(int *arr, int size);

//pid in tree nodes methods
void delete_pidnode(void **root, int key);
void free_pidnode(void *ptr);
void add_pidnode(void **root, struct pid_node *node);
void exit_with_message(char *message);
struct pid_node *make_node(unsigned int key, unsigned int pid);
int compare(const void *a, const void *b);
//void fileProcessor(FILE *file);