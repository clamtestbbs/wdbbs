# ------------------------------------------------------ #
#  lib/Makefile		( NTHU CS MapleBBS Ver 3.00 )	 #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw		 	 #
#  target : Makefile for MapleBBS library routines	 #
#  create : 95/03/29				 	 #
#  update : 95/12/15					 #
# ------------------------------------------------------ #

ARCHI	!= uname -m

SRCROOT	= ../..

INCLUDE	= $(SRCROOT)/include

CC	=  clang
RANLIB	=  ranlib
CPROTO	=  cproto -E"clang -pipe -E" -I../../include
CFLAGS	=  -g -O2 -pipe -fomit-frame-pointer -Wunused -I$(INCLUDE)

.if  $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
.endif

SRC =	file.c attr_lib.c dl_lib.c


OBJ =	file.o attr_lib.o dl_lib.o

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdaofile.a


daofile.p: $(SRC)
	$(CPROTO) -o $@ $? -I$(INCLUDE)


libdaofile.a: $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) *.a *.bak *.BAK *~ tags


tags:
	ctags $(SRC) $(INCLUDE)/*.h
