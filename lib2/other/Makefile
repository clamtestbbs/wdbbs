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

SRC =	\
	is_alnum.c is_alpha.c is_fname.c is_fpath.c not_addr.c \
	\
	dns.c dns_addr.c dns_name.c dns_smtp.c dns_ident.c dns_open.c \
	\
	archiv32.c archiv32m.c chrono32.c hash32.c radix32.c \
	\
	rec_add.c rec_num.c rec_del.c rec_get.c \
	rec_ins.c rec_mov.c rec_put.c rec_apply.c \
	\
	hdr_fpath.c hdr_stamp.c \
	\
	shm.c sem.c\
	\
	splay.c dl_lib.c attr_lib.c\
	\
	acl_addr.c acl_has.c xsort.c xwrite.c \
	
OBJ =	\
	is_alnum.o is_alpha.o is_fname.o is_fpath.o not_addr.o \
	\
	dns.o dns_addr.o dns_name.o dns_smtp.o dns_ident.o dns_open.o \
	\
	archiv32.o archiv32m.o chrono32.o hash32.o radix32.o \
	\
	rec_add.o rec_num.o rec_del.o rec_get.o \
	rec_ins.o rec_mov.o rec_put.o rec_apply.o \
	\
	hdr_fpath.o hdr_stamp.o \
	\
	shm.o sem.o\
	\
	splay.o dl_lib.o attr_lib.o\
	\
	acl_addr.o acl_has.o xsort.o xwrite.o	

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdao.a


dao.p:	$(SRC)
	$(CPROTO) -o $@ $?


libdao.a:	$(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) libdao.a *.bak *.BAK *~


tags:
	ctags $(SRC) ../include/*.h
