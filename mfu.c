
//mfu.c
//This is most frequently used replacement algorithm
//Ethan Huang

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

typedef struct mfu_entry{  
  int pid;
  ptentry_t *ptentry;
  struct mfu_entry *next;
  struct mfu_entry *prev;
} mfu_entry_t;

typedef struct mfu{
  mfu_entry_t *first;
} mfu_t;

mfu_t *page_list;

/**********************************************************************

    Function    : init_mfu
    Description : initialize mfu list
    Inputs      : fp - input file of data
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int init_mfu( FILE *fp )
{
  printf("initiate mfu...\n");
  page_list = (mfu_t *)malloc(sizeof(mfu_t));
  page_list->first = NULL;
  return 0;
}

/**********************************************************************

    Function    : print_mfu
    Description : print the containers

***********************************************************************/
void print_mfu(){
  mfu_entry_t *mfu_ptr=page_list->first;
  int first_access=1;
  printf("mfu_page_list: ----");
  // while(mfu_ptr->ptentry->frame!=page_list->first->ptentry->frame||first_access){
   while(mfu_ptr!=page_list->first||first_access){
    first_access=0;
    printf("frame(%d)_ct=%d\t",mfu_ptr->ptentry->frame,mfu_ptr->ptentry->ct);
    mfu_ptr=mfu_ptr->next;
  }
  printf("----\n");
}

/**********************************************************************

    Function    : replace_mfu
    Description : choose victim based on mfu algorithm, take the frame 
                  associated the page with the largest count as victim 
    Inputs      : pid - process id of victim frame 
                  victim - frame assigned from fifo -- to be replaced
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int replace_mfu( int *pid, frame_t **victim )
{
  // replace the most frequently used page.
  /* Task 3 */
  if (page_list->first==NULL)
  {
    exit(-1);
  }else{
    /* return info on victim */
    *pid=page_list->first->pid;
    *victim=&physical_mem[page_list->first->ptentry->frame];
    int highest_count=page_list->first->ptentry->ct; // record the use count of the most frequently used frame
    mfu_entry_t *mfu_ptr=page_list->first; // pointer to the list to iterate through it
    mfu_entry_t *victim_ptr=page_list->first; // pointer to the container/mfu_entry corresponds to the victim frame.
    // print_mfu();
    int first_access=1;
    while(mfu_ptr!=page_list->first||first_access){
      printf("replace_mfu: reference counter for page #%d (pid: %d) is %d\n",mfu_ptr->ptentry->number,mfu_ptr->pid,mfu_ptr->ptentry->ct);
      first_access=0;
      if(mfu_ptr->ptentry->ct>highest_count)
      {
        highest_count=mfu_ptr->ptentry->ct;
        *pid=mfu_ptr->pid;
        *victim=&physical_mem[mfu_ptr->ptentry->frame];
        victim_ptr=mfu_ptr;
      }
      mfu_ptr=mfu_ptr->next;
    }
    printf("replace_mfu: choose frame #%d (associated with page #%d, pid: %d) as the victim\n",victim_ptr->ptentry->frame,victim_ptr->ptentry->number,victim_ptr->pid);
    /* remove from list */
    if (victim_ptr==page_list->first && page_list->first->next==page_list->first)
    {
      // if the victim frame's mfu_entry is what page_list->first points to and page_list only has one item. reset page_list->first to NULL.
      page_list->first=NULL;
    }else{
      if(victim_ptr==page_list->first){
      // if the victim frame's mfu_entry is what page_list->first points to and page_list has more than one item. set page_list->first to its next.
        page_list->first=page_list->first->next;
      }
      victim_ptr->prev->next=victim_ptr->next;
      victim_ptr->next->prev=victim_ptr->prev;
    }
    free(victim_ptr);
    // printf("After replacement:  ");
    // print_mfu();
  }
  // printf("replace_mfu: pid=%d\n",*pid);
  return 0;
}



/**********************************************************************

    Function    : update_mfu
    Description : create container for the newly allocated frame (and 
                  associated page), and insert it to the end (with respect
                  to page_list->first) of page list 
    Inputs      : pid - process id
                  f - frame
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int update_mfu( int pid, frame_t *f )
{
  // printf("update_mfu: pid=%d, frame=%d\n",pid,f->number);
  printf("update_mfu: added new mapping: page#%d --- frame#%d\n",f->page,f->number);

  /* Task 3 */
  ptentry_t* pid_s_pt=&processes[pid].pagetable[f->page];
  mfu_entry_t *list_entry=( mfu_entry_t *)malloc(sizeof(mfu_entry_t));
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
  // print_mfu();
  return 0;
}
