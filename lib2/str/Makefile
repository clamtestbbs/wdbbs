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

SRC =	str_add.c str_cat.c str_cmp.c str_decode.c str_dup.c str_folder.c \
	str_fpath.c str_from.c str_has.c str_hash.c str_len.c str_lower.c \
	str_ncmp.c str_ncpy.c str_ndup.c str_passwd.c str_pat.c str_rev.c \
	str_rle.c str_stamp.c str_str.c str_tail.c str_time.c \
	str_trim.c str_ttl.c str_trim2.c

OBJ =	str_add.o str_cat.o str_cmp.o str_decode.o str_dup.o str_folder.o \
	str_fpath.o str_from.o str_has.o str_hash.o str_len.o str_lower.o \
	str_ncmp.o str_ncpy.o str_ndup.o str_passwd.o str_pat.o str_rev.o \
	str_rle.o str_stamp.o str_str.o str_tail.o str_time.o \
	str_trim.o str_ttl.o str_trim2.o

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdaostr.a


dao.p:	$(SRC)
	$(CPROTO) -o $@ $?


libdaostr.a: $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) *.a *.bak *.BAK *~


tags:
	ctags $(SRC) ../include/*.h
