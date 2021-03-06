# ------------------------------------------------------ #
#  Makefile	( WD-BBS Ver 2.3)			 #
# ------------------------------------------------------ #
#  target : Makefile for WD-BBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 98/12/21					 #
# ------------------------------------------------------ #

SRCROOT=	..
.include "$(SRCROOT)/wdbbs.mk"

SRC =	announce.c bbs.c board.c cache.c edit.c \
	io.c mail.c mbbsd.c menu.c more.c name.c read.c record.c\
	register.c screen.c stuff.c talk.c user.c term.c\
	admin.c list.c

MOBJ =	announce.o bbs.o board.o cache.o edit.o \
	io.o mail.o mbbsd.o menu.o more.o name.o read.o record.o\
	register.o screen.o stuff.o talk.o user.o term.o\
	admin.o list.o

.ifdef NO_SO
SODIR	= ../SO
SRC	+= $(SODIR)/chat.c $(SODIR)/vote.c $(SODIR)/votebrd.c
MOBJ	+= chat.o vote.o votebrd.o

chat.o:
	$(CC) $(CFLAGS) -c $(SODIR)/$*.c

vote.o:
	$(CC) $(CFLAGS) -c $(SODIR)/$*.c

votebrd.o:
	$(CC) $(CFLAGS) -c $(SODIR)/$*.c

.endif


PRO =	announce.x bbs.x board.x cache.x edit.x \
	io.x mail.x mbbsd.x menu.x more.x name.x read.x record.x\
	register.x screen.x stuff.x talk.x user.x term.x\
	admin.x list.x

.SUFFIXES: .o .c .ln .x

.c.o:   ;   $(CC) $(CFLAGS) $(OS_DEF) -c $*.c
.c.x:   ;   $(CPROTO) -o $*.x $*.c
.c.ln:  ;   lint -abhi $*.c

ALL =	mbbsd bbsrf bbsrf2 xchatd

all: $(ALL)
	
mbbsd: $(MOBJ) $(ETC)
	$(CC) -o mbbsd $(CFLAGS) $(MOBJ) $(LDFLAGS)

lint: $(LNFILES)
	lint -abh $(LNFILES)

xchatd: xchatd.c
	$(CC) -o xchatd $(OS_DEF) $(CFLAGS) xchatd.c $(LDFLAGS)

bbsrf: bbsrf.c 
	$(CC) $(CFLAGS) -o bbsrf $(OS_DEF) bbsrf.c

bbsrf2: bbsrf2.c 
	$(CC) $(CFLAGS) -o bbsrf2 $(OS_DEF) bbsrf2.c

update:	install

install: $(ALL)
	install -d $(BBSHOME)/bin/
	install -c -m 755 $(ALL) $(BBSHOME)/bin/
	mv -f $(BBSHOME)/bin/mbbsd $(BBSHOME)/bin/mbbsd.$(BUILDTIME)
	ln -sv $(BBSHOME)/bin/mbbsd.$(BUILDTIME) $(BBSHOME)/bin/mbbsd

WD.p: $(SRC)
	$(CPROTO) -o $@ $?

proto.x: $(PRO)
	/bin/rm	-f proto.x
	/bin/cat $(PRO) > proto.x

clean: /tmp
	rm -fr $(GARBAGE) $(MOBJ) $(ALL) $(PRO) proto.x *.p $(LNFILES) *~ 

tags:
	ctags $(SRC)
