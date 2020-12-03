#
# File          : Makefile
# Description   : Build file for CS537 project 4


# Environment Setup
LIBDIRS=-L. 
INCLUDES=-I.
CC=gcc 
CFLAGS=-c $(INCLUDES) -g -Wall
LINK=gcc -g
LDFLAGS=$(LIBDIRS)
AR=ar rc
RANLIB=ranlib

# Suffix rules
.c.o :
	${CC} ${CFLAGS} $< -o $@

#
# Setup builds

PT-TARGETS=sim
#CSE473LIB=
#CSE473LIBOBJS=

# proj lib
#LIBS=


# Project Protections

sim : $(PT-TARGETS)

sim :  sim.o pagetable.o mfu.o clock.o enh.o fifo.o lru.o utils.o
	$(LINK) $(LDFLAGS) sim.o pagetable.o mfu.o clock.o enh.o fifo.o lru.o utils.o -o $@

#lib$(CSE473LIB).a : $(CSE473LIBOBJS)
#	$(AR) $@ $(CSE473LIBOBJS)
#	$(RANLIB) $@

clean:
	rm -f *.o *~ $(TARGETS).a $(PT-TARGETS)
	#rm -f *.o *~ $(TARGETS) $(LIBOBJS) lib$(CSE473LIB).a 


