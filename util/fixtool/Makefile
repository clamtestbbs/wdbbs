# ------------------------------------------------------- #
#  util/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #

BBSHOME	?= $(HOME)

CC	= clang
CFLAGS	= -m32 -g -O -pipe -fomit-frame-pointer -I../../include -D_BBS_UTIL_C_ -Wno-invalid-source-encoding
LDFLAGS	= -L../../lib -ldao -lcrypt

.SUFFIXES: .o .c

.c:   ;   $(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

TOOL =	buildetc buildnames transb transvote #\
	#buildir repasswd rebrd transpass transboard \
	#transpal transbpal transaloha  

all: 	$(TOOL)
#	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)


install:$(TOOL)
	install -d $(BBSHOME)/bin/
	install -c -m 755 $(TOOL) $(BBSHOME)/bin/

clean:
	rm -fr $(TOOL) *.o *.BAK *.log *~
