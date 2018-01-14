#!/bin/sh

/usr/local/bin/lynx -source http://www.cwb.gov.tw/Data/forecast/W03.txt > /home/bbs/etc/weather
/usr/local/bin/lynx -dump "http://myweb.ltsh.ilc.edu.tw/weather/warea.pl?area=%A5x%A5_%A5%AB" > /home/bbs/etc/weather2
