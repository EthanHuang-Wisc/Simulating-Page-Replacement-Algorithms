
/**********************************************************************

   File          : cs537fifo.c

   Description   : This is FIFO page replacement algorithm

   By            : Ethan Huang

***********************************************************************/
//File Name: 537fifo.c
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
#include "sim.h"

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

/**********************************************************************

    Function    : init_fifo
    Description : initialize fifo list
    Inputs      : fp - input file of data
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int init_fifo( FILE *fp )
{
  frame_list = (fifo_t *)malloc(sizeof(fifo_t));
  
  return 0;
}


/**********************************************************************

    Function    : replace_fifo
    Description : choose victim from fifo list (first in list is oldest)
    Inputs      : pid - process id of victim frame 
                  victim - frame assigned from fifo -- to be replaced
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

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


/**********************************************************************

    Function    : update_fifo
    Description : update fifo list on allocation (add entry to end)
    Inputs      : pid - process id
                  f - frame
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int update_fifo( int pid, frame_t *f )
{
  /* make new list entry */
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


