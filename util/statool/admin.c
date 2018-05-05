/*-------------------------------------------------------*/
/* util/topusr2.c        ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : ¨Ï¥ÎªÌ ¤W¯¸°O¿ý/¤å³¹½g¼Æ ±Æ¦æº]              */
/* create : 97/03/27                                     */
/* update : 95/03/31                                     */
/*-------------------------------------------------------*/

#undef MONTH
#define REAL_INFO
#undef HIDE
#define ADMIN_REPORT

#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"
#include "record.c"
int mannum;

struct binfo
{
  char  boardname[18];
  char  expname[28];
  int times;
  int sum;
  int post;
  int num;
  int del;
  int pur;
  int tag;
} st[MAXBOARD];

int numboards=0;
int numgroups=0;
int numhide=0;
int numhideg=0;

int
brd_cmp(b, a)
struct binfo *a, *b;
{
    if(a->sum!=b->sum)
            return (a->sum - b->sum);
    return a->times - b->times;
}

/*
char    *Ctime(date)
time_t  *date;
{
        static char buf[80];

        strcpy(buf, (char *)ctime(date));
        buf[strlen(buf)-1] = '\0';
        return buf;
}
*/

int
record_data(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].times++;
                        st[i].sum+=sec;
                        return;
                }
        }
        return ;
}

int
record_data2(board)
char *board;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].post++;
                        return;
                }
        }
        return ;
}

int
record_data3(board)
char *board;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].del++;
                        st[i].num++;
                        return;
                }
        }
        return ;
}

int
record_data4(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].pur++;
                        st[i].num+=sec;
                        return;
                }
        }
        return ;
}

int
record_data5(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].tag++;
                        st[i].num+=sec;
                        return;
                }
        }
        return ;
}






int
fillbcache(fptr)
    struct boardheader *fptr ;
{

    if( numboards >= MAXBOARD )
        return 0;

    /* Gene */
#ifdef HIDE
    if((fptr->level != 0) && !(fptr->level & PERM_POSTMASK))
        return;
#endif
    if((fptr->level != 0) && !(fptr->level & PERM_POSTMASK))
        numhide++;
    if ((strstr(fptr->title, "£["))||(strstr(fptr->title, "£U"))||
         (strstr(fptr->title, "£S")))
    {
    numgroups++;
    if((fptr->level != 0) && !(fptr->level & PERM_POSTMASK))
        numhideg++;
    return;}

    strcpy(st[numboards].boardname,fptr->brdname);
    strcpy(st[numboards].expname,fptr->title);
/*    printf("%s %s\n",st[numboards].boardname,st[numboards].expname); */
    st[numboards].times=0;
    st[numboards].sum=0;
    st[numboards].post=0;
    st[numboards].del=0;
    st[numboards].pur=0;
    st[numboards].num=0;
    st[numboards].tag=0;

    numboards++;
    return 0 ;
}

int
fillboard()
{
  rec_apply( BBSHOME"/.BOARDS", fillbcache, sizeof(boardheader));
}

/*
char *
timetostr(i)
int i;
{
        static char str[30];
        int minute,sec,hour;

        minute=(i/60);
        hour=minute/60;
        minute=minute%60;
        sec=i&60;
        sprintf(str,"%2d:%2d\'%2d\"",hour,minute,sec);
        return str;
}
*/

struct manrec
{
  char userid[IDLEN+1];
  char username[23];
  char userid2[IDLEN+1];
  usint userlevel;
  usint userlevel2;
  ushort numlogins;
  ushort numposts;
  ushort numloginsto;
  ushort numpoststo;
  ushort numloginsyes;
  ushort numpostsyes;
  ushort messto;
  ushort messfrom;
};
typedef struct manrec manrec;
struct manrec allman[MAXUSERS];

userec aman;
int num;
FILE *fp,*fp2;



int
record_mess(name,type)
char *name;
int type;
{
  int i;
  int n;

  for(i=0;i<mannum;i++)
  {
    if (!strcmp(name, allman[i].userid))
    {
      if (type == 0) allman[i].messfrom++;
      else allman[i].messto++;
      return;
    }
  }
  return ;
}


