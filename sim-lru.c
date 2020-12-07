//Filename: sim-lru.c
//Author: Ethan Huang
//Partner: NONE


#include "pt/pagetable.h"
#define USAGE "USAGE: 537pfsim-lru -p pagesize -m physical_memory tracefile\n"
int PAGE_SIZE = 0x1000; //default value
int PHYSICAL_FRAMES = 0x100; // default value
int VIRTUAL_PAGES = 20; //20
char *tracefile = NULL;


/**********************************************************************

    Function    : main
    Description : this is the main function for project #4
    Inputs      : argc - number of command line parameters
                  argv - the text of the arguments
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

//main function
int main(int argc, char **argv)
{
    int opt;
    unsigned int PHYSICAL_MEMORY = 0x400 * 0x400;
    //FILE *tfp = stdin;
    int ret = 0;
    int MEM;
    int t = 0;
    //for LRU
    char *replacement_alg = "2";

    // /* Check for arguments */
    if (argc < 4)
    {
        /* Complain, explain, and exit */
        fprintf(stderr, "missing or bad command line arguments\n");
        fprintf(stderr, USAGE);
        exit(-1);
    }

    //printf("main in \n");
    while ((opt = getopt(argc, argv, "m:p:")) != -1)
    {

        switch (opt)
        {
        case 'p':
            PAGE_SIZE = (int)strtoul(optarg, NULL, 10);
            if(PAGE_SIZE != 0){
                t = 1;
            }else{
                PAGE_SIZE = 0x1000;
            }
            break;
        case 'm':
            MEM = (int)strtoul(optarg, NULL, 10);
            if(MEM != 0) t = 1;
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
    
    //assert by allowance value
    assert(PAGE_SIZE>0);
    assert(PAGE_SIZE<PHYSICAL_MEMORY);
    
    if (tracefile != NULL)
    {
        ret = parse_data(tracefile, replacement_alg);
    }

    return ret;
}
