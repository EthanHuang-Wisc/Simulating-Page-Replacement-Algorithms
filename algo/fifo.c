//File Name: fifo.c
//Author: Ethan Huang
//Partner: NONE

/* Include Files */
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

/* fifo list */
typedef struct fifo_entry {  
  int pid;
  frame_t *frame;
  struct fifo_entry *next;
} fifo_entry_t;

typedef struct fifo {
  fifo_entry_t *first;
  fifo_entry_t *last;
} fifo_t;

fifo_t *frame_list;

//initialize fifo list
//fp - input file of data
//sucessful if return 0
int init_fifo( FILE *fp )
{
  //frame_list = (fifo_t *)malloc(sizeof(fifo_t)*VIRTUAL_PAGES);
  frame_list = (fifo_t *)malloc(sizeof(fifo_t));
  return 0;
}


//choose victim from fifo list (first in list is oldest)
//pid - process id of victim frame, victim - frame assigned from fifo -- to be replaced
//sucessful if return 0
int replace_fifo( int *pid, frame_t **victim )
{
  fifo_entry_t *first = frame_list->first;

  /* return info on victim */
  *victim = first->frame;
  *pid = first->pid;

  /* remove from list */
  frame_list->first = first->next;
  free( first );

  return 0;
}


//update fifo list on allocation (add entry to end)
//pid - process id, f - frame
//sucessful if return 0
int update_fifo( int pid, frame_t *f )
{
  
  /* make new list entry */
  //fifo_entry_t *list_entry = ( fifo_entry_t *)malloc(sizeof(fifo_entry_t)*VIRTUAL_PAGES);
  fifo_entry_t *list_entry = ( fifo_entry_t *)malloc(sizeof(fifo_entry_t));
  list_entry->frame = f;
  list_entry->pid = pid;
  list_entry->next = NULL;
  

  /* put it at the end of the list (beginning if null) */
  if ( frame_list->first == NULL ) {
    frame_list->first = list_entry;
    frame_list->last = list_entry;
    
  }
  /* or really at end */
  else {
    frame_list->last->next = list_entry;
    frame_list->last = list_entry;
  }

  return 0;  
}


