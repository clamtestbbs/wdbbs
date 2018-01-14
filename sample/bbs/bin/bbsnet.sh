
#!/bin/sh
#
#       define the standout() and standend() escape sequences...
#
SO="[7m"
SE="[0m"
#biff n
stty pass8 -istrip
echo "*[2J*[1;1H"
echo "歡迎光臨 【^[[1;33;44m批踢踢實業坊^[[m】 使用 ^[[1mGopher^[[m"
echo "記得兩小時內回來呀!!"
echo "請按 <Enter> 或 <Return> 以進入本功能 < "
read me1

while true
do

clear

echo "  ${SO}  國內各大學 BBS 站  ${SE}  

     [1] 台大電機                        [11] 信望愛 
     [2] 中山大學                        [12] Tiger II 
     [3] 交大資工                        [13] 周慧敏
     [4] 師大計中                        [14] 中山南風站
     [5] 中原大學                        [15] 人工智慧實驗室 X
     [6] 交大資科 X                      [16] 灰姑娘城堡
     [7] 清大化學 *                      [17] 中央龍貓站
     [8] 東海大學                        [18] 動感 
     [9] 中正資工 *                      [19] 美麗女神
     [10]成功大學 * 			 [20] hinet   
                                         [21] 桃花源               
		 
        
  ${SO} 其它相關服務 ${SE}

   [30] 交大資工gopher         [35] 交大 MUD (port 4040)
   [31] 中山大學gopher         [36] 
   [32] 中正大學gopher         [37] 台大圖書館資訊系統
   [33] 交通大學hytelnet       [38] 網路象棋
   [34] 中央大學archie         [39] 台大 MUD
                               [40] 台大 gopher"
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
                  echo "台大電機 bbs" 
                   telnet 140.112.19.32
                   ;;
                2)
                  echo "中山  bbs" 
                   telnet 140.117.11.2
                   ;;
                3)
                  echo "交大資工 bbs" 
                   telnet 140.113.17.154
                   ;;
                4)
                  echo "師大計中 bbs"
                   telnet 140.122.65.19
                   ;;
                5)
                  echo "中原大學 bbs"
                   telnet 140.135.18.12
                   ;;
                6)
                  echo "交大資科 bbs" 
                   telnet 140.113.23.3 
                   ;;
                7)
                  echo "清大化學  bbs"
                   telnet 140.114.45.12
                   ;;
                8)
                  echo "東海大學 bbs" 
                   telnet 140.128.99.114
                  ;;
		9)
		  echo "中正資工 bbs" 
		  telnet 140.123.101.78 
		  ;;
                10)
                  echo "成功大學 bbs" 
                   telnet 140.116.2.12
                  ;;
                11)
                  echo "信望愛" 
                   telnet 140.113.11.234
                  ;;
                12)
                  echo "Tiger II bbs" 
                  telnet 140.113.122.208
                  ;;
		13)
		  echo "快樂天堂 bbs" 
		  telnet 140.115.249.100
		  ;;
		14)
		  echo "中山南風站 bbs"
		  telnet 140.117.11.4
		  ;;
		15)
		  echo "人工智慧實驗室 bbs"
		  telnet 140.114.78.31
		  ;;
                16)
                  echo "灰姑娘城堡 bbs"
                  telnet 140.113.30.91
                  ;;
                17)
                  echo "中央龍貓站 bbs"
                  telnet 140.115.83.240
                  ;;
                18)
                  echo "動感 bbs"
                  telnet 140.113.122.157
                  ;;
                19)
                  echo "美麗女神 bbs"
                  telnet 140.113.189.34
                  ;;
                20)
		  echo "hinet"
		  telnet serv.hinet.net
	          ;;
                21)  
                   exho "桃花源"
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
                  echo "交大gopher" 
                  telnet gopher.csie.nctu.edu.tw 4270
                  ;;
                 31)
                  echo "中山gopher" 
                  echo "Login as 'gopher'"
                  sleep 1
                  telnet gopher.nsysu.edu.tw
                  ;;
                 32)
                  echo "中正gopher" 
                  echo "Login as 'gopher'"
                  sleep 1
                  telnet gopher.ccu.edu.tw
                  ;;
                 33)
                  echo "交大hytelnet" 
                  echo "Login as 'hytelnet'" 
                  sleep 1
                  telnet nctuccca.edu.tw
                  ;;
                 34)
                  echo "中央archie" 
                  echo "Login as 'twarchie' or 'archie'"
                  sleep 1
                  telnet archie.ncu.edu.tw
                  ;;
                35)
                  echo "交大 Merc DikuMud 2.0" 
                  telnet ccsun44.csie.nctu.edu.tw 4040
                  ;;
#                36)
#                  echo "交大 The Reamls of Wind II" 
#                  telnet ccsun44.csie.nctu.edu.tw 1234
#                  ;;
                37)
                  echo "台大圖書館" 
                  echo "以'tulips' login 可進入中文查詢主畫面"
                  echo "or login as 'library' for English users"
                  echo ""
                  telnet 140.112.192.1
                  ;;
                38)
                  echo "網路象棋" 
                  telnet 128.103.28.15 5555
                  ;;
                39)
                  echo "台大 MUD" 
                  telnet 140.112.2.33 3000
                  ;;
		40)
		  echo "台大gopher" 
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


