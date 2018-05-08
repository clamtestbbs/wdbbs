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

SRC =	dns.c dns_addr.c dns_name.c dns_smtp.c dns_ident.c dns_open.c 

OBJ =	dns.o dns_addr.o dns_name.o dns_smtp.o dns_ident.o dns_open.o 

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdaodns.a

libdaodns.a: $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) *.a *.bak *.BAK *~


tags:
	ctags $(SRC) ../include/*.h
