# ------------------------------------------------------- #
#  util/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #

SRCROOT	= ../..
.include "$(SRCROOT)/wdbbs.mk"

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

