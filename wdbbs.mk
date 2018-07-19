# Common Make Rules in WDBBS project

ARCHI	!= uname -m

OPSYS	!= uname -o

BUILDTIME	!= date '+%s'

BBSHOME	?= $(HOME)

INCLUDE	= $(SRCROOT)/include/

DAOLIB	= -L$(SRCROOT)/lib/maple3 -ldaom3 \
	  -L$(SRCROOT)/lib/stuff -lstuff  

CC	= clang

RANLIB	= ranlib

CPROTO	= cproto -E"clang -pipe -E -I$(INCLUDE)"

CFLAGS	= -g -O -Wunused -I$(INCLUDE) -pipe -fomit-frame-pointer -Wno-invalid-source-encoding -Wno-implicit-function-declaration \
	  -Wno-implicit-int

LDFLAGS	= $(DAOLIB) -lncurses -lcrypt 

.if $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
LDFLAGS	+= -m32
.endif

.if $(OPSYS) == "GNU/Linux"
LDFLAGS	+= -lresolv -ldl -rdynamic
.endif

.if $(OPSYS) == "FreeBSD"
LDFLAGS	+= -Wl,-export-dynamic -lcompat
.endif

###################################

.if $(OPSYS) == "Cygwin"
NO_SO =	yes
.endif

###################################

.SUFFIXES: .o .c .ln .x

.c.o:   ;   $(CC) $(CFLAGS) $(OS_DEF) -c $*.c
.c.x:   ;   $(CPROTO) -o $*.x $*.c
.c.ln:  ;   lint -abhi $*.c

.if $(OPSYS) == "FreeBSD"
.o.so:	;   ld -G $*.o -o $*.so $(DAOLIB) -melf_i386_fbsd
.else
.o.so:	;   ld -G $*.o -o $*.so $(DAOLIB) -melf_i386
.endif

