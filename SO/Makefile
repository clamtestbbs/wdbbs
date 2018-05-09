# ------------------------------------------------------ #
#  Makefile	( WD-BBS Ver 2.3 )			 #
# ------------------------------------------------------ #
#  target : Makefile for WD-BBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 98/12/21					 #
# ------------------------------------------------------ #

ARCHI	!= uname -m

OPSYS	!= uname -o

BBSHOME	?= $(HOME)

DAOLIB	= -L../lib2/str -ldaostr \
	  -L../lib2/file -ldaofile \
	  -L../lib2/rec -ldaorec \
	  -L../lib2/op -ldaoop \
	  -L../lib2/sys -ldaosys \
	  -L../lib2/wdlib -lwd

CC	= clang
CFLAGS	= -O2 -Wunused -I../include -pipe -fomit-frame-pointer -g 
LDFLAGS	= -lcrypt $(DAOLIB) -lcurses

.if  $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
LDFLAGS	+= -m32
.endif

.if $(OPSYS) == "FreeBSD"
LDFLAGS	+= -Wl,-export-dynamic -lcompat
.endif

.if $(OPSYS) == "GNU/Linux"
LDFLAGS	+= -rdynamic -lresolv -ldl
.endif

SO =	chat.so vote.so votebrd.so

ETC =	Makefile $(HDR)
       
.SUFFIXES: .o .c .ln .so

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c
.c.ln:  ;   lint -abhi $*.c


.if $(OPSYS) == "FreeBSD"
.o.so:	;   ld -G $*.o -o $*.so $(DAOLIB) -melf_i386_fbsd
.else
.o.so:	;   ld -G $*.o -o $*.so $(DAOLIB) -melf_i386
.endif

all:$(SO)

so: $(SO)

install: $(SO)
	install -m 0700 $(SO) $(BBSHOME)/SO

clean: /tmp
	rm -fr $(GARBAGE) *.o $(SO) $(LNFILES) *~ 
