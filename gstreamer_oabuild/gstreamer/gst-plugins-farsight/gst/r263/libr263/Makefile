# Makefile for R263 library functions, a compression library
# to H.263 bitstreams.
# (C) Copyright 1996, Twente University
# All rights reserved. Other copyrights and/or patents may also apply, see
# source code

SRCS =	libr263.c dct.c coder.c quant.c mot_est.c pred.c \
        countbit.c stream.c huffman.c \
	rlib.c \
        Makefile manual.tex README DISCLAIMER
HDRS = 	libr263.h sim.h macros.h huffman.h ctables.h config.h \
        indices.h \
	owntypes.h \
	rlib.h
OBJS =	dct.o coder.o quant.o mot_est.o pred.o  \
        countbit.o stream.o huffman.o libr263.o \
	rlib.o

# use -DLONGISDOUBLEINT if sizeof(long) is 2*sizeof(int) as copy
#                       operations with longs are faster than with ints
#                       (for instance, Alpha processor, but not for i486)
# Use either STANDARDIDCT, FASTIDCT or VERYFASTIDCT for IDCT operations.
# Use either STANDARDDCT or FASTDCT for DCT operations.

FORMAT = -DFASTDCT -DVERYFASTIDCT 

CC = gcc
CFLAGS = -O3 -finline-functions -Winline -funroll-loops -pedantic $(FORMAT) -Wall -Winline -c -DEXTERN_BUF -DRTP_SUPPORT -g

# If you want to compile with -Wall -pedantic -ansi on a Sun, use 
# -DBROKEN_SUN_HEADERS to avoid a lot of compiler warnings

library:        rlib.a libr263.a

help:
		@echo "libr263 package - (C) Copyright 1996, 1997, Roalt Aalmoes"
		@echo "                      and Twente University        "
		@echo "                - Version 0.1"
		@echo "make library: Create libr263.a library and supporting"
		@echo "              rlib.a library"
		@echo " "
		@echo "make man    : Create manual for libr263 in postscript"
		@echo " "
		@echo "make all    : Create both libraries and manual"
		@echo " "
		@echo "make tar    : Create tar file for distribution or backup"
		@echo " "


#library:	rlib.a libr263.a

all: 		rlib.a libr263.a

libr263.o:	libr263.c libr263.h sim.h
coder.o:	coder.c sim.h libr263.h
dct.o:		dct.c sim.h libr263.h 
quant.o:	quant.c sim.h libr263.h
pred.o:		pred.c sim.h libr263.h
mot_est.o:	mot_est.c sim.h config.h libr263.h
stream.o:	stream.c sim.h libr263.h
huffman.o:	huffman.c huffman.h ctables.h 
countbit.o:	huffman.c countbit.c huffman.h sim.h indices.h libr263.h


man:		manual.tex
		latex manual
		dvips manual


clean:		
		rm -f $(OBJS) *~ libr263.a rlib.a manual.ps manual.log manual.aux 

tar:		$(SRCS) $(HDRS)
		tar cvf libr263.tar $(SRCS) $(HDRS)
		gzip libr263.tar

distdir:	
		cp -rf * $(distdir)/.
		cd $(distdir); make distclean

distclean:	clean
		

libr263.a:	$(OBJS)
		ar rc libr263.a $(OBJS)

rlib.a:		rlib.o
		ar rc rlib.a rlib.o

rlib.o: 	rlib.c rlib.h


install:	all

uninstall:
