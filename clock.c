
//clock.c
//This is second chance page replacement algorithm
//Ethan Huang

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
#include "pagetable.h"

/* Definitions */

/* second chance list */

typedef struct second_entry {  
  int pid;
  ptentry_t *ptentry;
  struct second_entry *next;
  struct second_entry *prev;
} second_entry_t;

typedef struct second {
  second_entry_t *first;
} second_t;

second_t *page_list;

/**********************************************************************

    Function    : init_second
    Description : initialize second-chance list
    Inputs      : fp - input file of data
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int init_second( FILE *fp )
{
  printf("initiate second...\n");
  page_list = (second_t *)malloc(sizeof(second_t));
  page_list->first = NULL;
  return 0;
}


/**********************************************************************

    Function    : print_second
    Description : print the containers

***********************************************************************/
void print_second(){
  second_entry_t *second_ptr=page_list->first;
  int first_access=1;
  printf("clock_page_list: ----");
  // while(mfu_ptr->ptentry->frame!=page_list->first->ptentry->frame||first_access){
   while(second_ptr!=page_list->first||first_access){
    first_access=0;
    printf("frame(%d)_ref_bits=0x%x\t",second_ptr->ptentry->frame,second_ptr->ptentry->bits);
    second_ptr=second_ptr->next;
  }
  printf("----\n");
}

/**********************************************************************

    Function    : replace_second
    Description : choose victim based on second chance algorithm (first with ref == 0)
    Inputs      : pid - process id of victim frame 
                  victim - frame assigned from fifo -- to be replaced
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int replace_second( int *pid, frame_t **victim )
{
  /* Task #3 */
  // print_second();
  second_entry_t *first = page_list->first;

  /* return info on victim */
  while((first->ptentry->bits&REFBIT)==REFBIT)
  {
    printf("replace_clock: look at frame %d bits 0x%d\n",first->ptentry->frame,first->ptentry->bits);
    first->ptentry->bits-=REFBIT;
    first=first->next;
  }
  printf("replace_clock: look at frame %d bits 0x%d\n",first->ptentry->frame,first->ptentry->bits);
  *victim = &physical_mem[first->ptentry->frame];
  *pid = first->pid;

  /* remove from list */
  first->prev->next=first->next;
  first->next->prev=first->prev;
  page_list->first = first->next;
  free( first );
  // printf("After replacement:  ");
  // print_second();
  // printf("replace_mfu: pid=%d\n",*pid);
  return 0;
}


/**********************************************************************

    Function    : update_second
    Description : update second chance on allocation 
    Inputs      : pid - process id
                  f - frame
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int update_second( int pid, frame_t *f )
{
  /* Task #3 */
  // printf("update_second: pid=%d, frame=%d\n",pid,f->number);
  /* Task 3 */
  ptentry_t* pid_s_pt=&processes[pid].pagetable[f->page];
  second_entry_t *list_entry=( second_entry_t *)malloc(sizeof(second_entry_t));
  list_entry->ptentry = pid_s_pt;
  list_entry->pid = pid;
  if(page_list->first==NULL)
  {
      list_entry->prev=list_entry;
      list_entry->next=list_entry;
      page_list->first=list_entry;
  }else{
      list_entry->prev=page_list->first->prev;
      list_entry->next=page_list->first;
      page_list->first->prev->next=list_entry;
      page_list->first->prev=list_entry;
  }
  // print_second();
  return 0;
}


