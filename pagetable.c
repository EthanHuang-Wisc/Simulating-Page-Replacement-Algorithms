//pagetable.c
//Author: Ethan Huang
//Partner: NONE

/* Project Include Files */
#include "pagetable.h"
#include "utils.h"

/* Definitions */
//#define USAGE "cs537sim <input.file> <output.file> <replacement.mech> \n"
#define USAGE  "USAGE: 537fifo  -m pagesize -p physical tracefile\n"
#define NUM_PROCESSES 100

// int PAGE_SIZE = 4096;
// int PHYSICAL_FRAMES = 1024;

// int PAGE_SIZE = 0x1000; //default value
// int PHYSICAL_FRAMES = 0x100; // default value

long pos = 0;
//char *tracefile = NULL;
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
int total_accesses = 0;  /* all accesses, this is TMR*/

//stats for p4
double AMU = 0; /* The value is a (total occupied)/(clock) */
double ARP = 0; /* This value is an average of the number of processes that are running */
int TPI = 0;    /* Total number of misses */
long RT = 0;    /* This is the total number of clock ticks for the simulator run. */
int MT = -1;    /* a siginal to return hit or miss, hit for 1, miss for 0*/
int TF = 0;     /* total frames */
int OF = 0;     /* stats of occupied frames*/
int AP = 0;     /* total process*/

/* page replacement algorithms for MFU, CLOCK(Second), enh, FIFO */
int (*pt_replace_init[])(FILE *fp) = {init_mfu, init_second, init_enh, init_fifo, init_lru};
int (*pt_choose_victim[])(int *pid, frame_t **victim) = {replace_mfu, replace_second, replace_enh, replace_fifo, replace_lru};

/* page replacement -- update state at allocation time */
int (*pt_update_replacement[])(int pid, frame_t *f) = {update_mfu, update_second, update_enh, update_fifo, update_lru};

/**********************************************************************

    Function    : main
    Description : this is the main function for project #1
    Inputs      : argc - number of command line parameters
                  argv - the text of the arguments
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

/* Functions */
// int main(int argc, char **argv)
// {
//   int eof = 0;
//   FILE *in, *out;
//   int op; /* read (0) or write (1) */

//   //initialize a binary tree
//   postree = NULL;

//   /* Check for arguments */
//   if (argc < 4)
//   {
//     /* Complain, explain, and exit */
//     fprintf(stderr, "missing or bad command line arguments\n");
//     fprintf(stderr, USAGE);
//     exit(-1);
//   }

//   /* open the input file and return the file descriptor */
//   if ((in = fopen(argv[1], "r")) < 0)
//   {
//     fprintf(stderr, "input file open failure\n");
//     return -1;
//   }
//   //time record
//   struct timespec simStart, simEnd;
//   clock_gettime(CLOCK_REALTIME, &simStart);

//   //read file fere from init method
//   /* Initialization */
//   /* for example: build optimal list */
//   page_replacement_init(in, atoi(argv[3]));

//   /* execution loop */
//   while (TRUE)
//   {
//     int pid;
//     unsigned int vaddr, paddr;
//     int valid;

//     //scan file
//     //fseek(in, pos, SEEK_SET); /* start at pos */
//     pos = ftell(in);
//     //test
//     printf("pid = %d, vpn = %d\n",pid,vaddr);
//     printf("file position is %ld \n", pos);

//     /* get memory access */
//     if (get_memory_access(in, &pid, &vaddr, &op, &eof))
//     {
//       fprintf(stderr, "get_memory_access\n");
//       exit(-1);
//     }

//     //test miss or hit
//     printf("Hit or Miss in page table ? 1-hit ,0-miss -> %d \n", MT);

//     /* done at eof */
//     if (eof)
//       break;

//     total_accesses++;

//     /* if memory access count reaches window size, update  bits */
//     processes[pid].ct++;

//     /* check if need to context switch */
//     if ((!current_pid) || (pid != current_pid))
//     {
//       if (context_switch(pid))
//       {
//         fprintf(stderr, "context_switch\n");
//         exit(-1);
//       }
//     }

//     /* lookup mapping in TLB */
//     if (!tlb_resolve_addr(vaddr, &paddr, op))
//     {
//       pt_resolve_addr(vaddr, &paddr, &valid, op);
//       /* if invalid, update page tables (w/ replacement, if necessary) */
//       if (!valid)
//         pt_demand_page(pid, vaddr, &paddr, op, atoi(argv[3]));
//     }
//   }//end while

//   //TMR = total_accesses;
//   clock_gettime(CLOCK_REALTIME, &simEnd);
//   /* close the input file */
//   fclose(in);

