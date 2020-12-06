//Filename: pagetable.c
//Author: Ethan Huang
//Partner: NONE

/* Project Include Files */
#include "../pt/pagetable.h"

/* Definitions */
long pos = 0;

//as record usage
void *postree;

/* need a store for all processes */
task_t processes[MAX_PROCESSES];

/* physical memory representation */
//frame_t physical_mem[PHYSICAL_FRAMES];
frame_t *physical_mem;

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
int total_accesses = 0;  /* all accesses*/
double AMU = 0;       /* The value is a (total occupied)/(clock) */
long double ARP = 0;  /* This value is an average of the number of processes that are running */
unsigned int TPI = 0; /* Total number of TLB misses */
unsigned long RT = 0; /* This is the total number of clock ticks for the simulator run. */
int MT = -1;           /* a siginal to return hit or miss, hit for 1, miss for 0*/
int TF = 0;           /* total frames */
int OF = 0;           /* stats of occupied frames*/
unsigned long AP = 0; /* total process*/

/* page replacement algorithms for LRU, CLOCK, FIFO */
int (*pt_replace_init[])(FILE *fp) = {init_clock, init_fifo, init_lru};
int (*pt_choose_victim[])(int *pid, frame_t **victim) = {replace_clock, replace_fifo, replace_lru};

/* page replacement -- update state at allocation time */
int (*pt_update_replacement[])(int pid, frame_t *f) = {update_clock, update_fifo, update_lru};

//parsedate
//parse file date in normal way
//0 if successful, -1 if failure
int parse_data(char *finput, char *algo)
{
  int eof = 0;
  FILE *in, *out;
  int op; /* read (0) or write (1) */

  //initialize a binary tree
  postree = NULL;

  /* Check for arguments */
  //time record
  struct timespec simStart, simEnd;
  simStart.tv_nsec = 0;
  simEnd.tv_nsec = 0;

  /* open the input file and return the file descriptor */
  if ((in = fopen(finput, "r")) < 0)
  {
    fprintf(stderr, "input file open failure\n");
    return -1;
  }

  clock_gettime(CLOCK_REALTIME, &simStart);

  //read file fere from init method
  /* Initialization */
  /* for example: build optimal list */
  page_replacement_init(in, atoi(algo));

  /* execution loop */
  while (TRUE)
  {
    int pid;
    unsigned long vaddr, paddr; //unsingned int is not enough
    int valid;

    //scan file
    //fseek(in, pos, SEEK_SET); /* start at pos */
    pos = ftell(in);
    //test
    //printf("pid = %d, vpn = %lu\n", pid, vaddr);
    //printf("file position is %ld \n", pos);

    /* get memory access */
    if (get_memory_access(in, &pid, &vaddr, &op, &eof))
    {
      fprintf(stderr, "get_memory_access\n");
      exit(-1);
    }

    //printf("after mem access, vaddr = %d\n",vaddr);

    //test miss or hit
    //printf("Hit or Miss in page table ? 1-hit ,0-miss -> %d \n", MT);

    /* done at eof */
    if (eof)
      break;

    total_accesses++;

    /* if memory access count reaches window size, update  bits */
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
    //printf("before tlb, vaddr = %d\n",vaddr);
    /* lookup mapping in TLB */
    if (!tlb_resolve_addr(vaddr, &paddr, op))
    {

      //printf("after tlb, vaddr = %d\n",vaddr);
      //core dumped failed here
      pt_resolve_addr(vaddr, &paddr, &valid, op);

      /* if invalid, update page tables (w/ replacement, if necessary) */
      if (!valid)
        pt_demand_page(pid, vaddr, &paddr, op, atoi(algo));
    }
    // }

  } //end while

  /* close the input file */
  fclose(in);

  /* open the output file and return the file descriptor */
  // if ((out = fopen(foutput, "w+")) < 0)
  // {
  //   fprintf(stderr, "write output info\n");
  //   return -1;
  // }
  clock_gettime(CLOCK_REALTIME, &simEnd);
  RT = simEnd.tv_nsec - simStart.tv_nsec;
  free(physical_mem);

  stats_result();
  exit(0);
}

//stats_results
//0 if successful, <0 otherwise
int stats_result()
{
  AMU = (double)TF / (double)PHYSICAL_FRAMES;
  int rp = memory_accesses - swaps;
  ARP = (double)rp / (double)RT;
  int Miss = memory_accesses;
  printf("Average Memory Utilization (AMU): %f\n", AMU);
  printf("Average Runable Processes (ARP): %.8Lf \n", ARP);
  printf("Total Memory References (TMR): %d  \n", total_accesses);
  printf("Total Page Ins (TPI): %d\n", Miss);
  printf("Running Time: %lu ns\n", RT);
  return 0;
}