main(argc, argv)
  int argc;
  char **argv;
{
  FILE *inf3;
  FILE *inf2;
  FILE *inf;
  FILE *fpf;
  int i;
  int n;
  int numlog=0, numlog2=0;
  int numpo=0, numpo2=0;
  int maxlog=0, maxlog2=0;
  int maxpo=0, maxpo2=0;
  char maxlogid[IDLEN+1];
  char maxpoid[IDLEN+1];
  char maxlogid2[IDLEN+1];
  char maxpoid2[IDLEN+1];
  int userlog=0, userlog2=0;
  int userpo=0, userpo2=0;
  time_t now = time(0);
  struct tm *ptime;
  char *progmode;
  FILE *fp1;
/*  FILE *op; */
  char buf[256], *p,bname[20];
/*  char date[80];
  int mode;
  int c[3]; */
  int max[3];
  unsigned int ave[3];
  int sec;
  int j,k;
  char timesbname[20];
  char sumbname[20];
  char uname_from[20];
  char uname_to[20];
  int messnum;
  int max_from=0;
  int max_to=0;
  int user_from=0;
  char setby[13];
  char setto[13];
  int act[27];                  /* ¦¸¼Æ/²Ö­p®É¶¡/pointer */
  int user_to=0;
  int hour;
  int newreg=0;
  int numtalk=0;
  int numchat=0;
  int numnewb=0;
  int numnameb=0;
  int numdelb=0;
  int numdated=0;
  int numclean=0;
  int numsetb=0;
  int numkill=0;
  int numsuci=0;
  int numsetu=0;
  int numsetself=0;
  int num;
  int alltime=0;
  int alltimes=0;
  int allpost=0;
  int allnum=0;
  int alldel=0;
  int allpur=0;
  int alltag=0;
  int maxtoday=0;
  int numsysop=0;
  int numboard=0;
  int numaccount=0;
  int numchatroom=0;
  int numbm=0;
  int numsee=0;
  int numcloak=0;
  int numloginok=0;
  int guestnum=0;

  setuid(BBSUID);
  setgid(BBSGID);
  chdir(BBSHOME);

#ifdef MONTH
  if ((fp1 = fopen(BBSHOME "/adm/board.read", "r")) == NULL)
#else
  if ((fp1 = fopen(BBSHOME"/usboard", "r")) == NULL)
#endif
  {
    printf("cann't open usboard\n");
    /* return 1 */;
  }

  fillboard();
  while (fgets(buf, 256, fp1))
  {
    if ( !strncmp(buf, "USE", 3))
    {
      p=strstr(buf,"USE");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+25, "Stay: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data(bname,sec);
    }
    if ( !strncmp(buf, "DEL", 3))
    {
      p=strstr(buf,"DEL");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    record_data3(bname);
    }

    if ( !strncmp(buf, "PUR", 3))
    {
      p=strstr(buf,"PUR");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+25, "with: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data4(bname,sec);
    }
    if ( !strncmp(buf, "TAG", 3))
    {
      p=strstr(buf,"TAG");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+25, "with: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data5(bname,sec);
    }


    if ( !strncmp(buf, "POS", 3))
    {
      p=strstr(buf,"POS");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
      record_data2(bname);
    }


   }
   /* qsort */
   ave[0]=0;
   ave[1]=0;
   ave[2]=0;
   max[1]=0;
   max[0]=0;
   max[2]=0;
   for(i=0;i<numboards;i++)
   {
        ave[0]+=st[i].times;
        ave[1]+=st[i].sum;
        ave[2]+=st[i].times==0?0:st[i].sum/st[i].times;
        if(max[0]<st[i].times)
        {
                max[0]=st[i].times;
                strcpy(timesbname, st[i].boardname);
        }
        if(max[1]<st[i].sum)
        {
                max[1]=st[i].sum;
                strcpy(sumbname, st[i].boardname);
        }
        if(max[2]<(st[i].times==0?0:st[i].sum/st[i].times))
        {
                max[2]=(st[i].times==0?0:st[i].sum/st[i].times);
        }
        alltime+=st[i].sum;
        alltimes+=st[i].times;
        alldel+=st[i].del;
        allpur+=st[i].pur;
        alltag+=st[i].tag;
        allpost+=st[i].post;
        allnum+=st[i].num;
   }
   numboards++;
   st[numboards-1].times=ave[0]/numboards;
   st[numboards-1].sum=ave[1]/numboards;
   strcpy(st[numboards-1].boardname,"Total");
   strcpy(st[numboards-1].expname,"Á`¦X");
   qsort(st, numboards, sizeof( st[0] ), brd_cmp);


  now = time(NULL);
  ptime = localtime(&now);

#ifdef MONTH
  inf = fopen(BBSHOME "/.PASSWDS.yes", "rb");
#else
  inf = fopen(BBSHOME "/.PASSWDS", "rb");
#endif

  if (inf == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    /* exit(0) */;
  }

  for (i = 0; fread(&aman, sizeof(userec), 1, inf); i++)
  {
    strcpy(allman[i].userid, aman.userid);
    strncpy(allman[i].username, aman.username,23);
    allman[i].numloginsto = aman.numlogins;
    allman[i].numpoststo = aman.numposts;
    allman[i].userlevel = aman.userlevel;

#ifdef  HAVE_TIN
    allman[i].numposts += post_in_tin(allman[i].userid);
#endif
  }

#ifdef MONTH
  inf2 = fopen(BBSHOME "/.PASSWDS.month", "rb");
#else
  inf2 = fopen(BBSHOME "/.PASSWDS.yes", "rb");
#endif
  if (inf2 == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    /* exit(0) */;
  }

  for (i = 0; fread(&aman, sizeof(userec), 1, inf2); i++)
  {
    strcpy(allman[i].userid2, aman.userid);
    allman[i].numloginsyes = aman.numlogins;
    allman[i].numpostsyes = aman.numposts;
    allman[i].userlevel2 = aman.userlevel;

#ifdef  HAVE_TIN
    allman[i].numposts += post_in_tin(allman[i].userid);
#endif
  }

  n=i-1;
  mannum=n;

  for (i = 0; i<=n; i++)
  {
    if (!strcmp(allman[i].userid, allman[i].userid2))
    {
      allman[i].numlogins = allman[i].numloginsto - allman[i].numloginsyes;
      allman[i].numposts = allman[i].numpoststo - allman[i].numpostsyes;
    }
    else
    {
      allman[i].numlogins = allman[i].numloginsto;
      allman[i].numposts = allman[i].numpoststo;
    }
    if (allman[i].numpoststo < allman[i].numpostsyes)
      allman[i].numposts = 0;
    if (allman[i].numloginsto < allman[i].numloginsyes)
      allman[i].numlogins = 0;
  }

#ifdef MONTH
    if ((fpf = fopen(BBSHOME "/adm/usies", "r")) == NULL)
#else
    if ((fpf = fopen(BBSHOME "/usies", "r")) == NULL)
#endif
  {
    printf("cann't open usies\n");
    /* return 1 */;
  }

    while (fgets(buf, 256, fpf))
  {
    hour = atoi(buf + 9);
    if (hour < 0 || hour > 23)
    {
      continue;
    }
    if (!(strncmp(buf +28 , "guest ", 6)))
      {
      guestnum++;
      }

    if (!(strncmp(buf +22, "APPLY", 5)))
      {
      newreg++;
      continue;
      }
    if (!(strncmp(buf +22, "DATED", 5)))
      {
      numdated++;
      continue;
      }
    if (!(strncmp(buf +22, "CLEAN", 5)))
      {
      numclean++;
      continue;
      }
    if (!(strncmp(buf +22, "SUCI", 4)))
      {
      numsuci++;
      continue;
      }
    if (!(strncmp(buf +22, "KILL", 4)))
      {
      numkill++;
      continue;
      }
    if (!(strncmp(buf +22, "NewBoard", 8)))
      {
      numnewb++;
      continue;
      }
    if (!(strncmp(buf +22, "DelBoard", 8)))
      {
      numdelb++;
      continue;
      }
    if (!(strncmp(buf +22, "SetBoard", 8)))
      {
      numsetb++;
      continue;
      }
    if (!(strncmp(buf +22, "NameBoard", 9)))
      {
      numnameb++;
      continue;
      }


    if (!(strncmp(buf +22, "CHAT ", 5)))
      {
      numchat++;
      continue;
      }
    if (!(strncmp(buf +22, "TALK ", 5)))
      {
      numtalk++;
      continue;
      }
    if (!strncmp(buf + 22, "ENTER", 5))
    {
      act[hour]++;
      continue;
    }
    if (!strncmp(buf + 22, "SetUser", 7))
    {
      p=strstr(buf,"SetUser");
      p+=8;
      p=strtok(p," ");
      strcpy(setby,p);
      p=strstr(buf, setby);
      p+=13;
      p=strtok(p," ");
      if (strstr(p,"\n"))
          p=strtok(p,"\n");
      strcpy(setto,p);
      if (strcmp(setto, setby))
        numsetu++;
      else
        numsetself++;
    }
    if (p = (char *) strstr(buf + 40, "Stay:"))
    {
      if (hour = atoi(p + 5))
      {
        act[24] += hour;
        act[25]++;
      }
      continue;
    }
  }
  fclose(fpf);

  if(fpf = fopen(".maxtoday", "r"))
  {
    fscanf(fpf, "%d", &maxtoday);
    fclose(fpf);
  }

  if ((fp = fopen(BBSHOME "/log/admin.log", "w")) == NULL)
  {
    printf("cann't open admin.log\n");
    /* return 0*/;
  }
  if((fp2 = fopen(BBSHOME "/log/board.log", "w")) == NULL)
  {
    printf("cann't open board.log\n");
    /* return 0*/;
  }

  for(i=0; i<=n; i++)
  {
    numlog+=allman[i].numloginsto;
    numpo+=allman[i].numpoststo;
    numlog2+=allman[i].numlogins;
    numpo2+=allman[i].numposts;
    if (allman[i].numloginsto>0) userlog++;
    if (allman[i].numlogins>0) userlog2++;
    if (allman[i].numpoststo>0) userpo++;
    if (allman[i].numposts>0) userpo2++;
    if (allman[i].numloginsto>maxlog) {
      maxlog=allman[i].numloginsto;
      strcpy(maxlogid, allman[i].userid2);
    }
    if (allman[i].numpoststo>maxpo) {
      maxpo=allman[i].numpoststo;
      strcpy(maxpoid, allman[i].userid2);
    }
    if (allman[i].numlogins>maxlog2) {
      maxlog2=allman[i].numlogins;
      strcpy(maxlogid2, allman[i].userid2);
    }
    if (allman[i].numposts>maxpo2) {
      maxpo2=allman[i].numposts;
      strcpy(maxpoid2, allman[i].userid2);
    }
    if (allman[i].messfrom>0) user_from++;
    if (allman[i].messto>0) user_to++;
    if (allman[i].messfrom>max_from) {
      max_from=allman[i].messfrom;
      strcpy(uname_from, allman[i].userid2);
    }
    if (allman[i].messto>max_to) {
      max_to=allman[i].messto;
      strcpy(uname_to, allman[i].userid2);
    }
    if (allman[i].userlevel & PERM_SYSOP) numsysop++;
    if (allman[i].userlevel & PERM_ACCOUNTS) numaccount++;
    if (allman[i].userlevel & PERM_BOARD) numboard++;
    if (allman[i].userlevel & PERM_CHATROOM) numchatroom++;
    if (allman[i].userlevel & PERM_BM) numbm++;
    if (allman[i].userlevel & PERM_SEECLOAK) numsee++;
    if (allman[i].userlevel & PERM_CLOAK) numcloak++;
    if (allman[i].userlevel & PERM_LOGINOK) numloginok++;
  }

  fprintf(fp, "    [1;31m%s[m %s ³ø§i\n",
     BOARDNAME,Ctime(&now));
  fprintf(fp, "\n");
  fprintf(fp, "    ¨´¤µ ¤w¦³ [1;33m%10d[m ¤H¤W¯¸¹L [1;33m%10d[m ¦¸, ¥­§¡¨C¤H [1;33m%5d[m ¦¸\n",
     userlog, numlog, numlog/userlog);
  fprintf(fp, "    ¨´¤µ ¤w¦³ [1;33m%10d[m ¤Hµoªí¹L [1;33m%10d[m ½g, ¥­§¡¨C¤H [1;33m%5d[m ½g\n\n",
     userpo, numpo, numpo/userlog);
  fprintf(fp, "    ¤µ¤Ñ ¤w¦³ [1;33m%10d[m ¤H¤W¯¸¹L [1;33m%10d[m ¦¸, ¥­§¡¨C¤H [1;33m%5d[m ¦¸\n",
     userlog2, numlog2, numlog2/userlog2);
  fprintf(fp, "    ¤µ¤Ñ ¤w¦³ [1;33m%10d[m ¤Hµoªí¹L [1;33m%10d[m ½g, ¥­§¡¨C¤H [1;33m%5d[m ½g\n\n",
     userpo2, numpo2, numpo2/userlog2);
  fprintf(fp, "    ¤µ¤Ñ ¤W¯¸ ³Ì¦h¦¸ªº¤H¬O [1;33m%13s[m ¦³ [1;33m%10d[m ¦¸\n",
     maxlogid2, maxlog2);
  fprintf(fp, "    ¤µ¤Ñ µoªí ³Ì¦h¦¸ªº¤H¬O [1;33m%13s[m ¦³ [1;33m%10d[m ½g\n\n",
     maxpoid2, maxpo2);
  fprintf(fp, "    ¤µ¤Ñ Åªª© [1;33m%8d[m ¦¸ ¦@ [1;33m%8d[m ¤À"
             " ¥­§¡¨Cª© [1;33m%5d[m ¤H¦¸ ¦@ [1;33m%5d[m ¤À \n",
     ave[0], ave[1]/60, ave[0]/numboards, ave[1]/(numboards*60));
  fprintf(fp, "    ¤µ¤Ñ Åªª© ¦¸¼Æ³Ì°ª¬O [1;33m%-13s[m ª© ¦@ [1;33m%5d[m ¦¸ ¤@¯ëª©­Ó¼Æ¬° [1;33m%5d[m ­Ó \n"
              "    ¤µ¤Ñ Åªª© ®É¶¡³Ì°ª¬O [1;33m%-13s[m ª© ¦@ [1;33m%5d[m ¤À ¤@¯ë¸s²Õ¼Æ¬° [1;33m%5d[m ­Ó\n\n",
     timesbname, max[0], numboards-1, sumbname, max[1]/60, numgroups);
  fprintf(fp, "    ¤µ¤Ñ Á`¦@¦³ [1;33m%6d[m ­Ó°T®§ ¨ä¤¤ ¦³ [1;33m%5d[m ­Ó¤Hµo ¦³ [1;33m%5d[m ­Ó¤H¦¬\n"
              "    µo³Ì¦hªº¬O [1;33m%13s[m ¦³ [1;33m%4d[m ¦¸"
              " ¦¬³Ì¦hªº¬O [1;33m%13s[m ¦³ [1;33m%4d[m ¦¸\n\n",
     messnum, user_from, user_to, uname_from, max_from, uname_to, max_to);
  fprintf(fp, "    ¤µ¤Ñ ¦³ [1;33m%5d[m ­Ó¤Hµù¥U  ¦³ [1;33m%5d[m ­Ó guest ¤W¨Ó¹L"
              " ¥þ³¡ªá¤F [1;33m%8d[m ¤ÀÄÁ\n"
              "    ¤µ¤Ñ ³Ì°ª¦³ [1;33m%5d[m ¦P®É¤W¯¸ ¥­§¡¦³ [1;33m%5d[m ¤H¤W¯¸",
     newreg, /* act[25]-numlog2 */  guestnum , act[24], maxtoday, act[24]/1440);
  fprintf(fp,"\n    ¤µ¤Ñ ¦³ [1;33m%5d[m ¦¸ TALK ¦³ [1;33m%5d[m ­Ó¤H CHAT\n",
      numtalk, numchat);
  fprintf(fp, "    ¤µ¤Ñ ¦³ [1;33m%5d[m ­Ó id ³Q¬å ¦³ [1;33m%5d[m ¹L´Á"
              " ¦³ [1;33m%5d[m ³Q²M ¦³ [1;33m%5d[m ¦Û±þ\n",
     numkill, numdated, numclean, numsuci);
  fprintf(fp, "    ¤µ¤Ñ ¦³ [1;33m%5d[m ­Ó¤H§ï¸ê®Æ ¦³ [1;33m%5d[m ­Ó¤H³Q§ï¸ê®Æ\n",
     numsetself, numsetu);

  fprintf(fp, "\n    ¦³ [1;33m%5d[m ­Ó ¦³­­¨îªº ª© ¤Î [1;33m%5d[m ­Ó ¦³­­¨îªº ¸s²Õ \n",
      numhide-numhideg, numhideg);

  fprintf(fp, "\n    ¤µ¤Ñ ¶} [1;33m%5d[m ­Óª© Ãö [1;33m%5d[m ­Óª© "
              "³]©w [1;33m%5d[m ­Óª© ©R¦W [1;33m%5d[m ­Óª©\n",
      numnewb, numdelb, numsetb, numnameb);

  fprintf(fp, "\n    ¯¸ªø¦³ [1;33m%3d[m ¤H, ±b¸¹Á`ºÞ¦³ [1;33m%3d[m ¤H, "
              "¬Ýª©Á`ºÞ¦³ [1;33m%3d[m ¤H, ²á¤Ñ«ÇÁ`ºÞ¦³ [1;33m%3d[m ¤H\n",
    numsysop, numaccount, numboard, numchatroom);
  fprintf(fp, "    ª©¥D¦³ [1;33m%3d[m ¤H, ¬Ý¨£§ÔªÌ¦³ [1;33m%3d[m ¤H, "
              "¦³Áô¨­³N¦³ [1;33m%3d[m ¤H, §¹¦¨µù¥U¦³ [1;33m%5d[m ¤H\n",
    numbm, numsee, numcloak, numloginok);

/*------- wildcat : ¤À¨â­ÓÀÉ¬ö¿ý -------*/

  fprintf(fp2, "==>[1;32m ¬ÝªOª¬ªp³ø§i [33m%s[m\n",Ctime(&now));
  fprintf(fp2,"»¡©ú:®É¶¡->°±¯d®É¶¡(¬í) ¤H¦¸->¶iªO¤H¦¸ §R°£->³Q§R°£½g¼Æ");

  fprintf(fp2,"\n[1;37;42m¦W¦¸ %-15.15s%-28.28s %6s %4s %4s %3s %3s %3s[m\n",
                "°Q½×°Ï¦WºÙ","¤¤¤å±Ô­z","  ®É¶¡","¤H¦¸","§R°£","POST","TAG","DEL");

 for(i=0;i<MAXBOARD;i++)
 {
   if(st[i].sum)
     fprintf(fp2,"[1;33m%4d[m %-15.15s%-28.28s [1;32m%6d [31m%4d [m%4d [1;36m%3d[m %3d %3d\n",
     i+1,st[i].boardname,st[i].expname,st[i].sum,st[i].times,st[i].num
        , st[i].post,st[i].tag,st[i].del);
 }
     fprintf(fp2,"[1;37;42m     %-15.15s%-28.28s %6d %4d %4d %3d %3d %3d\n",
     "Total","Á`¦X",alltime,alltimes,allnum,allpost,alltag,alldel);

  printf("numlog = %d\n", numlog);
  printf("numlog2 = %d\n", numlog2);
  printf("numpo = %d\n", numpo);
  printf("numpo2 = %d\n", numpo2);
  printf("userlog = %d\n", userlog);
  printf("userlog2 = %d\n", userlog2);
  printf("userpo = %d\n", userpo);
  printf("userpo2 = %d\n", userpo2);
  printf("Maxpost %s = %d\n", maxpoid, maxpo);
  printf("Maxlogin %s = %d\n", maxlogid, maxlog);
  printf("Maxpost2 %s = %d\n", maxpoid2, maxpo2);
  printf("Maxlogin2 %s = %d\n", maxlogid2, maxlog2);
  fclose(fp);
  fclose(fp2);
}
