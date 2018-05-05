
/* 	這是 舊WD(9806以後)==>>WD990617 轉 .PASSWDS 的程式 
	執行後新的 PASSWDS 會出現在 ~bbs/PASSWDS.NEW
 	注意 :  使用前請務必備份現有 .PASSWDS
		覆蓋檔案前請清光 SHM 並確定站上無任何 USER 	
						紫筠軒 Ziyun   */
/*	wildcat patch : 支援多種 bbs version */
/* 請配合自己的系統版本 define */
#define	Current_WD	/* WD_BBS 0617 */
#undef	OLD_WD		/* OLD WD_BBS */
#undef	SOB		/* SOB Version */
#undef	Ptt		/* Ptt Version */

#include "bbs.h"


userec tuser;

struct new
{
  char userid[IDLEN + 1];         /* 使用者名稱  13 bytes */
  char realname[20];              /* 真實姓名    20 bytes */
  char username[24];              /* 暱稱        24 bytes */
  char passwd[PASSLEN];           /* 密碼        14 bytes */
  uschar uflag;                   /* 使用者選項   1 byte  */
  usint userlevel;                /* 使用者權限   4 bytes */
  ushort numlogins;               /* 上站次數     2 bytes */
  ushort numposts;                /* POST次數     2 bytes */
  time_t firstlogin;              /* 註冊時間     4 bytes */
  time_t lastlogin;               /* 前次上站     4 bytes */
  char lasthost[24];              /* 上站地點    24 bytes */
  char vhost[24];                 /* 虛擬網址    24 bytes */
  char email[50];                 /* E-MAIL      50 bytes */
  char address[50];               /* 地址        50 bytes */
  char justify[REGLEN + 1];       /* 註冊資料    39 bytes */
  uschar month;                   /* 出生月份     1 byte  */
  uschar day;                     /* 出生日期     1 byte  */
  uschar year;                    /* 出生年份     1 byte  */
  uschar sex;                     /* 性別         1 byte  */
  uschar state;                   /* 狀態??       1 byte  */
  usint habit;                    /* 喜好設定     4 bytes */
  uschar pager;                   /* 心情顏色     1 bytes */
  uschar invisible;               /* 隱身模式     1 bytes */
  usint exmailbox;                /* 信箱封數     4 bytes */
  usint exmailboxk;               /* 信箱K數      4 bytes */
  usint toquery;                  /* 好奇度       4 bytes */
  usint bequery;                  /* 人氣度       4 bytes */
  char toqid[IDLEN + 1];	  /* 前次查誰    13 bytes */
  char beqid[IDLEN + 1];	  /* 前次被誰查  13 bytes */
  unsigned long int totaltime;    /* 上線總時數   8 bytes */
  usint sendmsg;                  /* 發訊息次數   4 bytes */
  usint receivemsg;               /* 收訊息次數   4 bytes */
  unsigned long int goldmoney;	  /* 風塵金幣     8 bytes */ 
  unsigned long int silvermoney;  /* 銀幣         8 bytes */
  unsigned long int exp;          /* 經驗值       8 bytes */
  time_t dtime;                   /* 存款時間     4 bytes */
  int limitmoney;		  /* 金錢下限	  4 bytes */
  uschar rtimes;		  /* 填註冊單次數 1 bytes */
  int award;			  /* 獎懲判斷	  4 bytes */ 
  int pagermode; 		  /* 呼叫器門號   4 bytes */
  char pagernum[7];		  /* 呼叫器號碼   7 bytes */
  char feeling[5];		  /* 心情指數     5 bytes */
  char pad[123];		  /* 空著填滿至512用      */
};

typedef struct new new;

