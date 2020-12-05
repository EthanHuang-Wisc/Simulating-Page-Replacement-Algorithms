# Simulating Page Replacement Algorithms
# File          : Makefile
# Description   : Build file for CS537 project 4

537pfsim-fifo: $(FIFOPT-TARGETS)
537pfsim-lru: $(LRUPT-TARGETS)

all : 537pfsim-fifo 537pfsim-lru 537pfsim-clock

537pfsim-fifo: sim-fifo.o pt/pagetable.o algo/mfu.o algo/clock.o algo/enh.o algo/fifo.o algo/lru.o utils.o
	gcc -o 537pfsim-fifo sim-fifo.o pt/pagetable.o algo/mfu.o algo/clock.o algo/enh.o algo/fifo.o algo/lru.o utils.o

537pfsim-lru: sim-lru.o pt/pagetable.o algo/mfu.o algo/clock.o algo/enh.o algo/fifo.o algo/lru.o utils.o
	gcc -o 537pfsim-lru sim-lru.o pt/pagetable.o algo/mfu.o algo/clock.o algo/enh.o algo/fifo.o algo/lru.o utils.o

537pfsim-clock: sim-clock.o pt/pagetable.o algo/mfu.o algo/clock.o algo/enh.o algo/fifo.o algo/lru.o utils.o
	gcc -o 537pfsim-clock sim-clock.o pt/pagetable.o algo/mfu.o algo/clock.o algo/enh.o algo/fifo.o algo/lru.o utils.o

pagetable.o: pt/pagetable.o pt/pagetable.h
	gcc -c pt/pagetable.o

clock.o: algo/clock.c pt/pagetable.h
	gcc -c algo/clock.c

fifo.o: algo/fifo.c pt/pagetable.h
	gcc -c algo/fifo.c

lru.o: algo/lru.c pt/pagetable.h
	gcc -c algo/lru.c

enh.o: algo/enh.c pt/pagetable.h
	gcc -c algo/enh.c         

utils.o: utils.c utils.h
	gcc -c utils.c

537pfsim-fifo.o: 537pfsim-fifo.c pt/pagetable.h
	gcc -c 537pfsim-fifo.c

537pfsim-lru.o: 537pfsim-lru.c pt/pagetable.h
	gcc -c 537pfsim-lru.c


clean:
	rm -f *.o *~ algo/*.o algo/*~ pt/*.o pt/*.~  537pfsim-fifo 537pfsim-lru 537pfsim-clock