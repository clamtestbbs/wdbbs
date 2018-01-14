#!/bin/sh
#
# Last updated date: Apr.03,1995
#
set LC_CTYPE=iso_8859_1
#biff n
TERM=vt100
export TERM
export LC_CTYPE
LOCALDOMAIN=.
export LOCALDOMAIN
stty pass8 -istrip
echo "  "
echo "Åwªï¥úÁ{ ¡i[1;33;44m­·»P¹Ð®Jªº¹ï¸Ü[m¡j ¨Ï¥Î [1mGopher[m"
echo "°O±o¦^¨Ó§r!!"
echo "½Ð«ö <Enter> ©Î <Return> ¥H¶i¤J¥»¥\¯à < "
read me1
exec /usr/local/bin/gopher -s cc.shu.edu.tw

