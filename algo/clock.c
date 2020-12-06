//Filename: clock.c
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

/* clock  list */

typedef struct clock_entry {  
  int pid;
  ptentry_t *ptentry;
  struct clock_entry *next;
  struct clock_entry *prev;
} clock_entry_t;

typedef struct clock {
  clock_entry_t *first;
} clockalgo_t;

clockalgo_t *page_list;


//initialize clock list
//fp - input file of data
// 0 if successful, -1 otherwise
int init_clock( FILE *fp )
{
  //printf("initiate clock...\n");
  page_list = (clockalgo_t *)malloc(sizeof(clockalgo_t)* VIRTUAL_PAGES);
  page_list->first = NULL;
  return 0;
}



//print the containers
void print_clock(){
  clock_entry_t *clock_ptr=page_list->first;
  int first_access=1;
  //printf("clock_page_list: ----");
  // while(mfu_ptr->ptentry->frame!=page_list->first->ptentry->frame||first_access){
   while(clock_ptr!=page_list->first||first_access){
    first_access=0;
    //printf("frame(%d)_ref_bits=0x%u\t",clock_ptr->ptentry->frame,clock_ptr->ptentry->bits);
    clock_ptr=clock_ptr->next;
  }
  //printf("----\n");
}


//choose victim based on clock chance algorithm (first with ref == 0)
//0 if successful, -1 otherwise
int replace_clock( int *pid, frame_t **victim )
{
  
  // print_clock();
  clock_entry_t *first = page_list->first;

  /* return info on victim */
  while((first->ptentry->bits&REFBIT)==REFBIT)
  {
    //printf("replace_clock: look at frame %d bits 0x%d\n",first->ptentry->frame,first->ptentry->bits);
    first->ptentry->bits-=REFBIT;
    first=first->next;
  }
  //printf("replace_clock: look at frame %d bits 0x%d\n",first->ptentry->frame,first->ptentry->bits);
  *victim = &physical_mem[first->ptentry->frame];
  *pid = first->pid;

  /* remove from list */
  first->prev->next=first->next;
  first->next->prev=first->prev;
  page_list->first = first->next;
  free( first );
  // printf("After replacement:  \n");
  // print_clock();
  // printf("replace_mfu: pid=%d\n",*pid);
  return 0;
}



//update clock chance on allocation 
//0 if successful, -1 otherwise
int update_clock( int pid, frame_t *f )
{
  
  // printf("update_clock: pid=%d, frame=%u\n",pid,f->number);
  
  ptentry_t* pid_s_pt=&processes[pid].pagetable[f->page];
  clock_entry_t *list_entry=( clock_entry_t *)malloc(sizeof(clock_entry_t)* VIRTUAL_PAGES);
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
  // print_clock();
  return 0;
}


