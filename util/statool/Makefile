# ------------------------------------------------------- #
#  util/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #

ARCHI	!= uname -m

OPSYS	!= uname -o

SUBDIR	= statool fixtool

BBSHOME	?= $(HOME)

SRCROOT	= ../..

INCLUDE	= $(SRCROOT)/include

DAOLIB	= -L$(SRCROOT)/lib/maple3 -ldaom3 \
	  -L$(SRCROOT)/lib/stuff -lstuff 

CC	= clang

CFLAGS	= -m32 -O -Wunused -I$(INCLUDE) -pipe -fomit-frame-pointer -g -Wno-invalid-source-encoding 

LDFLAGS	= -lcrypt -lncurses $(DAOLIB)

.if  $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
LDFLAGS	+= -m32
.endif

.if $(OPSYS) == "GNU/Linux"
LDFLAGS	+= -lresolv -ldl -rdynamic
.endif

.if $(OPSYS) == "FreeBSD"
LDFLAGS	+= -Wl,-export-dynamic -lcompat
.endif

.SUFFIXES: .o .c

.c:   ;   $(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

TOOL =	poststat birth horoscope topmsg voteboard\
	topquery toppost toptime toplogin topexp\
	topmoney topboard topgold sexcount toplimit\
	toppersonal account #admin bm_check gnp yearsold 

all: $(TOOL)

install:$(TOOL)
	install -d $(BBSHOME)/bin/
	install -c -m 0755 $(TOOL) $(BBSHOME)/bin/

clean:
	rm -fr $(TOOL) *.o *.BAK *.log *~
