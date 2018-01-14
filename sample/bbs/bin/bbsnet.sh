
#!/bin/sh
#
#       define the standout() and standend() escape sequences...
#
SO="[7m"
SE="[0m"
#biff n
stty pass8 -istrip
echo "*[2J*[1;1H"
echo "Åwªï¥úÁ{ ¡i^[[1;33;44m§å½ð½ð¹ê·~§{^[[m¡j ¨Ï¥Î ^[[1mGopher^[[m"
echo "°O±o¨â¤p®É¤º¦^¨Ó§r!!"
echo "½Ð«ö <Enter> ©Î <Return> ¥H¶i¤J¥»¥\¯à < "
read me1

while true
do

clear

echo "  ${SO}  °ê¤º¦U¤j¾Ç BBS ¯¸  ${SE}  

     [1] ¥x¤j¹q¾÷                        [11] «H±æ·R 
     [2] ¤¤¤s¤j¾Ç                        [12] Tiger II 
     [3] ¥æ¤j¸ê¤u                        [13] ©P¼z±Ó
     [4] ®v¤j­p¤¤                        [14] ¤¤¤s«n­·¯¸
     [5] ¤¤­ì¤j¾Ç                        [15] ¤H¤u´¼¼z¹êÅç«Ç X
     [6] ¥æ¤j¸ê¬ì X                      [16] ¦Ç©h®Q«°³ù
     [7] ²M¤j¤Æ¾Ç *                      [17] ¤¤¥¡Às¿ß¯¸
     [8] ªF®ü¤j¾Ç                        [18] °Ê·P 
     [9] ¤¤¥¿¸ê¤u *                      [19] ¬üÄR¤k¯«
     [10]¦¨¥\¤j¾Ç * 			 [20] hinet   
                                         [21] ®çªá·½               
		 
        
  ${SO} ¨ä¥¦¬ÛÃöªA°È ${SE}

   [30] ¥æ¤j¸ê¤ugopher         [35] ¥æ¤j MUD (port 4040)
   [31] ¤¤¤s¤j¾Çgopher         [36] 
   [32] ¤¤¥¿¤j¾Çgopher         [37] ¥x¤j¹Ï®ÑÀ]¸ê°T¨t²Î
   [33] ¥æ³q¤j¾Çhytelnet       [38] ºô¸ô¶H´Ñ
   [34] ¤¤¥¡¤j¾Çarchie         [39] ¥x¤j MUD
                               [40] ¥x¤j gopher"
