#include "bbs.h"

#include <stdlib.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define SERVER_CATV     "www.favor.net.tw"

#define CGI_CATV        "http://www.favor.net.tw/cgi/prog2.cgi"

#define REFER_CATV      "http://www.favor.net.tw/cgi/prog2.cgi#top"

#define WEBPORT         80
#define PARA            "Connection: Keep-Alive\nUser-Agent: Mozilla/4.5b1 [en] (X11; I; FreeBSD 2.2.7-STABLE i386)\nContent-type:
application/x-www-form-urlencoded\nAccept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*\nAccept-Encoding: gzip\nAccept-Language:
en\nAccept-Charset: iso-8859-1,*,utf-8\n"

#define vget(a,b,c,d,e,f) getdata(a,b,c,d,e,f,0)

char *show_channel(int i)
{
clear();
switch(i)
{

case(1):
return "»Oµø";
break;
case(2):
return "¤¤µø";
break;
case(3):
return "µØµø";
break;
case(4):
return "¥Áµø";
break;
case(5):
return "¤½µø";
break;
case(6):
return "¥Áµø·s»D";
break;
case(7):
return "Ápµn¹q¼v";
break;
case(8):
return "Às²»¹q¼v";
break;
case(9):
return "¾ÇªÌ°ê¤ù";
break;
case(10):
return "¾ÇªÌ°]¸g";
break;
case(11):
return "¥þ²y¼v«°";
break;
case(12):
return "HBO";
break;
case(13):
return "CINEMAX";
break;
case(14):
return "GOGO TV ";
break;
case(15):
return "TV-TIME";
break;
case(16):
return "¬K·u¹q¼v";
break;
case(17):
return "¦nµÜ¶õ";
break;
case(18):
return "­}¤h¥§¥d³q";
break;
case(19):
return "­ºµØ¥d³q";
break;
case( 20):
return "³Õ·sªF¬M";
break;
case(21):
return "½n¨Ó¤é¥»¥x";
break;
case(22):
return "½n¨Ó¹qµø¥x";
break;
case(23):
return "½n¨ÓÅé¨|";
break;
case(24):
return "¶Wµø¤@¥x";
break;
case(25):
return "AXN";
break;
case(26):
return "¤T¥ßºîÃÀ";
break;
case(27):
return "¤T¥ß³£·|";
break;
case(28):
return "¤T¥ß¥xÆW¥x";
break;
case(29):
return "½Ãµø¤¤¤å¥x";
break;
case(30):
return "½Ãµø¹q¼v¥x";
break;
case(31):
return "½ÃµøÅé¨|¥x";
break;
case(32):
return "½Ãµø­µ¼Ö¥x";
break;
case(33):
return "½Ãµø¦è¤ù¥x";
break;
case(34):
return "TVBS";
break;
case(35):
return "TVBS-N";
break;
case(36):
return "TVIS";
break;
case(37):
return "GTV-27";
break;
case(38):
return "GTV-28";
break;
case(39):
return "ªFµø¹q¼v";
break;
case(40):
return "ªFµøºî¦X";
break;
case(41):
return "ªFµø¨|¼Ö";
break;
case(42):
return "ªFµø¥d³q¥x";
break;
case(43):
return "ªFµø·s»D";
break;
case(44):
return "¤¤³£¹q¼v";
break;
case(45):
return "¤¤³£À¸¼@";
break;
case(46):
return "µØ½Ã­µ¼Ö";
break;
case(47):
return "µØ½Ã¹qµø";
break;
case(48):
return "µØ½Ã·s»D¥x";
break;
case(49):
return "µØ½Ã®È¹C";
break;
case(50):
return "CTN¤¤¤ÑÀW";
break;
case(51):
return "¤j¦aÀW¹D";
break;
case(52):
return "°ê¿³¤é¤ù";
break;
case(53):
return "JET¤é¥»¥x";
break;
case(54):
return "NHK¨È¬w¥x";
break;
case(55):
return "CNN";
break;
case(56):
return "DISCOVERY";
break;
case(57):
return "TNT¥d³q";
break;
case(58):
return "ESPNÅé¨|";
break;
case(59):
return "ÅRÆEÀW¹D";
break;
case(60):
return "µØ®L½Ã¬P¥x";
break;
case(61):
return "·sµø½Ã¬P";
break;
case(62):
return "¤¤µø¤G¥x";
break;
case(63):
return "«D¤Z¤@¥x";
break;
case(64):
return "«D¤Z·s»D";
break;
case(65):
return "Àô²y·s»D";
break;
case(66):
return "¶W¯Å¤Ó¶§¥x";
break;
case(67):
return "®ü¤W¥P¤s";
break;
case(68):
return "ªk¬É½Ã¬P";
break;
case(69):
return "·OÀÙ¤j·R";
break;
case(70):
return "±Ð¨|ÀW¹D";
break;
case(71):
return "½Ã´¶¹q¸£¥x";
break;
case(72):
return "±s»Ê±Ð¨|";
break;
case(73):
return "EXPLORE";
break;
case(74):
return "¥ÁÃÀ½Ã¬P";
break;
case(75):
return "CASA";
break;
case(76):
return "ºëÀY¸£";
break;
}
}



