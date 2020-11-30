//simulator.c
//Author: Ethan Huang
//Partner: NONE
/**********************************************************************

   File          : cse473-p1.c

   Description   : This is the main file for page replacement project
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
#include "utils.h"

/* Definitions */
#define USAGE "cs537sim <input.file> <output.file> <replacement.mech> \n"
#define NUM_PROCESSES 100

/* need a store for all processes */
task_t processes[MAX_PROCESSES];

/* physical memory representation */
frame_t physical_mem[PHYSICAL_FRAMES];

/* tlb */
tlb_t tlb[TLB_ENTRIES];

/* current pagetable */
ptentry_t *current_pt;
int current_pid = 0;

/* overall stats */
int swaps = 0;           /* swaps to disk */
int invalidates = 0;     /* reassign page w/o swap */
int pfs = 0;             /* all page faults */
int memory_accesses = 0; /* accesses that miss TLB but hit memory */
int total_accesses = 0;  /* all accesses, this is TMR*/

//stats for p4
double AMU = 0; /* The value is a (total occupied)/(clock) */
double ARP = 0; /* This value is an average of the number of processes that are running */
int TMR = 0;    /* A count of the total number of memory references in the trace file */
int TPI = 0;    /* Total number of page faults (resulting in disk transfers into memory). */
double RT = 0;  /* This is the total number of clock ticks for the simulator run. */

/* page replacement algorithms for MFU, CLOCK(Second), enh, FIFO */
int (*pt_replace_init[])(FILE *fp) = {init_mfu, init_second, init_enh, init_fifo};

int (*pt_choose_victim[])(int *pid, frame_t **victim) = {replace_mfu, replace_second, replace_enh, replace_fifo};

/* page replacement -- update state at allocation time */
int (*pt_update_replacement[])(int pid, frame_t *f) = {update_mfu, update_second, update_enh, update_fifo};

/**********************************************************************

    Function    : main
    Description : this is the main function for project #1
    Inputs      : argc - number of command line parameters
                  argv - the text of the arguments
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

/* Functions */
int main(int argc, char **argv)
{
  int eof = 0;
  FILE *in, *out;
  int op; /* read (0) or write (1) */

  /* Check for arguments */
  if (argc < 4)
  {
    /* Complain, explain, and exit */
    fprintf(stderr, "missing or bad command line arguments\n");
    fprintf(stderr, USAGE);
    exit(-1);
  }

  /* open the input file and return the file descriptor */
  if ((in = fopen(argv[1], "r")) < 0)
  {
    fprintf(stderr, "input file open failure\n");
    return -1;
  }

  //read file fere from init method
  /* Initialization */
  /* for example: build optimal list */
  page_replacement_init(in, atoi(argv[3]));

  /* execution loop */
  while (TRUE)
  {
    int pid;
    unsigned int vaddr, paddr;
    int valid;

    //test
    printf("pid = %d, vpn = %d\n",pid,vaddr);
    
    //test miss or hit
    //int t ;

    //printf("Hit or Miss in page table ? 1-hit ,0-miss -> %d \n",t);

    /* get memory access */
    if (get_memory_access(in, &pid, &vaddr, &op, &eof))
    {
      fprintf(stderr, "get_memory_access\n");
      exit(-1);
    }

    /* done at eof */
    if (eof)
      break;

    total_accesses++;

    /* if memory access count reaches window size, update working set bits */
    processes[pid].ct++;

    /* check if need to context switch */
    if ((!current_pid) || (pid != current_pid))
    {
      if (context_switch(pid))
      {
        fprintf(stderr, "context_switch\n");
        exit(-1);
      }
    }

    /* lookup mapping in TLB */
    if (!tlb_resolve_addr(vaddr, &paddr, op))
    {
      pt_resolve_addr(vaddr, &paddr, &valid, op);
      /* if invalid, update page tables (w/ replacement, if necessary) */
      if (!valid)
        pt_demand_page(pid, vaddr, &paddr, op, atoi(argv[3]));
    }
  }
  TMR = total_accesses;
  /* close the input file */
  fclose(in);

  /* open the output file and return the file descriptor */
  if ((out = fopen(argv[2], "w+")) < 0)
  {
    fprintf(stderr, "write output info\n");
    return -1;
  }

  //write_results(out);
  stats_result(out);

  exit(0);
}

