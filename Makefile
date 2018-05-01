# ------------------------------------------------------- #
#  Makefile  ( NTHU CS MapleBBS )                         #
# ------------------------------------------------------- #
#  target : Makefile for ALL                              #
#  create : 00/02/12                                      #
#  update : 18/05/01                                      #
# ------------------------------------------------------- #

OPSYS	!= uname -o

# 需要 compile 的目錄
# lib WD SO util

all:
	@cd lib; $(MAKE) all
	@cd WD; $(MAKE) all
.if $(OPSYS) != "Cygwin"
	@cd SO; $(MAKE) all
.endif
	@cd util; $(MAKE) all

install:
	@cd WD; $(MAKE) install
.if $(OPSYS) != "Cygwin"
	@cd SO; $(MAKE) install
.endif
	@cd util; $(MAKE) install

clean:
	@cd lib; $(MAKE) clean
	@cd WD; $(MAKE) clean
	@cd SO; $(MAKE) clean
	@cd util; $(MAKE) clean