int
catv()
{
        int sockfd,i;
        char ID[10], Name[32],*CID,*CName;
        char trn[512], sendform[512],result1[665];
        struct sockaddr_in serv_addr;
        struct hostent *hp;
        int flag=0;
        FILE *fp;
        char fpath[200];
        char* pstr;
        char ans[5];
#if 0
        memset(trn, 0, 512);
        memset(sendform, 0, 512);
        memset(result, 0, 4096);
        memset(Name, 0, 32);
        memset(ID, 0, 16);
#endif

	counter("log/counter/¸`¥Ø¬d¸ß","¨Ï¥Î¹qµø¸`¥Ø¬d¸ß",0);
        setutmpmode(CATV);
        do{
                clear();
                show_file("/home/bbs/etc/TVPro",1,19,ONLY_COLOR);
                if(!(vget(20, 2, "½Ð¿é¤J±z­n¬d¸ßªºÀW¹D¡G", ID, 5, DOECHO))) return 0;
                i=atoi(ID);
        }while(i<1 || i>76);

        switch(i)
        {
                case(1):
                        strcpy(ID,"ttv");
                        break;
                case(2):
                        strcpy(ID,"cts");
                        break;
                case(3):
                        strcpy(ID,"ctv");
                        break;
                case(4):
                        strcpy(ID,"mintv");
                        break;
                case(5):
                        strcpy(ID,"pts");
                        break;
                case(6):
                        strcpy(ID,"mintv1");
                        break;
                case(7):
                        strcpy(ID,"fsw2");
                        break;
                case(8):
                        strcpy(ID,"ls1");
                        break;
                case(9):
                        strcpy(ID,"shl1");
                        break;
                case(10):
                        strcpy(ID,"shl2");
                        break;
                case(11):
                        strcpy(ID,"shl3");
                        break;
                case(12):
                        strcpy(ID,"hbo1");
                        break;
                case(13):
                        strcpy(ID,"cinemax1");
                        break;
                case(14):
                        strcpy(ID,"gogo1");
                        break;
                case(15):
                        strcpy(ID,"time1");
                        break;
                case(16):
                        strcpy(ID,"sun1");
                        break;
                case(17):
                        strcpy(ID,"cbs1");
                        break;
                case(18):
                        strcpy(ID,"phtv1");
                        break;
                case(19):
                        strcpy(ID,"sw1");
                        break;
                case(20):
                        strcpy(ID,"phtv2");
                        break;
                case(21):
                        strcpy(ID,"vl1");
                        break;
                case(22):
                        strcpy(ID,"vl2");
                        break;
                case(23):
                        strcpy(ID,"vl3");
                        break;
                case(24):
                        strcpy(ID,"tvbs4");
                        break;
                case(25):
                        strcpy(ID,"tvbs5");
                        break;
                case(26):
                        strcpy(ID,"sing1");
                        break;
                case(27):
                        strcpy(ID,"sing2");
                        break;
                case(28):
                        strcpy(ID,"sing3");
                        break;
                case(29):
                        strcpy(ID,"star0");
                        break;
                case(30):
                        strcpy(ID,"star4");
                        break;
                case(31):
                        strcpy(ID,"star2");
                        break;
                case(32):
                        strcpy(ID,"star3");
                        break;
                case(33):
                        strcpy(ID,"star5");
                        break;
                case(34):
                        strcpy(ID,"tvbs1");
                        break;
                case(35):
                        strcpy(ID,"tvbs3");
                        break;
                case(36):
                        strcpy(ID,"tvbs2");
                        break;
                case(37):
                        strcpy(ID,"fsw1");
                        break;
                case(38):
                        strcpy(ID,"gtv1");
                        break;
                case(39):
                        strcpy(ID,"u1");
                        break;
                case(40):
                        strcpy(ID,"u2");
                        break;
                case(41):
                        strcpy(ID,"u3");
                        break;
                case(42):
                        strcpy(ID,"u5");
                        break;
                case(43):
                        strcpy(ID,"u4");
                        break;
                case(44):
                        strcpy(ID,"cd1");
                        break;
                case(45):
                        strcpy(ID,"cd2");
                        break;
                case(46):
                        strcpy(ID,"chst1");
                        break;
                case(47):
                        strcpy(ID,"chst2");
                        break;
                case(48):
                        strcpy(ID,"chst3");
                        break;
                case(49):
                        strcpy(ID,"chst4");
                        break;
                case(50):
                        strcpy(ID,"ctn1");
                        break;
                case(51):
                        strcpy(ID,"ctn2");
                        break;
                case(52):
                        strcpy(ID,"jap1");
                        break;
                case(53):
                        strcpy(ID,"jet1");
                        break;
                case(54):
                        strcpy(ID,"nhka1");
                        break;
                case(55):
                        strcpy(ID,"cnn");
                        break;
                case(56):
                        strcpy(ID,"disc1");
                        break;
                case(57):
                        strcpy(ID,"tnt1");
                        break;
                case(58):
                        strcpy(ID,"espn1");
                        break;
                case(59):
                        strcpy(ID,"pl1");
                        break;
                case(60):
                        strcpy(ID,"ch1");
                        break;
                case(61):
                        strcpy(ID,"sgp1");
                        break;
                case(62):
                        strcpy(ID,"ffbus1");
                        break;
                case(63):
                        strcpy(ID,"ffbus2");
                        break;
                case(64):
                        strcpy(ID,"world1");
                        break;
                case(65):
                        strcpy(ID,"ssun1");
                        break;
                case(66):
                        strcpy(ID,"sea1");
                        break;
                case(67):
                        strcpy(ID,"fg1");
                        break;
                case(68):
                        strcpy(ID,"heart1");
                        break;
                case(69):
                        strcpy(ID,"love1");
                        break;
                case(70):
                        strcpy(ID,"vhf");
                        break;
                case(71):
                        strcpy(ID,"cpt1");
                        break;
                case(72):
                        strcpy(ID,"hin1");
                        break;
                case(73):
                        strcpy(ID,"explore");
                        break;
                case(74):
                        strcpy(ID,"minee");
                        break;
                case(75):
                        strcpy(ID,"casa1");
                        break;
                case(76):
                        strcpy(ID,"mind1");
                        break;
        }
        vget(21,2, "½Ð¿é¤J±z­n¬d¸ßªº¤é´Á¡G", Name, 10, DOECHO);

   move(22,2);
        if (answer(msg_sure_ny) != 'y')
        {
                pressanykey("©ñ±ó¬d¸ß");
                return;
        }

        CID=ID;CName=Name;
        sprintf(trn,"DATE=%s&CH=%s",CName, CID);

        sprintf(sendform, "post %s HTTP/1.0\nReferer: %s\n%sContent-length:%d\n\n%s",
            CGI_CATV, REFER_CATV, PARA, strlen(trn), trn);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                return;

        memset((char *)&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;

        if ((hp = gethostbyname(SERVER_CATV)) == NULL)
                return;

        memcpy(&serv_addr.sin_addr, hp->h_addr, hp->h_length);

        serv_addr.sin_port = htons(WEBPORT);

        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof serv_addr))
        {
                return;
        }
        else
        {
                refresh();
        }

        write(sockfd, sendform, strlen(sendform));
        shutdown(sockfd, 1);

   setuserfile(fpath,"catv.log");
   
        while (read(sockfd, result1, sizeof(result1)) > 0)
        {
                
                if ((strstr(result1, "¸`¥Ø¦WºÙ")) != NULL)
                {
                        fp=fopen(fpath, "w");
                        //read(sockfd, result1, sizeof(result1));
                        pstr=show_channel(i);
                        fprintf(fp,"[1;33;44m¹qµø¥x¡G%s  ¬d¸ß¤é´Á¡G%s            [0m\n",pstr,Name);
                        //fprintf(fp,"%s",result1);
                        for(i=1;i<665;i++)
                        {
                                if(result1[i]=='O' && flag==0) break;
                                if(flag==1)
                                {
                                        if(result1[i]=='<')
                                                flag=0;
                                        else
                                        {
                                                fprintf(fp,"%c",result1[i]);
                                        }
                                }
                                if(result1[i]=='>')
                                        flag=1;
                        }

                        memset(result1, 0, sizeof(result1));
                        read(sockfd, result1, sizeof(result1));

                        for(i=0;i<665;i++)
                        {
                                if(result1[i]=='O' && flag==0) break;
                                if(flag==1)
                                {
                                        if(result1[i]=='<')
                                                flag=0;
                                        else
                                        {
                                                fprintf(fp,"%c",result1[i]);
                                        }
                                }
                                if(result1[i]=='>')
                                        flag=1;
                        }

                        memset(result1, 0, sizeof(result1));
                        read(sockfd, result1, sizeof(result1));

                        for(i=0;i<665;i++)
                        {
                                if(result1[i]=='O' && flag==0) break;
                                if(flag==1)
                                {
                                        if(result1[i]=='<')
                                                flag=0;
                                        else
                                        {
                                                fprintf(fp,"%c",result1[i]);
                                        }
                                }
                                if(result1[i]=='>')
                                        flag=1;
                        }
                        memset(result1, 0, sizeof(result1));
                        close(sockfd);
                        fclose(fp);
                        more(fpath);
                        getdata(b_lines, 0, "²¾¦Ü³Æ§Ñ¿ý¡H [y/N]",ans, 3, LCECHO, 0);
                        if(ans[0]=='Y' || ans[0]=='y')
                        {
                                fileheader mymail;
                                char title[128], buf[80];
                                sethomepath(buf, cuser.userid);
                                stampfile(buf, &mymail);
                                mymail.savemode = '\0';        /* hold-mail flag */
                                mymail.filemode = FILE_READ;
                                strcpy(mymail.owner, "[³Æ.§Ñ.¿ý]");
                                strcpy(mymail.title, "¹qµø¥x¬d¸ßµ²ªG");
                                sethomedir(title, cuser.userid);
                                rec_add(title, &mymail, sizeof(mymail));
                                f_mv(fpath, buf);
                        }
                        else
                                unlink(fpath);
                        pressanykey("¬d¸ß§¹¦¨");
                        return;
                }
                else
                {
                        memset(result1, 0, sizeof(result1));
                }
        }

        close(sockfd);
        pressanykey("¬d¸ß¥¢±Ñ");
        return;
}


