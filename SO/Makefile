# ------------------------------------------------------ #
#  Makefile	( WD-BBS Ver 2.3 )			 #
# ------------------------------------------------------ #
#  target : Makefile for WD-BBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 98/12/21					 #
# ------------------------------------------------------ #

ARCHI	!= uname -m

OPSYS	!= uname -o

#BBSHOME	= /home/bbs

CC	= clang
CFLAGS	= -O2 -Wunused -I../include -pipe -fomit-frame-pointer -g 
LDFLAGS	= -lcrypt -L../lib -ldao -ltermcap  

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
.o.so:	;   ld -G $*.o -o $*.so -L../lib -ldao -melf_i386_fbsd
.else
.o.so:	;   ld -G $*.o -o $*.so -L../lib -ldao -melf_i386
.endif

all:$(SO)

so: $(SO)

install: $(SO)
	install -m 0700 $(SO) $(HOME)/bin

clean: /tmp
	rm -fr $(GARBAGE) *.o $(SO) $(LNFILES) *~ 
