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

sim : sim.o mfu.o second.o enh.o fifo.o utils.o
	$(LINK) $(LDFLAGS) sim.o mfu.o second.o enh.o fifo.o utils.o -o $@

#lib$(CSE473LIB).a : $(CSE473LIBOBJS)
#	$(AR) $@ $(CSE473LIBOBJS)
#	$(RANLIB) $@

clean:
	rm -f *.o *~ $(TARGETS).a
	#rm -f *.o *~ $(TARGETS) $(LIBOBJS) lib$(CSE473LIB).a 

BASENAME=p4
UWID=938538712
tar: 
	tar cvfz $(UWID).tgz -C ..\
	    $(BASENAME)/Makefile \
	    $(BASENAME)/sim.c \
	    $(BASENAME)/sim.h \
	    $(BASENAME)/mfu.c \
	    $(BASENAME)/second.c \
	    $(BASENAME)/enh.c \
		$(BASENAME)/fifo.c \
	    $(BASENAME)/input.txt \
	    $(BASENAME)/project1-demo 
