
#include "pagetable.h"
#define USAGE "USAGE: 537fifo  -m pagesize -p physical_memory tracefile\n"
int PAGE_SIZE = 0x1000; //default value
int PHYSICAL_FRAMES = 0x100; // default value
char *tracefile = NULL;
//main test
int main(int argc, char **argv)
{
    int opt;
    unsigned int PHYSICAL_MEMORY = 0x400 * 0x400;
    //FILE *tfp = stdin;
    int ret = 0;
    int MEM;
    int t = 0;
    
    //for FIFO
    char *replacement_alg = "3";

    // /* Check for arguments */
    if (argc < 4)
    {
        /* Complain, explain, and exit */
        fprintf(stderr, "missing or bad command line arguments\n");
        fprintf(stderr, USAGE);
        exit(-1);
    }

    printf("main in \n");
    while ((opt = getopt(argc, argv, "m:p:")) != -1)
    {

        switch (opt)
        {
        case 'm':
            PAGE_SIZE = (int)strtoul(optarg, NULL, 10);
            t = 1;
            break;
        case 'p':
            MEM = (int)strtoul(optarg, NULL, 10);
            t = 1;
            break;
        default:
            fprintf(stderr, "missing or bad command line arguments\n");
            fprintf(stderr, "%s", USAGE);
            break;
        }
    }

    if (MEM != 0)
        PHYSICAL_MEMORY = MEM * 0x400 * 0x400;
    if (t == 1)
        PHYSICAL_FRAMES = (abs)(PHYSICAL_MEMORY / PAGE_SIZE);

    tracefile = argv[argc-1];
    printf("tracefile = %s\n", tracefile);
    printf("Physical Memory = %d, PAGE_SIZE = %d \n", PHYSICAL_MEMORY, PAGE_SIZE);

    PHYSICAL_FRAMES =  PHYSICAL_MEMORY/ PAGE_SIZE;
    printf("PHYSICAL_FRAMES = %d\n",  PHYSICAL_FRAMES);
    assert(PAGE_SIZE>PHYSICAL_MEMORY);
    if (tracefile != NULL)
    {

        ret = parsedata(tracefile,replacement_alg);
    }

    

    return ret;
}


