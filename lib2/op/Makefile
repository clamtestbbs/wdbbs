# ------------------------------------------------------ #
#  lib/Makefile		( NTHU CS MapleBBS Ver 3.00 )	 #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw		 	 #
#  target : Makefile for MapleBBS library routines	 #
#  create : 95/03/29				 	 #
#  update : 95/12/15					 #
# ------------------------------------------------------ #

ARCHI	!= uname -m

CC	=  clang
RANLIB	=  ranlib
CPROTO	=  cproto -E"gcc -pipe -E" 
CFLAGS	=  -g -O2 -pipe -fomit-frame-pointer -Wunused -I../../include

.if  $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
.endif

SRC =	is_alnum.c  is_alpha.c  is_fname.c  is_fpath.c  not_addr.c

OBJ =	is_alnum.o  is_alpha.o  is_fname.o  is_fpath.o  not_addr.o

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdaoop.a


dao.p:	$(SRC)
	$(CPROTO) -o $@ $?


libdaoop.a:	$(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) libdao.a *.bak *.BAK *~ tags


tags:
	ctags $(SRC) ../../include/*.h
