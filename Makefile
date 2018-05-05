# ------------------------------------------------------- #
#  Makefile  ( NTHU CS MapleBBS )                         #
# ------------------------------------------------------- #
#  target : Makefile for ALL                              #
#  create : 00/02/12                                      #
#  update : 18/05/01                                      #
# ------------------------------------------------------- #

OPSYS	!= uname -o

# 需要 compile 的目錄
SUBDIR = lib WD util

.if $(OPSYS) != "Cygwin"
SUBDIR += SO
.endif

all:
	@for i in $(SUBDIR); do \
	        cd $$i;         \
	       	$(MAKE) all;    \
	        cd ..;          \
	done

install:
	@for i in $(SUBDIR); do \
	        cd $$i;         \
	       	$(MAKE) install;    \
	        cd ..;          \
	done

clean:
	@for i in $(SUBDIR); do \
	        cd $$i;         \
	       	$(MAKE) clean;    \
	        cd ..;          \
	done


