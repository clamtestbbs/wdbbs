#!/bin/sh
PATH=/bin:/usr/bin:/usr/local/bin
export PATH                      
cp -f /home/bbs/src/WD/*.c /home/ftp/bbs/src/WD
cp -f /home/bbs/src/WD/Makefile /home/ftp/bbs/src/WD
cp -f /home/bbs/src/lib/* /home/ftp/bbs/src/lib
cp -f /home/bbs/src/util/*.c /home/ftp/bbs/src/util
cp -f /home/bbs/src/util/Makefile /home/ftp/bbs/src/util
cp -f /home/bbs/src/include/*.h /home/ftp/bbs/src/include
cp -fr /home/bbs/src/innbbsd/* /home/ftp/bbs/src/innbbsd
