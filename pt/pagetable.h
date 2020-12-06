//File Name: pagetable.c
//Author: Ethan Huang
//Partner: NONE

#ifndef _PAGETABLE_H_
#define _PAGETABLE_H_
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
#include <time.h>
#include <math.h>
#include <getopt.h>
#endif



#define TRUE             1
#define MAX_PROCESSES    1000
#define TLB_ENTRIES      32
#define WRITE_FRAC       15
#define TLB_INVALID      -2

/* Global value */
extern int PAGE_SIZE;
extern int PHYSICAL_FRAMES;
extern int VIRTUAL_PAGES;
/* bitmasks */
#define VALIDBIT          0x1
#define REFBIT            0x2
#define DIRTYBIT          0x4 

/* constants for display */
#define SWAP_TIME   2000000      /* in ns */


/* page table entry */
typedef struct ptentry {
  int number;
  int frame;
  int bits;    //refrence bits, dirty bits
  int op;
  int ct;   //refered counts
} ptentry_t;


/* physical frame representation */
typedef struct frame {
  int number;
  int allocated;
  int page;
  int op;
} frame_t;


/* TLB entry */
typedef struct tlbentry {
  int page; 
  int frame;
  int op; 
} tlb_t;


/* need a process structure */
typedef struct task {
  int pid;                      /* process id */
  ptentry_t *pagetable;         /* process page table */
  int ct;                       /* memory reference count */
} task_t;


/* need a store for all processes */
task_t processes[MAX_PROCESSES];

//extern frame_t physical_mem[PHYSICAL_FRAMES];
extern frame_t *physical_mem;
extern ptentry_t *current_pt;

/*data parsing method*/
int parse_data(char *finput, char *algo);

/* initialization */
extern int page_replacement_init( FILE *fp, int mech );

/* page table functions */
extern int pt_resolve_addr( unsigned long vaddr, unsigned long *paddr, int *valid, int op );
extern int pt_demand_page( int pid, unsigned long vaddr, unsigned long *paddr, int op, int mech );
extern int pt_write_frame( frame_t *frame );
extern int pt_alloc_frame( int pid, frame_t *f, ptentry_t *ptentry, int op, int mech );
extern int pt_invalidate_mapping( int pid, int page );

/* external functions */
extern int get_memory_access( FILE *fp, int *pid, unsigned long *vaddr, int *op, int *eof );
extern int context_switch( int pid );
extern int hardware_update_pageref( ptentry_t *ptentry, int op );
extern int stats_result();

/* TLB functions */
extern int tlb_resolve_addr( unsigned long vaddr, unsigned long *paddr, int op );
extern int tlb_update_pageref( int frame, int page, int op );
extern int tlb_flush( void );

/* process (task) functions */
extern int process_create( int pid );
extern int process_frames( int pid, int *frames );

/* clock- second.c */
extern int init_clock( FILE *fp );
extern int update_clock( int pid, frame_t *f );
extern int replace_clock( int *pid, frame_t **victim );

/* fifo - fifo.c*/
extern int init_fifo( FILE *fp );
extern int update_fifo( int pid, frame_t *f );
extern int replace_fifo( int *pid, frame_t **victim );

/* lru - lru.c*/
extern int init_lru(FILE *fp);
extern int replace_lru(int *pid, frame_t **victim);
extern int update_lru(int pid, frame_t *f);