/**********************************************************************

    Function    : write_results
    Description : Write the working set history and memory access performance
    Inputs      : out - file pointer of output file
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int write_results(FILE *out)
{
  float tlb_hit_ratio, tlb_miss_ratio, pf_ratio, swap_out_ratio;

  fprintf(out, "++++++++++++++++++++ Effective Memory-Access Time ++++++++++++++++++\n");
  fprintf(out, "Assuming,\n %dns TLB search time and %dns memory access time\n",
          TLB_SEARCH_TIME, MEMORY_ACCESS_TIME);
  tlb_miss_ratio = ((float)memory_accesses / (float)(total_accesses - pfs));
  tlb_hit_ratio = 1.0 - tlb_miss_ratio;
  fprintf(out, "memory accesses: %d; total memory accesses %d (less page faults)\n", memory_accesses, total_accesses - pfs);
  fprintf(out, "TLB hit rate = %f\n", tlb_hit_ratio);
  float emat = tlb_hit_ratio * (TLB_SEARCH_TIME + MEMORY_ACCESS_TIME) + tlb_miss_ratio * (TLB_SEARCH_TIME + 2 * MEMORY_ACCESS_TIME);
  fprintf(out, "Effective memory-access time = %fns\n",
          /* Task #3: ADD THIS COMPUTATION */
          emat);
  //http://stackoverflow.com/questions/18550370/calculate-the-effective-access-time

  fprintf(out, "++++++++++++++++++++ Effective Access Time ++++++++++++++++++\n");
  fprintf(out, "Assuming,\n %dms average page-fault service time (w/o swap out), a %dms average swap out time, and %dns memory access time\n",
          (PF_OVERHEAD + SWAP_IN_OVERHEAD + RESTART_OVERHEAD), SWAP_OUT_OVERHEAD, MEMORY_ACCESS_TIME);
  fprintf(out, "swaps: %d; invalidates: %d; page faults: %d\n",
          swaps, invalidates, pfs);
  pf_ratio = ((float)pfs / (float)total_accesses);
  swap_out_ratio = ((float)swaps / (float)pfs);
  fprintf(out, "Page fault ratio = %f\n", pf_ratio);
  float eat = (1 - pf_ratio) * emat / 1000 + pf_ratio * (PF_OVERHEAD + SWAP_IN_OVERHEAD + RESTART_OVERHEAD + swap_out_ratio * SWAP_OUT_OVERHEAD);
  fprintf(out, "Effective access time = %fms\n",
          /* Task #3: ADD THIS COMPUTATION */
          eat);
  return 0;
}

/**********************************************************************

    Function    : stats_results
    Description : Write the working set history and memory access performance
    Inputs      : out - file pointer of output file
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int stats_result(FILE *out)
{

  fprintf(out, "Average Memory Utilization (AMU): %f \n", AMU);
  fprintf(out, "Average Runable Processes (ARP): %f \n", ARP);
  fprintf(out, "Total Memory References (TMR): %d , total accesses : %d\n", TMR, total_accesses);
  fprintf(out, "Total Page Ins (TPI): %d %d\n", TPI, pfs);
  fprintf(out, "Running Time: %f \n", RT);
  return 0;
}

/**********************************************************************

    Function    : page_replacement_init
    Description : Initialize the system in which we will manage memory
    Inputs      : fp - input file
                  mech - replacement mechanism
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int page_replacement_init(FILE *fp, int mech)
{
  int i;
  int err;
  int pid;
  unsigned int vaddr;
  //void *fileroot;

  fseek(fp, 0, SEEK_SET); /* start at beginning */

  /* initialize process table, frame table, and TLB */
  memset(processes, 0, sizeof(task_t) * MAX_PROCESSES);
  memset(physical_mem, 0, sizeof(frame_t) * PHYSICAL_FRAMES);
  tlb_flush();
  current_pt = 0;

  /* initialize frames with numbers */
  for (i = 0; i < PHYSICAL_FRAMES; i++)
  {
    physical_mem[i].number = i;
  }

  /* create processes, including initial page table */
  while (fscanf(fp, "%d %x\n", &pid, &vaddr) == 2)
  {
    
    //printf("pid is : %d, vpn is : %x \n", pid, vaddr);
    
    //read processor
    //error from this if statement
    if (processes[pid].pagetable == NULL)
    {
      err = process_create(pid);
      if (err)
        return -1;
    }

    
  }

  fseek(fp, 0, SEEK_SET); /* reset at beginning */

  /* init replacement specific data */
  pt_replace_init[mech](fp);

  return 0;
}