//   /* open the output file and return the file descriptor */
//   if ((out = fopen(argv[2], "w+")) < 0)
//   {
//     fprintf(stderr, "write output info\n");
//     return -1;
//   }

//   RT = simEnd.tv_nsec - simStart.tv_nsec;
//   free(physical_mem);
//   //write_results(out);
//   stats_result(out);

//   exit(0);
// }

int parsedata(char *finput,  char *algo)
{
  int eof = 0;
  FILE *in, *out;
  int op; /* read (0) or write (1) */

  //initialize a binary tree
  postree = NULL;

  /* Check for arguments */
  //printf("PHYSICAL_FRAMES in pt = %d\n",PHYSICAL_FRAMES);

  /* open the input file and return the file descriptor */
  if ((in = fopen(finput, "r")) < 0)
  {
    fprintf(stderr, "input file open failure\n");
    return -1;
  }
  //time record
  struct timespec simStart, simEnd;
  clock_gettime(CLOCK_REALTIME, &simStart);

  //read file fere from init method
  /* Initialization */
  /* for example: build optimal list */
  page_replacement_init(in, atoi(algo));

  /* execution loop */
  while (TRUE)
  {
    int pid;
    unsigned int vaddr, paddr;
    int valid;

    //scan file
    //fseek(in, pos, SEEK_SET); /* start at pos */
    pos = ftell(in);
    //test
    printf("pid = %d, vpn = %d\n",pid,vaddr);
    printf("file position is %ld \n", pos);

    /* get memory access */
    if (get_memory_access(in, &pid, &vaddr, &op, &eof))
    {
      fprintf(stderr, "get_memory_access\n");
      exit(-1);
    }

    //test miss or hit
    printf("Hit or Miss in page table ? 1-hit ,0-miss -> %d \n", MT);

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

    /* lookup mapping in TLB */
    if (!tlb_resolve_addr(vaddr, &paddr, op))
    {
      pt_resolve_addr(vaddr, &paddr, &valid, op);
      /* if invalid, update page tables (w/ replacement, if necessary) */
      if (!valid)
        pt_demand_page(pid, vaddr, &paddr, op, atoi(algo));
    }
  }//end while

  //TMR = total_accesses;
  clock_gettime(CLOCK_REALTIME, &simEnd);
  /* close the input file */
  fclose(in);

  /* open the output file and return the file descriptor */
  if ((out = fopen("537out.txt", "w+")) < 0)
  {
    fprintf(stderr, "write output info\n");
    return -1;
  }

  RT = abs(simEnd.tv_nsec) - abs(simStart.tv_nsec);
  free(physical_mem);
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

  //p4 stats
  // int o,t,p =0;
  // for(int i = 0 ; i < PHYSICAL_FRAMES ; i++){
  //   TF = physical_mem[i].number;
  //   OF = physical_mem[i].allocated;
  //   AP = physical_mem[i].page;
  //   o = o + OF;
  //   t = t + TF;
  //   p = p + AP;
  // }

  TPI = pfs;
  AMU = (double)OF / (double)TF;
  ARP = (double)AP / RT;
  fprintf(out, "++++++++++++++++++++ Stats for Result ++++++++++++++++++\n");
  fprintf(out, "Average Memory Utilization (AMU): %f, MemAccess: %d ,occupied frames: %d, total frames: %d. \n", AMU, memory_accesses, OF, TF);
  fprintf(out, "Average Runable Processes (ARP): %f \n", ARP);
  fprintf(out, "Total Memory References (TMR): %d, total process: %d  \n", total_accesses, AP);
  fprintf(out, "Total Page Ins (TPI): %d\n", TPI);
  fprintf(out, "Running Time: %ldns\n", RT);
  fprintf(out, "++++++++++++++++++++ END of This Line ++++++++++++++++++\n");
  return 0;
}

//stats_results
//0 if successful, <0 otherwise
int stats_result(FILE *out)
{

  //TPI = pfs;
  AMU = (double)OF / (double)TF;
  ARP = (double)AP / (double)RT;

  fprintf(out, "++++++++++++++++++++ Stats for Result ++++++++++++++++++\n");
  fprintf(out, "Memory Access: %d, Occupied Frames: %d, Total Frames: %d.\n", memory_accesses, OF, TF);
  fprintf(out, "Running processes #:  %d\n", AP);
  fprintf(out, "Average Memory Utilization (AMU): %f\n", AMU);
  fprintf(out, "Average Runable Processes (ARP): %.10f \n", ARP);
  fprintf(out, "Total Memory References (TMR): %d  \n", total_accesses);
  fprintf(out, "Total Page Ins (TPI): %d\n", TPI);
  fprintf(out, "Running Time: %ldns\n", RT);
  fprintf(out, "++++++++++++++++++++ END of This Line ++++++++++++++++++\n");
  return 0;
}

