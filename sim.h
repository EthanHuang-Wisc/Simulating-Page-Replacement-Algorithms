
#define TRUE             1
#define PAGE_SIZE        0x1000 /* -p option*/
#define VIRTUAL_PAGES    64
#define PHYSICAL_FRAMES  4      /* -m option*/
#define MAX_PROCESSES    100
#define TLB_ENTRIES      16
#define WRITE_FRAC       15
#define TLB_INVALID      -2

/* bitmasks */
#define VALIDBIT          0x1
#define REFBIT            0x2
#define DIRTYBIT          0x4 

/* constants for display */
#define TLB_SEARCH_TIME   20      /* in ns */
#define MEMORY_ACCESS_TIME 1    /* in ns */
#define PF_OVERHEAD        1      /* in ms */
#define SWAP_IN_OVERHEAD   2     /* in ms */
#define SWAP_OUT_OVERHEAD  2     /* in ms */
#define RESTART_OVERHEAD   1      /* in ms */

/* page table entry */
typedef struct ptentry {
  int number;
  int frame;
  int bits;  /* ref, dirty */  //refrence bits
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


extern frame_t physical_mem[PHYSICAL_FRAMES];
extern ptentry_t *current_pt;


/* initialization */
extern int page_replacement_init( FILE *fp, int mech );

/* process (task) functions */
extern int process_create( int pid );
extern int process_frames( int pid, int *frames );

/* TLB functions */
extern int tlb_resolve_addr( unsigned int vaddr, unsigned int *paddr, int op );
extern int tlb_update_pageref( int frame, int page, int op );
extern int tlb_flush( void );

/* page table functions */
extern int pt_resolve_addr( unsigned int vaddr, unsigned int *paddr, int *valid, int op );
extern int pt_demand_page( int pid, unsigned int vaddr, unsigned int *paddr, int op, int mech );
extern int pt_write_frame( frame_t *frame );
extern int pt_alloc_frame( int pid, frame_t *f, ptentry_t *ptentry, int op, int mech );
extern int pt_invalidate_mapping( int pid, int page );

/* external functions */
extern int get_memory_access( FILE *fp, int *pid, unsigned int *vaddr, int *op, int *eof );
extern int context_switch( int pid );
extern int hw_update_pageref( ptentry_t *ptentry, int op );
extern int write_results( FILE *out );
extern int stats_result(FILE *out);


/* enhanced second chance enh.c */
extern int init_enh( FILE *fp );
extern int update_enh( int pid, frame_t *f );
extern int replace_enh( int *pid, frame_t **victim );

/* mfu - mfu.c */
extern int init_mfu( FILE *fp );
extern int update_mfu( int pid, frame_t *f );
extern int replace_mfu( int *pid, frame_t **victim );

/* second - second.c */
extern int init_second( FILE *fp );
extern int update_second( int pid, frame_t *f );
extern int replace_second( int *pid, frame_t **victim );

/* fifo - fifo.c*/
extern int init_fifo( FILE *fp );
extern int update_fifo( int pid, frame_t *f );
extern int replace_fifo( int *pid, frame_t **victim );