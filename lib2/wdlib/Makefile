# ------------------------------------------------------ #
#  lib/Makefile		( NTHU CS MapleBBS Ver 3.00 )	 #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw		 	 #
#  target : Makefile for MapleBBS library routines	 #
#  create : 95/03/29				 	 #
#  update : 95/12/15					 #
# ------------------------------------------------------ #

ARCHI	!= uname -m

SRCROOT	= ../../

INCLUDE	= $(SRCROOT)/include

CC	=  clang
RANLIB	=  ranlib
CPROTO	=  cproto -E"clang -pipe -E" 
CFLAGS	=  -g -O2 -pipe -fomit-frame-pointer -Wunused -I$(INCLUDE)

.if  $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
.endif

SRC =	setpf.c strip_ansi.c ci_strncmp.c cut_ansistr.c \
	isprint2.c not_alnum.c not_alpha.c strstr_lower.c \
	friend_count.c nextfield.c bad_user.c counter.c file_list_count.c\
	b_suckinfile.c url_encode.c

OBJ =	setpf.o strip_ansi.o ci_strncmp.o cut_ansistr.o \
	isprint2.o not_alnum.o not_alpha.o strstr_lower.o \
	friend_count.o nextfield.o bad_user.o counter.o file_list_count.o \
	b_suckinfile.o url_encode.o

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdaowd.a


daowd.p:	$(SRC)
	$(CPROTO) -o $@ $? -I$(INCLUDE)


libdaowd.a: $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) *.a *.bak *.BAK *~ tags *.p


tags:
	ctags $(SRC) $(INCLUDE)/*.h
