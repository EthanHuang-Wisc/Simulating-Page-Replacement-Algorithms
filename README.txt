
Simulating Page Replacement Algorithms
The goal of this assignment is to evaluate several page replacement algorithms. You will use real memory trace data from UNIX systems to test these algorithms. Your assignment is to write a program that reads in the trace data and simulates the page replacement scheduling. You will keep track of various performance statistics and print these out at the completion of the trace file.

#Program structure

    1.sim-clock.c
    this is the main simulation function for clock algorithm

    2.sim-fifo.c
    this is the main simulation function for fifo algorithm

    3.sim-lru.c
    this is the main simulation function for lru algorithm

    4.pagetable.c
    This is the pagetable for the page replacement project

    5.lru.c
    This is lru page replacement algorithm for the page replacement project
    
    6.clock.c
    This is clock page replacement algorithm for the page replacement project
    
    7.fifo.c
    This is fifo page replacement algorithm for the page replacement project

#Running the program
    
    Three modes are supported: 
    1. FIFO - First In First Out Algorithm 
    2. LRU - Least Recently Used Algorithm
    3. CLOCK - Clock Algorithm
    
    type ' $make all ' to generate three types of executors as following  
        1). 537pfsim-fifo 
        2). 537pfsim-lru
        3). 537pfsim-clock
    
    USAGE:    
    e.g. $ ./537pfsim-lru -p 8192 -m 2 tracefile1

P.S
The running time might go 18446744073409552319 ns or more, it's not correct. Please try on more times. Thank you.

#Author
Ethan Huang