/**********************************************************************

    Function    : process_create
    Description : Initialize process's task structure
    Inputs      : pid - id (and index) of process
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int process_create(int pid)
{
  ptentry_t *pgtable;
  int i;

  printf("create process, pid: %d\n", pid);

  assert(pid >= 0);
  //assert( pid < MAX_PROCESSES );

  /* initialize to zero -- particularly for stats */
  memset(&processes[pid], 0, sizeof(task_t));

  /* set process data */
  processes[pid].pid = pid;
  pgtable = (ptentry_t *)malloc(sizeof(ptentry_t) * VIRTUAL_PAGES);

  if (pgtable == 0)
    return -1;

  /* initialize page table */
  memset(pgtable, 0, (sizeof(ptentry_t) * VIRTUAL_PAGES));

  /* assign numbers to pages in page table */
  for (i = 0; i < VIRTUAL_PAGES; i++)
  {
    pgtable[i].number = i;
  }

  /* store process's page table */
  processes[pid].pagetable = pgtable;

  return 0;
}

/**********************************************************************

    Function    : get_memory_access
    Description : Determine the address accessed
    Inputs      : fp - file pointer
                  pid - process id
                  vaddr - address of access
                  eof - are we done?
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int get_memory_access(FILE *fp, int *pid, unsigned int *vaddr, int *op, int *eof)
{
  int err = 0;
  *op = 0; /* read */

  /* create processes, including initial page table */
  if (fscanf(fp, "%d %x\n", pid, vaddr) == 2)
    ;
  else
    *eof = 1;

  if (*eof != 1)
  {
    /* write: for certain addresses (< 0x200(int 512)) */
    if ((*vaddr - ((*vaddr / PAGE_SIZE) * PAGE_SIZE)) < 0x200)
    {
      *op = 1;
      printf("=== get_memory_access: process %d writes at 0x%x\n", *pid, *vaddr);
    }
    else
    {
      printf("=== get_memory_access: process %d reads at 0x%x\n", *pid, *vaddr);
    }
  }

  return err;
}

/**********************************************************************

    Function    : context_switch
    Description : Switch from one process id to another
    Inputs      : pid - new process id
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int context_switch(int pid)
{
  /* flush tlb */
  tlb_flush();

  /* switch page tables */
  current_pt = processes[pid].pagetable;
  current_pid = pid;

  return 0;
}

/**********************************************************************

    Function    : tlb_flush
    Description : set the TLB entries to TLB_INVALID
    Inputs      : none
    Outputs     : 1 if hit, 0 if miss

***********************************************************************/

int tlb_flush(void)
{
  int i;

  for (i = 0; i < TLB_ENTRIES; i++)
  {
    tlb[i].page = TLB_INVALID;
    tlb[i].frame = TLB_INVALID;
    tlb[i].op = TLB_INVALID;
  }

  return 0;
}

/**********************************************************************

    Function    : tlb_resolve_addr
    Description : convert vaddr to paddr if a hit in the tlb
    Inputs      : vaddr - virtual address
                  paddr - physical address
                  op - 0 for read, 1 for read-write
    Outputs     : 1 if hit, 0 if miss

***********************************************************************/

