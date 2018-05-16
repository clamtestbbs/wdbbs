# ------------------------------------------------------ #
#  Makefile	( WD-BBS Ver 2.3 )			 #
# ------------------------------------------------------ #
#  target : Makefile for WD-BBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 98/12/21					 #
# ------------------------------------------------------ #
BBSHOME = /home/bbs

# ------------------------------------------------------
# FreeBSD 3.x, Linux
# ------------------------------------------------------
OS_DEF	= -DBSD44 -DTWO_LONGJMP_ARGS 
CC	= gcc
CFLAGS	= -O2 -Wunused -I../include -pipe -fomit-frame-pointer -g
LDFLAGS	= -lcrypt -ltermcap -lcompat -L../lib -ldao -export-dynamic -L/usr/local/lib/mysql -lmysqlclient

# ------------------------------------------------------
# NetBSD, 386BSD, FreeBSD 2.x
# ------------------------------------------------------
#OS_DEF	= -DBSD44 -DTWO_LONGJMP_ARGS 
#CC	= gcc
#CFLAGS	= -O2 -Wunused -I../include -pipe -fomit-frame-pointer -s
#LDFLAGS	= -lcrypt -ltermcap -lcompat -L../lib -ldao
#CPROTO	= cproto -E"gcc -pipe -E -I../include"

# ------------------------------------------------------
# Linux
# ------------------------------------------------------
#OS_DEF	= -DLINUX -DTWO_LONGJMP_ARGS 
#CC	= gcc
#CFLAGS	= -O2 -Wunused -I../include -pipe -fomit-frame-pointer -I/usr/include/bsd
#LDFLAGS	= -ltermcap -lbsd -L../lib -ldao -export-dynamic -ldl -lcrypt
#CPROTO	= cproto -E"gcc -pipe -E -I../include"

# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

SO =	bwboard.so railway.so pip.so bbcall.so tv.so nine.so dice.so gp.so\
	pnote.so indict.so chat.so mine.so xyz.so song.so race.so five.so\
	guessnum.so stock.so bingo.so bj.so gagb.so gamble.so marie.so\
	poker.so seven.so chessmj.so bet.so vote.so mn.so rpg.so buy.so\
	fortune.so votebrd.so mime.so tobuy.so

ETC =	Makefile $(HDR)

       
.SUFFIXES: .o .c .ln .so

.c.o:   ;   $(CC) $(CFLAGS) $(OS_DEF) -c $*.c
.c.ln:  ;   lint -abhi $*.c
.o.so:	;   ld -s -G $*.o -o $*.so -L../lib -ldao

so: $(SO)

install: $(SO)
	cp -f *.so ../../SO

clean: /tmp
	rm -fr $(GARBAGE) *.o $(SO) $(LNFILES) *~ 
