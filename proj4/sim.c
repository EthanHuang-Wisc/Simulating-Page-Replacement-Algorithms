
/* Project Include Files */
#include "pagetable.h"
#define USAGE "sim -p<Byte> -m<MB> <input.file> <output.file> <replacement.algo> \n"

/* main method */
int main(int argc, char **argv)
{
    int opt;
	unsigned swapsize = 4096;
	FILE *tfp = stdin;
	char *replacement_alg = NULL;
	char *usage = "USAGE: sim -f tracefile -m memorysize -s swapsize -a algorithm\n";

	while ((opt = getopt(argc, argv, "f:m:a:s:")) != -1) {
		switch (opt) {
		case 'f':
			tracefile = optarg;
			break;
		case 'm':
			memsize = (unsigned)strtoul(optarg, NULL, 10);
			break;
		case 'a':
			replacement_alg = optarg;
			break;
		case 'p':
			swapsize = (unsigned)strtoul(optarg, NULL, 10);
			break;
		default:
			fprintf(stderr, "%s", usage);
			exit(1);
		}
	}

    // -p PAGE_SIZE 
    //-m PHYSICAL_FRAMES
    //assert(PAGE_SIZE > PHYSICAL_FRAMES)
    if (argc < 5)
    {
        /* Complain, explain, and exit */
        fprintf(stderr, "missing or bad command line arguments\n");
        fprintf(stderr, USAGE);
        exit(-1);
    }
    parsedata(argv[3], argv[4], argv[5]);
}