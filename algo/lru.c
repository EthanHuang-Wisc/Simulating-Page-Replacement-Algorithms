//Filename: lru.c
//Author: Ethan Huang
//Partner: NONE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <sched.h>

/* Project Include Files */
#include "../pt/pagetable.h"

/* Definitions */
/* lru list */
typedef struct lru_entry
{
    int pid;
    ptentry_t *ptentry;
    struct lru_entry *next;
    struct lru_entry *prev;
} lru_entry_t;

typedef struct lru
{
    lru_entry_t *first;
} lru_t;

lru_t *page_list;


//initialize lru list
//0 if successful, -1 otherwise
int init_lru(FILE *fp)
{
    //printf("initiate lru...\n");
    page_list = (lru_t *)malloc(sizeof(lru_t)*2);
    page_list->first = NULL;
    return 0;
}


//print the containers
void print_lru()
{
    lru_entry_t *lru_ptr = page_list->first;
    int first_access = 1;
    //printf("lru_page_list: ----");
    // while(lru_ptr->ptentry->frame!=page_list->first->ptentry->frame||first_access){
    while (lru_ptr != page_list->first || first_access)
    {
        first_access = 0;
        //printf("frame(%d)_ct=%d\t", lru_ptr->ptentry->frame, lru_ptr->ptentry->ct);
        lru_ptr = lru_ptr->next;
    }
    //printf("----\n");
}


//choose victim based on lru algorithm, take the frame 
// associated the page with the largest count as victim
//0 if successful, -1 otherwise
int replace_lru(int *pid, frame_t **victim)
{
    // replace the least recently used page.
    
    if (page_list->first == NULL)
    {
        exit(-1);
    }
    else
    {
        /* return info on victim */
        *pid = page_list->first->pid;
        *victim = &physical_mem[page_list->first->ptentry->frame];
        //int highest_count = page_list->first->ptentry->ct; // record the use count of the least frequently used frame
        int min_count = page_list->first->ptentry->ct;
        lru_entry_t *lru_ptr = page_list->first;           // pointer to the list to iterate through it
        lru_entry_t *victim_ptr = page_list->first;        // pointer to the container/lru_entry corresponds to the victim frame.
        // print_lru();
        int first_access = 1;
        while (lru_ptr != page_list->first || first_access)
        {
            //printf("replace_lru: reference counter for page #%d (pid: %d) is %d\n", lru_ptr->ptentry->number, lru_ptr->pid, lru_ptr->ptentry->ct);
            first_access = 0;
            if (lru_ptr->ptentry->ct < min_count)
            {
                min_count = lru_ptr->ptentry->ct;
                *pid = lru_ptr->pid;
                *victim = &physical_mem[lru_ptr->ptentry->frame];
                victim_ptr = lru_ptr;
            }
            lru_ptr = lru_ptr->next;
        }
        //printf("replace_lru: choose frame #%d (associated with page #%d, pid: %d) as the victim\n", victim_ptr->ptentry->frame, victim_ptr->ptentry->number, victim_ptr->pid);
        /* remove from list */
        if (victim_ptr == page_list->first && page_list->first->next == page_list->first)
        {
            // if the victim frame's lru_entry is what page_list->first points to and page_list only has one item. reset page_list->first to NULL.
            page_list->first = NULL;
        }
        else
        {
            if (victim_ptr == page_list->first)
            {
                // if the victim frame's lru_entry is what page_list->first points to and page_list has more than one item. set page_list->first to its next.
                page_list->first = page_list->first->next;
            }
            victim_ptr->prev->next = victim_ptr->next;
            victim_ptr->next->prev = victim_ptr->prev;
        }
        free(victim_ptr);
        // printf("After replacement:  \n");
        // print_lru();
    }
    // printf("replace_lru: pid=%d\n",*pid);
    return 0;
}


//create container for the newly allocated frame (and 
//associated page), and insert it to the end (with respect to page_list->first) of page list
//0 if successful, -1 otherwise
int update_lru(int pid, frame_t *f)
{
    // printf("update_lru: pid=%d, frame=%d\n",pid,f->number);
    //printf("update_lru: added new mapping: page#%d --- frame#%u\n", f->page, f->number);

    
    ptentry_t *pid_s_pt = &processes[pid].pagetable[f->page];
    
    lru_entry_t *list_entry = (lru_entry_t *)malloc(sizeof(lru_entry_t)*VIRTUAL_PAGES);
    list_entry->ptentry = pid_s_pt;
    list_entry->pid = pid;
    if (page_list->first == NULL)
    {
        list_entry->prev = list_entry;
        list_entry->next = list_entry;
        page_list->first = list_entry;
    }
    else
    {
        list_entry->prev = page_list->first->prev;
        list_entry->next = page_list->first;
        page_list->first->prev->next = list_entry;
        page_list->first->prev = list_entry;
    }
    // print_lru();
    return 0;
}