main()
{
  int fdr,fdw,i=0;
  new new;
  
  fdr=open(BBSHOME"/.PASSWDS",O_RDONLY);
  fdw=open(BBSHOME"/PASSWDS.NEW",O_WRONLY | O_CREAT | O_TRUNC, 0644);
  printf("%d",sizeof(new));

  while(read(fdr,&tuser,sizeof(userec))==sizeof(userec))
  {
    if (strlen(tuser.userid) < 2) continue;
    if (not_alpha(*tuser.userid)) continue;
/*
    while (ch = *(++userid))
    {
      if (not_alnum(ch))
       continue;
    }
*/
        memcpy(new.userid,tuser.userid,IDLEN+1);
        memcpy(new.realname,tuser.realname,20); 
        memcpy(new.username,tuser.username,24); 
        memcpy(new.passwd,tuser.passwd,PASSLEN);
        new.uflag=tuser.uflag;                  
        new.userlevel=tuser.userlevel;          
        new.numlogins=tuser.numlogins;          
        new.numposts=tuser.numposts;            
  	new.firstlogin=tuser.firstlogin;    
  	new.lastlogin=tuser.lastlogin;      
  	memcpy(new.lasthost,tuser.lasthost,24);
  	memcpy(new.vhost,tuser.lasthost,24);   
  	memcpy(new.email,tuser.email,50);      
  	memcpy(new.address,tuser.address,50);  
  	memcpy(new.justify,tuser.justify,REGLEN + 1);  
  	new.month=tuser.month;              
  	new.day=tuser.day;                  
  	new.year=tuser.year;                
  	new.sex=tuser.sex;                  
  	new.state=tuser.state;              
  	new.pager=tuser.pager;          
  	new.invisible=tuser.invisible;  
  	new.exmailbox=tuser.exmailbox;  
#ifdef Current_WD
  	memcpy(new.toqid,tuser.userid,IDLEN+1);     
  	memcpy(new.beqid,tuser.userid,IDLEN+1);     
  	new.habit=tuser.habit;          
  	new.exmailboxk=tuser.exmailboxk;
  	new.toquery=tuser.toquery;      
  	new.bequery=tuser.bequery;      
  	new.totaltime=tuser.totaltime;  
  	new.sendmsg=tuser.sendmsg;      
  	new.receivemsg=tuser.receivemsg;
  	new.dtime=tuser.dtime;
	new.silvermoney=tuser.silvermoney;
#endif
#ifdef OLD_WD
  	memcpy(new.toqid,tuser.userid,IDLEN+1);     
  	memcpy(new.beqid,tuser.userid,IDLEN+1);     
  	new.habit=tuser.habit;          
  	new.exmailboxk=tuser.exmailboxk;
  	new.toquery=tuser.toquery;      
  	new.bequery=tuser.bequery;      
  	new.totaltime=tuser.totaltime;  
  	new.sendmsg=tuser.sendmsg;      
  	new.receivemsg=tuser.receivemsg;
  	new.dtime=tuser.dtime;
	new.silvermoney=(tuser.money+tuser.deposit);
#endif
#ifdef SOB
  	memcpy(new.toqid,0,IDLEN+1);     
  	memcpy(new.beqid,0,IDLEN+1);     
  	new.habit=0;
  	new.exmailboxk=0;
  	new.exmailbox=0;
  	new.toquery=0;
  	new.bequery=0;
  	new.totaltime=0;
  	new.sendmsg=0;
  	new.receivemsg=0;
  	new.dtime=0;
	new.silvermoney=1000;
#endif
#ifdef Ptt
  	memcpy(new.toqid,0,IDLEN+1);     
  	memcpy(new.beqid,0,IDLEN+1);     
  	new.habit=0;
  	new.exmailboxk=0;
  	new.toquery=0;
  	new.bequery=0;
  	new.totaltime=0;
  	new.sendmsg=0;
  	new.receivemsg=0;
  	new.dtime=0;
	new.silvermoney=tuser.money;
  	new.exmailbox=tuser.emailbox;
#endif
#if defined (OLD_WD) || (SOB) || (Ptt)
	new.exp=0;
	new.goldmoney=0;
	new.limitmoney=0;
	new.rtimes=0;
	new.award=0;
        memcpy(new.feeling,"?",4);
	new.pagermode=0;
	memcpy(new.pagernum,"000",6);
#else
	new.exp=tuser.exp;
	new.goldmoney=tuser.goldmoney;
	new.limitmoney=tuser.limitmoney;
	new.rtimes=tuser.rtimes;
	new.award=tuser.award;
        memcpy(new.feeling," ",5);
	new.pagermode=tuser.pagermode;
	memcpy(new.pagernum," ",7);
#endif
        write(fdw,&new,sizeof(new));
        ++i;
//        if(!new.userid[0] && new.address[0])
//        printf("%-5d - %-13s %s\n",i,new.userid,new.address);
   }
   close(fdr);
   close(fdw);
}     