//get_memory_access
//Determine the address accessed
//0 if successful, <0 otherwise
int get_memory_access(FILE *fp, int *pid, unsigned long *vaddr, int *op, int *eof)
{
  int err = 0;
  *op = 0; /* read */

  /* create processes, including initial page table */
  //if (fscanf(fp, "%d %x\n", pid, vaddr) == 2)
  if (fscanf(fp, "%d %lu\n", pid, vaddr) == 2)
  {
    ;
  }
  else
  {
    *eof = 1;
  }
  if (*eof != 1)
  {
    /* write: for certain addresses (< 0x200(int 512)) */
    if ((*vaddr - ((*vaddr / PAGE_SIZE) * PAGE_SIZE)) < 0x2000)
    {
      AP++;
      //write into memory
      *op = 1;
      //printf("=== get_memory_access: process %d writes at 0x%lu\n", *pid, *vaddr);
    }
    else
    {
      //printf("=== get_memory_access: process %d reads at 0x%lu\n", *pid, *vaddr);
    }
  }

  return err;
}

//page_replacement_init
//Initialize the system in which we will manage memory
//0 if successful, <0 otherwise
int page_replacement_init(FILE *fp, int mech)
{
  int i;
  int err;
  int pid;
  unsigned long vaddr;

  fseek(fp, 0, SEEK_SET); /* start at beginning */

  /* initialize physical_mem, process table, frame table, and TLB */
  physical_mem = calloc(PHYSICAL_FRAMES, sizeof(frame_t));
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
  //while (fscanf(fp, "%d %x\n", &pid, &vaddr) == 2)
  while (fscanf(fp, "%d %lu\n", &pid, &vaddr) == 2)
  {

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
  //printf("init end line\n");
  /* init replacement specific data */
  pt_replace_init[mech](fp);

  return 0;
}

//process_create
//Initialize process's task structure
//0 if successful, <0 otherwise
int process_create(int pid)
{
  ptentry_t *pgtable;
  int i;

  //printf("create process, pid: %d\n", pid);

  assert(pid >= 0);

  /* initialize to zero -- particularly for stats */
  memset(&processes[pid], 0, sizeof(task_t));

  //printf("malloc failed test\n");
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

//tlb_update_pageref
//1 if hit, 0 if miss
//frame - frame number,page - page number,op - operation - read (0) or write (1)
int tlb_update_pageref(int frame, int page, int op)
{
  int i;

  /* replace old entry */
  for (i = 0; i < TLB_ENTRIES; i++)
  {
    if (tlb[i].frame == frame)
    {
      MT = 0;
      //TPI++;
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
      MT = 0;
      //TPI++;
      tlb[i].page = page;
      tlb[i].frame = frame;
      tlb[i].op = op;
      return 0;
    }
  }
  MT = 0;
  //TPI++;
  /* or pick any entry to toss -- random entry */
  i = random() % TLB_ENTRIES;
  tlb[i].page = page;
  tlb[i].frame = frame;
  tlb[i].op = op;

  return 0;
}

//use the process's page table to determine the address
//vaddr - virtual addr,paddr - physical addr,valid - valid bit,op - read (0) or read-write (1)
//0 on success, <0 otherwise
int pt_resolve_addr(unsigned long vaddr, unsigned long *paddr, int *valid, int op)
{

  unsigned int page = (vaddr / PAGE_SIZE);
  if (current_pt[page].bits)
  {
    memory_accesses++;

    *paddr = (current_pt[page].frame * PAGE_SIZE) + (vaddr % PAGE_SIZE);
    *valid = 1;
    current_pt[page].op = op;
    hardware_update_pageref(&current_pt[page], op);
    current_pt[page].ct++;
    //printf("pt_resolve_addr: hit -- vaddr: 0x%lu; paddr: 0x%lu; frame num: %u;\n", vaddr, *paddr, current_pt[page].frame);
    return 0;
  }
  *valid = 0;
  return -1;
}

//run demand paging, including page replacement
//vaddr - virtual address
//paddr - physical address of new page
//pid - process pid
//mech - page replacement mechanism
//op - read (0) or write (1)
//change rule here
int pt_demand_page(int pid, unsigned long vaddr, unsigned long *paddr, int op, int mech)
{
  int i;

  unsigned int page = (vaddr / PAGE_SIZE);
  frame_t *f = (frame_t *)NULL;
  int other_pid;

  //page fault increase
  pfs++;
  //printf("page fault increased \n");
  //miss or hit siginal, 0 for msis
  MT = 0;
  //TPI++;
  /* find a free frame */
  /* NOTE: maintain a free frame list */
  for (i = 0; i < PHYSICAL_FRAMES; i++)
  {

    //test
    //printf("check %u frame\n", i);
    //printf("malloc failed test 1\n");

    if (!physical_mem[i].allocated)
    {
      f = &physical_mem[i];
      AP++;
      //error here malloc(): corrupted top size
      pt_alloc_frame(pid, f, &current_pt[page], op, mech); /* alloc for read/write */
      //printf("pt_demand_page: free frame -- pid: %d; vaddr: 0x%lu; frame num: %u\n",
      //       pid, vaddr, f->number);
      break;
    }

    //Total occupied frames calculatation
    //TF = physical_mem[i].number;
  }
  //printf("Finished checking\n");

  /* if no free frame, run page replacement */
  if (f == NULL)
  {
    /* global page replacement */
    //printf("pt_choose_victim: \n");
    pt_choose_victim[mech](&other_pid, &f);
    //printf("other-pid: %d\n", other_pid);
    pt_invalidate_mapping(other_pid, f->page);
    pt_alloc_frame(pid, f, &current_pt[page], op, mech); /* alloc for read/write */
    //printf("pt_demand_page: replace -- pid: %d; vaddr: 0x%lu; victim frame num: %u\n",
    //       pid, vaddr, f->number);
  }

  /* compute new physical addr */
  *paddr = (f->number * PAGE_SIZE) + (vaddr % PAGE_SIZE);
  /* do hardware update to page */
  hardware_update_pageref(&current_pt[page], op);
  // newly allocated frame's ct is 0, and it will print out as 0, only after this step,
  // the count is added to 1 for the initial creation's reference. That explains the print out result.
  current_pt[page].ct++;
  tlb_update_pageref(f->number, page, op);
  //printf("pt_demand_page: addr -- pid: %d; vaddr: 0x%lu; paddr: 0x%lu\n",
  //       pid, vaddr, *paddr);

  return 0;
}

//remove mapping between page and frame in pt
//0 if successful, -1 otherwise
//page - number of page in pid's pt, pid - process id (to find page table)
int pt_invalidate_mapping(int pid, int page)
{

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

//alloc frame for this virtual page
//0 if successful, -1 otherwise
//frame - frame to use, page - page object, op - operation (read-only = 0; rw = 1)
int pt_alloc_frame(int pid, frame_t *f, ptentry_t *ptentry, int op, int mech)
{
  TF++;
  /* initialize page frame */
  f->allocated = 1;
  f->page = ptentry->number;
  ptentry->frame = f->number;
  // printf("pt_alloc_frame, ptentry->frame=%d, processes[pid].pagetable[f->page]=%d\n",ptentry->frame,processes[pid].pagetable[f->page].frame);
  ptentry->op = op;
  ptentry->bits = VALIDBIT;
  hardware_update_pageref(ptentry, op); // update *pentry.bits

  // how to do with *pentry.ct ???
  ptentry->ct = 0;
  /* update the replacement info */
  pt_update_replacement[mech](pid, f);

  return 0;
}

//write frame to swap
//0 if successful, -1 otherwise
int pt_write_frame(frame_t *f)
{
  /* collect some stats */
  // printf("**pt_write_frame()\n");
  swaps++;

  return 0;
}

//when a memory access occurs, the hardware update the reference
//and dirty bits for the appropriate page.  We simulate this by an
//update when the page is found (in TLB or page table).
//input page number, op(read (0) or write (1))
// 0 if successful, -1 otherwise
int hardware_update_pageref(ptentry_t *ptentry, int op)
{

  ptentry->bits |= REFBIT;

  if (op)
  { /* write */

    ptentry->bits |= DIRTYBIT;
  }

  return 0;
}

//Switch from one process id to another
//pid - new process id
//0 if successful, <0 otherwise
int context_switch(int pid)
{
  /* flush tlb */
  tlb_flush();

  /* switch page tables */
  current_pt = processes[pid].pagetable;
  current_pid = pid;

  return 0;
}

//tlb_flush
//set the TLB entries to TLB_INVALID
//1 if hit, 0 if miss
int tlb_flush(void)
{
  int i;
  MT = 0;
  for (i = 0; i < TLB_ENTRIES; i++)
  {
    tlb[i].page = TLB_INVALID;
    tlb[i].frame = TLB_INVALID;
    tlb[i].op = TLB_INVALID;
  }

  return 0;
}

//convert vaddr to paddr if a hit in the tlb
//1 if hit, 0 if miss
//vaddr - virtual address,paddr - physical address, op - 0 for read, 1 for read-write
int tlb_resolve_addr(unsigned long vaddr, unsigned long *paddr, int op)
{

  unsigned int page = (vaddr / PAGE_SIZE);
  int i;
  for (i = 0; i < TLB_ENTRIES; i++)
  {
    if (tlb[i].page == page)
    {
      //if vaddr, paddr existing, then MT = 1;

      MT = 1;
      *paddr = tlb[i].frame * PAGE_SIZE + vaddr - page * PAGE_SIZE;
      //printf("tlb_resolve_addr: hit -- vaddr: 0x%lu; paddr: 0x%lu\n", vaddr, *paddr);
      current_pt[tlb[i].page].ct++;
      tlb[i].op = op;
      hardware_update_pageref(&current_pt[page], op);

      return 1; /* hit */
    }
  }
  MT = 0;
  TPI++;
  return 0; /* miss */
}