/* note: normally, the operations associated with a page are based on the address space
   segments in the ELF binary (read-only, read-write, execute-only).  Assume that this is
   already done */

int tlb_resolve_addr(unsigned int vaddr, unsigned int *paddr, int op)
{

  /* Task #2 */
  unsigned int page = (vaddr / PAGE_SIZE);
  int i;
  for (i = 0; i < TLB_ENTRIES; i++)
  {
    if (tlb[i].page == page)
    {
      *paddr = tlb[i].frame * PAGE_SIZE + vaddr - page * PAGE_SIZE;
      printf("tlb_resolve_addr: hit -- vaddr: 0x%x; paddr: 0x%x\n", vaddr, *paddr);
      current_pt[tlb[i].page].ct++;
      tlb[i].op = op;
      hw_update_pageref(&current_pt[page], op);
      return 1;
    }
  }
  return 0; /* miss */
}

/**********************************************************************

    Function    : tlb_update_pageref
    Description : associate page and frame in TLB
    Inputs      : frame - frame number
                  page - page number
                  op - operation - read (0) or write (1)
    Outputs     : 1 if hit, 0 if miss

***********************************************************************/

int tlb_update_pageref(int frame, int page, int op)
{
  int i;

  /* replace old entry */
  for (i = 0; i < TLB_ENTRIES; i++)
  {
    if (tlb[i].frame == frame)
    {
      tlb[i].page = page;
      tlb[i].op = op;
      return 0;
    }
  }

  /* or add anywhere in tlb */
  for (i = 0; i < TLB_ENTRIES; i++)
  {
    if (tlb[i].page == TLB_INVALID)
    {
      tlb[i].page = page;
      tlb[i].frame = frame;
      tlb[i].op = op;
      return 0;
    }
  }

  /* or pick any entry to toss -- random entry */
  i = random() % TLB_ENTRIES;
  tlb[i].page = page;
  tlb[i].frame = frame;
  tlb[i].op = op;

  return 0;
}

/**********************************************************************

    Function    : pt_resolve_addr
    Description : use the process's page table to determine the address
    Inputs      : vaddr - virtual addr
                  paddr - physical addr
                  valid - valid bit
                  op - read (0) or read-write (1)
    Outputs     : 0 on success, <0 otherwise

***********************************************************************/

int pt_resolve_addr(unsigned int vaddr, unsigned int *paddr, int *valid, int op)
{
  /* Task #2 */
  unsigned int page = (vaddr / PAGE_SIZE);
  if (current_pt[page].bits)
  {
    memory_accesses++;
    *paddr = (current_pt[page].frame * PAGE_SIZE) + (vaddr % PAGE_SIZE);
    *valid = 1;
    current_pt[page].op = op;
    hw_update_pageref(&current_pt[page], op);
    current_pt[page].ct++;
    printf("pt_resolve_addr: hit -- vaddr: 0x%x; paddr: 0x%x; frame num: %d;\n", vaddr, *paddr, current_pt[page].frame);
    return 0;
  }
  *valid = 0;
  return -1;
}