/**********************************************************************

    Function    : page_replacement_init
    Description : Initialize the system in which we will manage memory
    Inputs      : fp - input file
                  mech - replacement mechanism
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

//page_replacement_init
//Initialize the system in which we will manage memory
//0 if successful, <0 otherwise
int page_replacement_init(FILE *fp, int mech)
{
  int i;
  int err;
  int pid;
  unsigned int vaddr;

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
  while (fscanf(fp, "%d %x\n", &pid, &vaddr) == 2)
  {

    //printf("pid is : %d, vpn is : %x \n", pid, vaddr);
    //if(MT==0)
    //pos = ftell(fp);

    //
    //add_pidnode(&postree, make_node(pid, pos));
    //printf("file position is %ld \n", pos);

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
  printf("init end line\n");
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

//get_memory_access
//Determine the address accessed
//0 if successful, <0 otherwise
int get_memory_access(FILE *fp, int *pid, unsigned int *vaddr, int *op, int *eof)
{
  int err = 0;
  *op = 0; /* read */

  /* create processes, including initial page table */
  if (fscanf(fp, "%d %x\n", pid, vaddr) == 2)
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
    if ((*vaddr - ((*vaddr / PAGE_SIZE) * PAGE_SIZE)) < 0x1000)
    {

      //write into memory
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

//context_switch
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

  unsigned int page = (vaddr / PAGE_SIZE);
  int i;
  for (i = 0; i < TLB_ENTRIES; i++)
  {
    if (tlb[i].page == page)
    {
      //if vaddr, paddr existing, then MT = 1;

      MT = 1;
      *paddr = tlb[i].frame * PAGE_SIZE + vaddr - page * PAGE_SIZE;
      printf("tlb_resolve_addr: hit -- vaddr: 0x%x; paddr: 0x%x\n", vaddr, *paddr);
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

/**********************************************************************

    Function    : tlb_update_pageref
    Description : associate page and frame in TLB
    Inputs      : frame - frame number
                  page - page number
                  op - operation - read (0) or write (1)
    Outputs     : 1 if hit, 0 if miss

***********************************************************************/

//
int tlb_update_pageref(int frame, int page, int op)
{
  int i;

  /* replace old entry */
  for (i = 0; i < TLB_ENTRIES; i++)
  {
    if (tlb[i].frame == frame)
    {
      MT = 0;
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
      tlb[i].page = page;
      tlb[i].frame = frame;
      tlb[i].op = op;
      return 0;
    }
  }
  MT = 0;
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
  
  unsigned int page = (vaddr / PAGE_SIZE);
  if (current_pt[page].bits)
  {
    memory_accesses++;
    *paddr = (current_pt[page].frame * PAGE_SIZE) + (vaddr % PAGE_SIZE);
    *valid = 1;
    current_pt[page].op = op;
    hardware_update_pageref(&current_pt[page], op);
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

  //page fault increase
  pfs++;

  //miss or hit siginal, 0 for msis
  MT = 0;

  /* find a free frame */
  /* NOTE: maintain a free frame list */
  for (i = 0; i < PHYSICAL_FRAMES; i++)
  {

    //test
    printf("check %d frame\n", i);
    if (!physical_mem[i].allocated)
    {
      f = &physical_mem[i];

      pt_alloc_frame(pid, f, &current_pt[page], op, mech); /* alloc for read/write */
      printf("pt_demand_page: free frame -- pid: %d; vaddr: 0x%x; frame num: %d\n",
             pid, vaddr, f->number);
      break;
    }

    //Total frames calculate
    TF = TF + physical_mem[i].number;
    OF = OF + physical_mem[i].allocated;
    AP = AP + physical_mem[i].page;
  }
  printf("Finished checking\n");

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
  hardware_update_pageref(&current_pt[page], op);
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
//pt_write_frame
//write frame to swap
//0 if successful, -1 otherwise
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
//pt_alloc_frame
//alloc frame for this virtual page
//0 if successful, -1 otherwise
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
  hardware_update_pageref(ptentry, op); // update *pentry.bits
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

int hardware_update_pageref(ptentry_t *ptentry, int op)
{
  ptentry->bits |= REFBIT;

  if (op)
  { /* write */
    ptentry->bits |= DIRTYBIT;
  }

  return 0;
}
