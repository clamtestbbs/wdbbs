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
CPROTO	=  cproto -E"gcc -pipe -E" 
CFLAGS	=  -g -O2 -pipe -fomit-frame-pointer -Wunused -I$(INCLUDE) -Wno-invalid-source-encoding

.if  $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
.endif

SRC =	string.c

OBJ =	string.o

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdaostr.a


dao.p:	$(SRC)
	$(CPROTO) -o $@ $? -I$(INCLUDE)


libdaostr.a: $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) *.a *.bak *.BAK *~ tags


tags:
	ctags $(SRC) $(INCLUDE)/*.h
