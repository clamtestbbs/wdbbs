# ------------------------------------------------------ #
#  Makefile	( WD-BBS Ver 2.3 )			 #
# ------------------------------------------------------ #
#  target : Makefile for WD-BBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 98/12/21					 #
# ------------------------------------------------------ #

SRCROOT=	..
.include "$(SRCROOT)/wdbbs.mk"

SO =	bet.so bj.so buy.so bwboard.so chat.so chessmj.so \
       	dice.so gagb.so votebrd.so vote.so #gamble.so gp.so guessnum.so indict.so \
#       	marie.so mn.so nine.so ordersong.so pip.so pip_vs_fight.so \
#       	pnote.so poker.so railway.so rpg.so seven.so song.so stock.so\
#       	tv.so xyz.so 

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
