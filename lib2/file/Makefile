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
CPROTO	=  cproto -E"clang -pipe -E" -I../../include
CFLAGS	=  -g -O2 -pipe -fomit-frame-pointer -Wunused -I../../include

.if  $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
.endif

SRC =	f_cat.c f_cp.c f_img.c f_ln.c f_map.c f_mode.c f_mv.c \
	f_new.c f_open.c f_path.c f_rm.c f_suck.c f_lock.c attr_lib.c dl_lib.c


OBJ =	f_cat.o f_cp.o f_img.o f_ln.o f_map.o f_mode.o f_mv.o \
	f_new.o f_open.o f_path.o f_rm.o f_suck.o f_lock.o attr_lib.o dl_lib.o

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdaofile.a


daofile.p: $(SRC)
	$(CPROTO) -o $@ $?


libdaofile.a: $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) *.a *.bak *.BAK *~ tags


tags:
	ctags $(SRC) ../../include/*.h
