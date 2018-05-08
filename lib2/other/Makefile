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
	str_add.c str_cat.c str_cmp.c str_decode.c str_dup.c str_folder.c \
	str_fpath.c str_from.c str_has.c str_hash.c str_len.c str_lower.c \
	str_ncmp.c str_ncpy.c str_ndup.c str_passwd.c str_pat.c str_rev.c \
	str_rle.c str_stamp.c str_str.c str_tail.c str_time.c \
	str_trim.c str_ttl.c str_trim2.c\
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
	\
	setpf.c strip_ansi.c ci_strncmp.c cut_ansistr.c \
	isprint2.c not_alnum.c not_alpha.c strstr_lower.c \
	friend_count.c nextfield.c bad_user.c counter.c file_list_count.c\
	b_suckinfile.c url_encode.c

OBJ =	\
	is_alnum.o is_alpha.o is_fname.o is_fpath.o not_addr.o \
	\
	dns.o dns_addr.o dns_name.o dns_smtp.o dns_ident.o dns_open.o \
	\
	str_add.o str_cat.o str_cmp.o str_decode.o str_dup.o str_folder.o \
	str_fpath.o str_from.o str_has.o str_hash.o str_len.o str_lower.o \
	str_ncmp.o str_ncpy.o str_ndup.o str_passwd.o str_pat.o str_rev.o \
	str_rle.o str_stamp.o str_str.o str_tail.o str_time.o \
	str_trim.o str_ttl.o str_trim2.o\
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
	acl_addr.o acl_has.o xsort.o xwrite.o \
	\
	setpf.o strip_ansi.o ci_strncmp.o cut_ansistr.o \
	isprint2.o not_alnum.o not_alpha.o strstr_lower.o \
	friend_count.o nextfield.o bad_user.o counter.o file_list_count.o \
	b_suckinfile.o url_encode.o

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
