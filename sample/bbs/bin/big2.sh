#!/bin/sh
PATH=/bin:/usr/bin:/usr/local/bin
export PATH                      
stty pass8                       
stty -parenb cs8 -istrip         
clear                            
echo 你將前往成大大老二,請稍後.....
lcctype=iso_8859_1;               
LC_CTYPE=$lcctype; export LC_CTYPE
rlogin -l big2 140.116.23.74