#!/bin/sh
#
# Last updated : by netty, Apr.11,1995
#
PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/etc:/usr/local/bin
clear
stty pass8
stty -parenb cs8 -istrip
username=$1
nickname=$2
echo "<<< 歡迎使用中正大學資工系所之 news reader >>>"
echo ""
echo "[媽媽叮嚀]  若您是第一次使用本功能"
echo "            您在進入之後  所有討論區皆為未訂"
echo "            建議使用 S 指令, 將多個討論區訂閱上來"
echo ""
echo "[美眉說笑]  若您在進入之後  訊息以英文出現"
echo "            建議使用 D 指令, 以切換為中文顯示模式"
echo ""
HOME=home/$username
export HOME
if [ -f $HOME/signatures ] ; then
	/bin/split -6 $HOME/signatures  $HOME/.Sig
        /bin/mv $HOME/.Sigaa $HOME/.Siga1
if [ -f $HOME/.Sigab ] ; then
    /bin/mv $HOME/.Sigab $HOME/.Siga2
if [ -f $HOME/.Sigac ] ; then	
    /bin/mv $HOME/.Sigac $HOME/.Siga3
fi
if [ -f $HOME/.Sigad ] ; then	
    /bin/mv $HOME/.Sigad $HOME/.Siga4
fi
if [ -f $HOME/.Sigae ] ; then	
    /bin/mv $HOME/.Sigae $HOME/.Siga5
fi
if [ -f $HOME/.Sigaf ] ; then	
    /bin/mv $HOME/.Sigaf $HOME/.Siga6
fi
if [ -f $HOME/.Sigag ] ; then	
    /bin/mv $HOME/.Sigag $HOME/.Siga7
fi
if [ -f $HOME/.Sigah ] ; then	
    /bin/mv $HOME/.Sigah $HOME/.Siga8
fi
if [ -f $HOME/.Sigai ] ; then	
    /bin/mv $HOME/.Sigai $HOME/.Siga9
fi
	echo -n "請選用簽名檔: 1-9  < (1) "
	read me8
	echo ""
	me9=.Siga$me8
	if [ $me9 != .Siga ] ; then
	    if [ -f $HOME/$me9 ] ; then	
	        /bin/mv $HOME/$me9 $HOME/.signature
#	        echo "            您選用第 $me8 個簽名檔" 
#	        echo ""
	    else
	        /bin/mv $HOME/.Siga1 $HOME/.signature    
	        echo "  哎丫丫!!  您並無此簽名檔, netty 為您選第 1 
個簽名檔" 
	        echo ""
	    fi
	else
	    /bin/mv $HOME/.Siga1 $HOME/.signature    
#	    echo "            您選用第 1 個簽名檔" 
#  	    echo ""
	fi
else
#	echo "[全家喜悅]  好極!!  您有一個簽名檔"
#	echo ""
	/bin/mv $HOME/.Siga1 $HOME/.signature    
fi
else
	echo "[爸爸提醒]  您尚未編輯自己的簽名檔"
	echo ""
fi
if [ -f $HOME/.newsrc ] ; then
	/bin/sort -o $HOME/.newsrc $HOME/.newsrc
fi
EDITOR=bin/celvis
echo -n "請選用編輯器 (1)bbs (2)joe (3)vi < (1) "
read me4
echo ""
me5=a$me4
if [ $me5 != a3 ] ; then
	if [ $me5 != a2 ] ; then
	    EDITOR=bin/ve
	    if [ $me5 != a1 ] ; then
	        if [ $me5 != a ] ; then
	            echo "  哎丫丫!!  您的選擇有誤, netty 為您選 bbs 
editor" 
	            echo ""
	        fi
            fi		
#	    echo "            您的編輯器為 bbs editor "
	    echo ""
	else
		EDITOR=bin/joe
#		echo "            您的編輯器為 joe "
		echo ""
	fi		
else
#	echo "            您的編輯器為 vi"
	echo ""
fi
export EDITOR
#echo -n "請按 <Enter> 或 <Return> 以進入本功能 < "
#read me2
#clear
lcctype=iso_8859_1;
LC_CTYPE=$lcctype; export LC_CTYPE
nntpserver=news.cs.ccu.edu.tw
NNTPSERVER=$nntpserver; export NNTPSERVER
USER=${username}.bbs; export USER
NAME=${nickname}; export NAME
ORGANIZATION="中正資工四百年來第一站"; export ORGANIZATION
#
trap quit 0 1 2 3 15
#
tin
#
quit () {
#	kill `ps -aux | grep 'tail -f' | grep -v grep |awk '{print $1}'` 
> /dev/null 2>&1 
	echo ""
	echo ""
	echo "[臨別贈言]  謝謝您的光臨"
	echo ""
	echo "[太宗叩首]  若有須改進之處, 請在sysop板內留下您的建議"
	echo "            您宛如一面鏡子, 我們時時需要 !!"
	echo ""
        echo -n "請按 <Enter> 或 <Return> 以結束本功能 < "
read me
}
