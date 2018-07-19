# ------------------------------------------------------- #
#  Makefile  ( NTHU CS MapleBBS )                         #
# ------------------------------------------------------- #
#  target : Makefile for ALL                              #
#  create : 00/02/12                                      #
#  update : 18/05/01                                      #
# ------------------------------------------------------- #

SUBDIR 	= maple3 stuff

all clean:
	@for i in $(SUBDIR); do\
	    cd $$i && $(MAKE) $@ && cd ..;\
	done

install:
	@echo 'ok!'
