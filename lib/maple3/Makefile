# ------------------------------------------------------ #
#  lib/Makefile		( NTHU CS MapleBBS Ver 3.00 )	 #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw		 	 #
#  target : Makefile for MapleBBS library routines	 #
#  create : 95/03/29				 	 #
#  update : 95/12/15					 #
# ------------------------------------------------------ #

SRCROOT	= ../..
.include "$(SRCROOT)/wdbbs.mk"

SRC =	file.c  isnot.c  record.c  string.c  attr_lib.c  dl_lib.c

OBJ =	file.o  isnot.o  record.o  string.o  attr_lib.o  dl_lib.o

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c

all:	libdaom3.a

daom3.p: $(SRC)
	$(CPROTO) -o $@ $? -I$(INCLUDE)

libdaom3.a: $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) *.a *.bak *.BAK *~ tags


tags:
	ctags $(SRC) $(INCLUDE)/*.h
