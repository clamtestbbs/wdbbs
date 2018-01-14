#!/usr/local/bin/tcsh
setenv	CVSROOT	":pserver:anoncvs@rouge.dragon2.net:/cvs"
#cvs login
cd /home/bbs/src/CVS
cvs update -d
