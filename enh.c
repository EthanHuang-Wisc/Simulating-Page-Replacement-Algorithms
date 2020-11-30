
/**********************************************************************

   File          : cse473-p1-enh.c

   Description   : This is enhanced enh chance page replacement algorithm
                   (see .h for applications)
                   See http://www.cs.cf.ac.uk/Dave/C/node27.html for info

   By            : Trent Jaeger, Yuquan Shan

***********************************************************************/
/**********************************************************************
Copyright (c) 2016 The Pennsylvania State University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of The Pennsylvania State University nor the names of its contributors may be used to endorse or promote products derived from this softwiare without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

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

/* enhanced enh chance list */

typedef struct enh_entry {  
  int pid;
  ptentry_t *ptentry;
  struct enh_entry *next;
  struct enh_entry *prev;
} enh_entry_t;

typedef struct enh {
  enh_entry_t *first;
} enh_t;

enh_t *enh_list;

/**********************************************************************

    Function    : init_enh
    Description : initialize enh-chance list
    Inputs      : fp - input file of data
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int init_enh( FILE *fp )
{
  printf("initiate enh...\n");
  enh_list = (enh_t *)malloc(sizeof(enh_t));
  enh_list->first = NULL;
  return 0;
}

/**********************************************************************

    Function    : print_enh
    Description : print the containers

***********************************************************************/
void print_enh(){
  enh_entry_t *enh_ptr=enh_list->first;
  int first_access=1;
  printf("enh_page_list: ----");
  // while(mfu_ptr->ptentry->frame!=enh_list->first->ptentry->frame||first_access){
   while(enh_ptr!=enh_list->first||first_access){
    first_access=0;
    printf("frame(%d)_refbit=%d\t",enh_ptr->ptentry->frame,enh_ptr->ptentry->bits);
    enh_ptr=enh_ptr->next;
  }
  printf("----\n");
}


void print_enh2(){
  enh_entry_t *enh_ptr=enh_list->first;
  int first_access=1;
  // while(mfu_ptr->ptentry->frame!=enh_list->first->ptentry->frame||first_access){
   while(enh_ptr!=enh_list->first||first_access){
    first_access=0;
    printf("update_enh: entry: page %d: frame: %d\n",enh_ptr->ptentry->number,enh_ptr->ptentry->frame);
    enh_ptr=enh_ptr->next;
  }
}
/**********************************************************************

    Function    : replace_enh
    Description : choose victim based on enhanced enh chance algorithm (four classes)
    Inputs      : pid - process id of victim frame 
                  victim - frame assigned from fifo -- to be replaced
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int replace_enh( int *pid, frame_t **victim )
{
  /* Task #3 */
  // print_enh();
  enh_entry_t *first = enh_list->first;
  printf("replace_enh: new starting point: frame: %d\n",first->ptentry->frame);
  /* return info on victim */
  int first_access=1;
  int found_flag=0;
  while( first!=enh_list->first || first_access )
  {
    if ((first->ptentry->bits&REFBIT)!=REFBIT && (first->ptentry->bits&DIRTYBIT)!=DIRTYBIT)
    {
      *victim = &physical_mem[first->ptentry->frame];
      *pid = first->pid;
      found_flag=1;
      break;
    }
    first_access=0;
    first=first->next;
  }
  if(!found_flag){
      first_access=1;
      while( first!=enh_list->first || first_access )
      {
        if ((first->ptentry->bits&REFBIT)!=REFBIT && (first->ptentry->bits&DIRTYBIT)==DIRTYBIT)
        {
          *victim = &physical_mem[first->ptentry->frame];
          *pid = first->pid;
          found_flag=1;
          break;
        }else{
          first->ptentry->bits-=REFBIT;
        }
        first_access=0;
        first=first->next;
      }
  }
  if(!found_flag){
      first_access=1;
      while( first!=enh_list->first || first_access )
      {
        if ((first->ptentry->bits&REFBIT)!=REFBIT && (first->ptentry->bits&DIRTYBIT)!=DIRTYBIT)
        {
          *victim = &physical_mem[first->ptentry->frame];
          *pid = first->pid;
          found_flag=1;
          break;
        }
        first_access=0;
        first=first->next;
      }
  }
  if(!found_flag){
      first_access=1;
      while( first!=enh_list->first || first_access )
      {
        if ((first->ptentry->bits&REFBIT)!=REFBIT && (first->ptentry->bits&DIRTYBIT)==DIRTYBIT)
        {
          *victim = &physical_mem[first->ptentry->frame];
          *pid = first->pid;
          found_flag=1;
          break;
        }
        first_access=0;
        first=first->next;
      }
  }

  /* remove from list */
  first->prev->next=first->next;
  first->next->prev=first->prev;
  enh_list->first = first->next;
  free( first );
  // printf("After replacement:  ");
  // print_enh();
  // printf("replace_mfu: pid=%d\n",*pid);
  return 0;
}


/**********************************************************************

    Function    : update_enh
    Description : update enhanced enh chance on allocation 
    Inputs      : pid - process id
                  f - frame
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int update_enh( int pid, frame_t *f )
{
  /* Task #3 */
  // printf("update_enh: pid=%d, frame=%d\n",pid,f->number);
  printf("update_enh: -- current enh_list\n");
  /* Task 3 */
  ptentry_t* pid_s_pt=&processes[pid].pagetable[f->page];
  enh_entry_t *list_entry=( enh_entry_t *)malloc(sizeof(enh_entry_t));
  list_entry->ptentry = pid_s_pt;
  list_entry->pid = pid;
  if(enh_list->first==NULL)
  {
      list_entry->prev=list_entry;
      list_entry->next=list_entry;
      enh_list->first=list_entry;
  }else{
      list_entry->prev=enh_list->first->prev;
      list_entry->next=enh_list->first;
      enh_list->first->prev->next=list_entry;
      enh_list->first->prev=list_entry;
  }
  print_enh2();
  return 0; 
}


