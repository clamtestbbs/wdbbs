# ------------------------------------------------------- #
#  util/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #

SRCROOT	= ..
.include "$(SRCROOT)/wdbbs.mk"

SUBDIR	= statool fixtool


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
