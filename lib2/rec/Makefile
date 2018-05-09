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

SRC =	rec_add.c rec_num.c rec_del.c rec_get.c \
	rec_ins.c rec_mov.c rec_put.c rec_apply.c
	
OBJ =	rec_add.o rec_num.o rec_del.o rec_get.o \
	rec_ins.o rec_mov.o rec_put.o rec_apply.o 
	
.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdaorec.a


dao.p:	$(SRC)
	$(CPROTO) -o $@ $?


libdaorec.a:	$(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) libdao.a *.bak *.BAK *~ tags


tags:
	ctags $(SRC) ../../include/*.h
