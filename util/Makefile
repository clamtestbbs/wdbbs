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

SRCROOT	= ..

INCLUDE	= $(SRCROOT)/include

DAOLIB	= -L$(SRCROOT)/lib2/str -ldaostr \
	  -L$(SRCROOT)/lib2/file -ldaofile \
	  -L$(SRCROOT)/lib2/rec -ldaorec \
	  -L$(SRCROOT)/lib2/op -ldaoop \
	  -L$(SRCROOT)/lib2/sys -ldaosys \
	  -L$(SRCROOT)/lib2/wdlib -lwd 

CC	= clang

CPROTO	= cproto -E"clang -pipe -E -I$(INCLUDE)"

CFLAGS	= -m32 -O -Wunused -I../include -I../WD -pipe -fomit-frame-pointer -g -Wno-invalid-source-encoding 

LDFLAGS	= $(DAOLIB) -lcrypt -lncurses

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

TOOL =  byte showb showdir \
        expire showboard showuser setuser #mandex 

all: 	$(TOOL)
	$(MAKE) $(TOOL)
	@for i in $(SUBDIR); do \
	        cd $$i &&       \
	       	$(MAKE) all &&  \
	        cd ..;          \
	done

install:$(TOOL)
	install -d $(BBSHOME)/bin/
	install -c -m 0755 $(TOOL) $(BBSHOME)/bin/
	@for i in $(SUBDIR); do     \
	        cd $$i &&           \
	       	$(MAKE) install &&  \
	        cd ..;              \
	done

clean:
	rm -fr $(TOOL) *.o *.BAK *.log *~
	@for i in $(SUBDIR); do \
	        cd $$i;         \
	       	$(MAKE) clean;    \
	        cd ..;          \
	done
