# ------------------------------------------------------- #
#  util/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #

BBSHOME	?= $(HOME)

DAOLIB	= -L../lib2/str -ldaostr \
	  -L../lib2/file -ldaofile \
	  -L../lib2/wdlib -lwd \
	  -L../lib2/other -ldao

# For FreeBSD
CC	= clang
CFLAGS	= -m32 -g -O -pipe -fomit-frame-pointer -I../include -I../WD -D_BBS_UTIL_C_
LDFLAGS	= $(DAOLIB) -lcrypt

.SUFFIXES: .o .c

.c:   ;   $(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

TOOL =  setuser	

all: 	$(TOOL)

install:$(TOOL)
	install -d $(BBSHOME)/bin/
	install -c -m 755 $(TOOL) $(BBSHOME)/bin/

clean:
	rm -fr $(TOOL) *.o *.BAK *.log *~
