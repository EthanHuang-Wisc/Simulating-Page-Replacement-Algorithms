# Simulating Page Replacement Algorithms
# File          : Makefile
# Description   : Build file for CS537 project 4


all : 537pfsim-fifo 537pfsim-lru 537pfsim-clock 537pfsim-mfu 537pfsim-opt

537pfsim-fifo: sim-fifo.o pt/pagetable.o algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o
	gcc -o 537pfsim-fifo sim-fifo.o pt/pagetable.o algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o

537pfsim-lru: sim-lru.o pt/pagetable.o algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o
	gcc -o 537pfsim-lru sim-lru.o pt/pagetable.o algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o

537pfsim-clock: sim-clock.o pt/pagetable.o  algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o
	gcc -o 537pfsim-clock sim-clock.o pt/pagetable.o algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o

537pfsim-mfu: sim-mfu.o pt/pagetable.o  algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o
	gcc -o 537pfsim-mfu sim-mfu.o pt/pagetable.o algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o

537pfsim-opt: sim-opt.o pt/pagetable.o  algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o
	gcc -o 537pfsim-opt sim-opt.o pt/pagetable.o algo/clock.o algo/fifo.o algo/lru.o algo/enh.o algo/mfu.o utils.o

pagetable.o: pt/pagetable.o pt/pagetable.h
	gcc -c pt/pagetable.o

clock.o: algo/clock.c pt/pagetable.h
	gcc -c algo/clock.c

fifo.o: algo/fifo.c pt/pagetable.h
	gcc -c algo/fifo.c

lru.o: algo/lru.c pt/pagetable.h
	gcc -c algo/lru.c        

mfu.o: algo/mfu.c pt/pagetable.h
	gcc -c algo/mfu.c

enh.o: algo/enh.c pt/pagetable.h
	gcc -c algo/enh.c

utils.o: utils.c utils.h
	gcc -c utils.c

sim-fifo.o: sim-fifo.c pt/pagetable.h
	gcc -c sim-fifo.c

sim-lru.o: sim-lru.c pt/pagetable.h
	gcc -c sim-lru.c

sim-clock.o: sim-clock.c pt/pagetable.h
	gcc -c sim-clock.c

sim-mfu.o: sim-mfu.c pt/pagetable.h
	gcc -c sim-mfu.c

sim-opt.o: sim-opt.c pt/pagetable.h
	gcc -c sim-opt.c

clean:
	rm -f *.o *~ *.txt algo/*.o algo/*~ pt/*.o pt/*.~  537pfsim-fifo 537pfsim-lru 537pfsim-clock 537pfsim-opt 537pfsim-mfu