echo -n "Enter choice [0]: " \

        if read CHOICE
          then
            clear
            echo -n "$USER `date`" 
            case "${CHOICE}"
              in
                '')
                  echo "" 
                  break
                  ;;
                0)
                  echo ""
                  break
                  ;;
                1)
                  echo "¥x¤j¹q¾÷ bbs" 
                   telnet 140.112.19.32
                   ;;
                2)
                  echo "¤¤¤s  bbs" 
                   telnet 140.117.11.2
                   ;;
                3)
                  echo "¥æ¤j¸ê¤u bbs" 
                   telnet 140.113.17.154
                   ;;
                4)
                  echo "®v¤j­p¤¤ bbs"
                   telnet 140.122.65.19
                   ;;
                5)
                  echo "¤¤­ì¤j¾Ç bbs"
                   telnet 140.135.18.12
                   ;;
                6)
                  echo "¥æ¤j¸ê¬ì bbs" 
                   telnet 140.113.23.3 
                   ;;
                7)
                  echo "²M¤j¤Æ¾Ç  bbs"
                   telnet 140.114.45.12
                   ;;
                8)
                  echo "ªF®ü¤j¾Ç bbs" 
                   telnet 140.128.99.114
                  ;;
		9)
		  echo "¤¤¥¿¸ê¤u bbs" 
		  telnet 140.123.101.78 
		  ;;
                10)
                  echo "¦¨¥\¤j¾Ç bbs" 
                   telnet 140.116.2.12
                  ;;
                11)
                  echo "«H±æ·R" 
                   telnet 140.113.11.234
                  ;;
                12)
                  echo "Tiger II bbs" 
                  telnet 140.113.122.208
                  ;;
		13)
		  echo "§Ö¼Ö¤Ñ°ó bbs" 
		  telnet 140.115.249.100
		  ;;
		14)
		  echo "¤¤¤s«n­·¯¸ bbs"
		  telnet 140.117.11.4
		  ;;
		15)
		  echo "¤H¤u´¼¼z¹êÅç«Ç bbs"
		  telnet 140.114.78.31
		  ;;
                16)
                  echo "¦Ç©h®Q«°³ù bbs"
                  telnet 140.113.30.91
                  ;;
                17)
                  echo "¤¤¥¡Às¿ß¯¸ bbs"
                  telnet 140.115.83.240
                  ;;
                18)
                  echo "°Ê·P bbs"
                  telnet 140.113.122.157
                  ;;
                19)
                  echo "¬üÄR¤k¯« bbs"
                  telnet 140.113.189.34
                  ;;
                20)
		  echo "hinet"
		  telnet serv.hinet.net
	          ;;
                21)  
                   exho "®çªá·½"
		   telnet 140.115.234.5
		   ;;
		E)
                  echo "Eagle's Nest" 
                  telnet 131.95.127.2
                  ;;
                e)
                  echo "Eagle's Nest"
                  telnet 131.95.127.2
                  ;;
                B)
                  echo "Badboy" 
                  telnet 192.136.108.18
                  ;;
                b)
                  echo "Badboy" 
                  telnet 192.136.108.18
                  ;;
                A)
                  echo "Auggie" 
                  telnet 141.224.128.4
                  ;;
                a)
                  echo "Auggie" 
                  telnet 141.224.128.4
                  ;;
                U)
                  echo "UNINET" 
                  telnet 131.178.6.132
                  ;;
                u)
                  echo "UNINET" 
                  telnet 131.178.6.132
                  ;;
                Q)
                  echo "Quartz" 
                  telnet 128.6.60.6
                  ;;
                q)
                  echo "Quartz" 
                  telnet 128.6.60.6
                  ;;
                I)
                  echo "ISCA" 
                  telnet 128.255.40.203
                  ;;
                i)
                  echo "ISCA" 
                  telnet 128.255.40.203
                  ;;
                S)
                  echo "Sunset" 
                  telnet 128.196.230.7
                  ;;
                s)
                  echo "Sunset" 
                  telnet 128.196.230.7
                  ;;
                 30)
                  echo "¥æ¤jgopher" 
                  telnet gopher.csie.nctu.edu.tw 4270
                  ;;
                 31)
                  echo "¤¤¤sgopher" 
                  echo "Login as 'gopher'"
                  sleep 1
                  telnet gopher.nsysu.edu.tw
                  ;;
                 32)
                  echo "¤¤¥¿gopher" 
                  echo "Login as 'gopher'"
                  sleep 1
                  telnet gopher.ccu.edu.tw
                  ;;
                 33)
                  echo "¥æ¤jhytelnet" 
                  echo "Login as 'hytelnet'" 
                  sleep 1
                  telnet nctuccca.edu.tw
                  ;;
                 34)
                  echo "¤¤¥¡archie" 
                  echo "Login as 'twarchie' or 'archie'"
                  sleep 1
                  telnet archie.ncu.edu.tw
                  ;;
                35)
                  echo "¥æ¤j Merc DikuMud 2.0" 
                  telnet ccsun44.csie.nctu.edu.tw 4040
                  ;;
#                36)
#                  echo "¥æ¤j The Reamls of Wind II" 
#                  telnet ccsun44.csie.nctu.edu.tw 1234
#                  ;;
                37)
                  echo "¥x¤j¹Ï®ÑÀ]" 
                  echo "¥H'tulips' login ¥i¶i¤J¤¤¤å¬d¸ß¥Dµe­±"
                  echo "or login as 'library' for English users"
                  echo ""
                  telnet 140.112.192.1
                  ;;
                38)
                  echo "ºô¸ô¶H´Ñ" 
                  telnet 128.103.28.15 5555
                  ;;
                39)
                  echo "¥x¤j MUD" 
                  telnet 140.112.2.33 3000
                  ;;
		40)
		  echo "¥x¤jgopher" 
		  echo "Login as 'gopher'"
		  echo ""
		  telnet 140.112.8.19
                  ;;
                *)
                  echo "" 
                  echo ""
                  echo "${SO}${CHOICE} is an invalid option. Select again.${SE}" 
                  sleep 1
                  ;;
            esac
          else
            exit
        fi
done

clear
#biff y


