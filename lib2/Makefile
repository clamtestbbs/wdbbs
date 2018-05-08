# ------------------------------------------------------- #
#  Makefile  ( NTHU CS MapleBBS )                         #
# ------------------------------------------------------- #
#  target : Makefile for ALL                              #
#  create : 00/02/12                                      #
#  update : 18/05/01                                      #
# ------------------------------------------------------- #

OPSYS	!= uname -o

SUBDIR 	= other file str

all clean:
	@for i in $(SUBDIR); do\
	    cd $$i && $(MAKE) $@ && cd ..;\
	done

install:
	@echo 'ok!'