/**********************************************************************

    Function    : pt_demand_page
    Description : run demand paging, including page replacement
    Inputs      : pid - process pid
                  vaddr - virtual address
                  paddr - physical address of new page
                  op - read (0) or write (1)
                  mech - page replacement mechanism
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int pt_demand_page(int pid, unsigned int vaddr, unsigned int *paddr, int op, int mech)
{
  int i;
  unsigned int page = (vaddr / PAGE_SIZE);
  frame_t *f = (frame_t *)NULL;
  int other_pid;

  pfs++;

  /* find a free frame */
  /* NOTE: maintain a free frame list */
  for (i = 0; i < PHYSICAL_FRAMES; i++)
  {
    //test
     printf("check %d frame\n",i);
    if (!physical_mem[i].allocated)
    {
      f = &physical_mem[i];

      pt_alloc_frame(pid, f, &current_pt[page], op, mech); /* alloc for read/write */
      printf("pt_demand_page: free frame -- pid: %d; vaddr: 0x%x; frame num: %d\n",
             pid, vaddr, f->number);
      break;
    }
  }
  // printf("Finished checking\n");

  /* if no free frame, run page replacement */
  if (f == NULL)
  {
    /* global page replacement */
     printf("pt_choose_victim: \n");
    pt_choose_victim[mech](&other_pid, &f);
     printf("other-pid: %d\n", other_pid);
    pt_invalidate_mapping(other_pid, f->page);
    pt_alloc_frame(pid, f, &current_pt[page], op, mech); /* alloc for read/write */
    printf("pt_demand_page: replace -- pid: %d; vaddr: 0x%x; victim frame num: %d\n",
           pid, vaddr, f->number);
  }

  /* compute new physical addr */
  *paddr = (f->number * PAGE_SIZE) + (vaddr % PAGE_SIZE);

  /* do hardware update to page */
  hw_update_pageref(&current_pt[page], op);
  // newly allocated frame's ct is 0, and it will print out as 0, only after this step,
  // the count is added to 1 for the initial creation's reference. That explains the print out result.
  current_pt[page].ct++;
  tlb_update_pageref(f->number, page, op);
  printf("pt_demand_page: addr -- pid: %d; vaddr: 0x%x; paddr: 0x%x\n",
         pid, vaddr, *paddr);

  return 0;
}

/**********************************************************************

    Function    : pt_invalidate_mapping
    Description : remove mapping between page and frame in pt
    Inputs      : pid - process id (to find page table)
                  page - number of page in pid's pt
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int pt_invalidate_mapping(int pid, int page)
{
  /* Task #3 */
  // printf("pt_invalidate_mapping: hit -- pid: %d; frame: %d\n",pid, current_pt[page].frame);
  invalidates++;
  ptentry_t *current_pt = processes[pid].pagetable;
  if ((current_pt[page].bits & DIRTYBIT) == DIRTYBIT)
  {
    pt_write_frame(&physical_mem[current_pt[page].frame]);
  }
  physical_mem[current_pt[page].frame].allocated = 0;
  current_pt[page].bits = 0;
  // .numbers is initialized in process create in page_replacement_init and will not be modifed here.
  current_pt[page].frame = 0;
  current_pt[page].op = 0;
  current_pt[page].ct = 0;

  return 0;
}

/**********************************************************************

    Function    : pt_write_frame
    Description : write frame to swap
    Inputs      : frame - frame to be swapped
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int pt_write_frame(frame_t *f)
{
  /* collect some stats */
  // printf("**pt_write_frame()\n");
  swaps++;

  return 0;
}

/**********************************************************************

    Function    : pt_alloc_frame
    Description : alloc frame for this virtual page
    Inputs      : frame - frame to use
                  page - page object
                  op - operation (read-only = 0; rw = 1)
                  mech - replacement mechanism
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int pt_alloc_frame(int pid, frame_t *f, ptentry_t *ptentry, int op, int mech)
{
  /* Task #3 */
  /* initialize page frame */
  f->allocated = 1;
  f->page = ptentry->number;
  ptentry->frame = f->number;
  // printf("pt_alloc_frame, ptentry->frame=%d, processes[pid].pagetable[f->page]=%d\n",ptentry->frame,processes[pid].pagetable[f->page].frame);
  ptentry->op = op;
  ptentry->bits = VALIDBIT;
  hw_update_pageref(ptentry, op); // update *pentry.bits
  // how to do with *pentry.ct ???
  ptentry->ct = 0;

  /* update the replacement info */
  pt_update_replacement[mech](pid, f);

  return 0;
}

/**********************************************************************

    Function    : hw_update_pageref
    Description : when a memory access occurs, the hardware update the reference
                  and dirty bits for the appropriate page.  We simulate this by an
                  update when the page is found (in TLB or page table).
    Inputs      : page - page number
                  op - read (0) or write (1)
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int hw_update_pageref(ptentry_t *ptentry, int op)
{
  ptentry->bits |= REFBIT;

  if (op)
  { /* write */
    ptentry->bits |= DIRTYBIT;
  }

  return 0;
}

