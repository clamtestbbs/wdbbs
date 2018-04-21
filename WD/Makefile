# ------------------------------------------------------ #
#  Makefile	( WD-BBS Ver 2.3)			 #
# ------------------------------------------------------ #
#  target : Makefile for WD-BBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 98/12/21					 #
# ------------------------------------------------------ #
BBSHOME = /home/bbs

# ------------------------------------------------------
# FreeBSD 3.x 4.x
# ------------------------------------------------------
OS_DEF	= -DBSD44 -DTWO_LONGJMP_ARGS 
CC	= clang
CFLAGS	= -m32 -O -Wunused -I../include -pipe -fomit-frame-pointer -g -Wno-invalid-source-encoding  #-ggdb
LDFLAGS	= -lcrypt -ltermcap -lcompat -L../lib -ldao -Wl,-export-dynamic
CPROTO	= cproto -E"gcc -pipe -E -I../include"

# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

SRC =	announce.c bbs.c board.c cache.c edit.c \
	io.c mail.c mbbsd.c menu.c more.c name.c read.c record.c\
	register.c screen.c stuff.c talk.c user.c term.c\
	admin.c list.c

MOBJ =	announce.o bbs.o board.o cache.o edit.o \
	io.o mail.o mbbsd.o menu.o more.o name.o read.o record.o\
	register.o screen.o stuff.o talk.o user.o term.o\
	admin.o list.o

PRO =	announce.x bbs.x board.x cache.x edit.x \
	io.x mail.x mbbsd.x menu.x more.x name.x read.x record.x\
	register.x screen.x stuff.x talk.x user.x term.x\
	admin.x list.x

ETC =	Makefile $(HDR)

       
.SUFFIXES: .o .c .ln .x

.c.o:   ;   $(CC) $(CFLAGS) $(OS_DEF) -c $*.c
.c.x:   ;   $(CPROTO) -o $*.x $*.c
.c.ln:  ;   lint -abhi $*.c

ALL =	mbbsd bbsrf xchatd

all: $(ALL)

mbbsd: $(MOBJ) $(ETC)
	$(CC) -o mbbsd $(CFLAGS) $(MOBJ) $(LDFLAGS)
	mv -f ../../bin/mbbsd ../../bin/mbbsd.old
	cp mbbsd ../../bin

lint: $(LNFILES)
	lint -abh $(LNFILES)

xchatd: xchatd.c
	$(CC) -o xchatd $(OS_DEF) $(CFLAGS) xchatd.c $(LDFLAGS)

bbsrf: bbsrf.c 
	$(CC) $(CFLAGS) -o bbsrf $(OS_DEF) bbsrf.c

update:	install
install: $(ALL)
	if [ -f $(BBSHOME)/bin/mbbsd ]; then\
	  mv -f $(BBSHOME)/bin/mbbsd $(BBSHOME)/bin/mbbsd.old; \
	fi
	if [ -f $(BBSHOME)/bin/bbsrf ]; then\
	  mv -f $(BBSHOME)/bin/bbsrf $(BBSHOME)/bin/bbsrf.old; \
	fi
	if [ -f $(BBSHOME)/bin/xchatd ]; then\
	  mv -f $(BBSHOME)/bin/xchatd $(BBSHOME)/bin/xchatd.old; \
	fi
	chown bbs:bbs $(ALL) $(HDR) $(SRC) $(OBJ)
#
# shakalaca.990401: 前面 LIBS = -s 與 strip 效果相同
#	strip $(ALL)
#
	ls -l mbbsd
	cp -f $(ALL) $(BBSHOME)/bin

WD.p: $(SRC)
	$(CPROTO) -o $@ $?

proto.x: $(PRO)
	/bin/rm	-f proto.x
	/bin/cat $(PRO) > proto.x

clean: /tmp
	rm -fr $(GARBAGE) $(MOBJ) $(ALL) $(PRO) proto.x $(LNFILES) *~ 

tags:
	ctags $(SRC)
