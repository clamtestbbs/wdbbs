/******************/
/* ÂIºq¾÷         */
/* §@ªÌ¡GPtt      */
/* ­×§ï¡Gwildcat  */
/******************/

#include "bbs.h"

#define MAXSONGS 300
#define SONGPATH BBSHOME"/etc/SONGO"
#define SONGBOOK BBSHOME"/etc/SONGBOOK"
#define KTV BBSHOME"/man/boards/KTV"


typedef
struct songcmp
{
  char name[100];
  char cname[100];
  long int count;
}
songcmp ;

long int totalcount=0;

int
count_cmp(b, a)
 songcmp *a, *b;
{
    return (a->count - b->count);
}
void
topsong()
{
 more(FN_TOPSONG,YEA);
}
     
int strip_blank(char *cbuf, char *buf)
{
  for(;*buf;buf++)  if(*buf != ' ') *cbuf++=*buf;	  
  *cbuf=0;
}
void
sortsong()
{
  FILE *fo,*fp=fopen(BBSHOME "/" FN_USSONG,"r");
  songcmp songs[MAXSONGS + 1];
  int n;
  char buf[256],cbuf[256];

  memset( songs , 0, sizeof(songs));
  if(!fp) return;
  if(!(fo=fopen(FN_TOPSONG,"w"))) {fclose(fp); return;}

  totalcount=0;
  while(fgets(buf,200,fp))
    {
     strtok(buf,"\n");
     strip_blank(cbuf,buf);
     for(n=0;n<MAXSONGS && songs[n].name[0] ;n++)
        {
          if(!strcmp(songs[n].cname,cbuf)) break;
        }
     strcpy(songs[n].name,buf);
     strcpy(songs[n].cname,cbuf);
     songs[n].count++;
     totalcount++;
    }
  qsort(songs, MAXSONGS, sizeof(songcmp), count_cmp);
  fprintf(fo,
"    [36m¢w¢w[37m¦W¦¸[36m¢w¢w¢w¢w¢w¢w[37mºq"
"  ¦W[36m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[37m¦¸¼Æ[36m"
"¢w¢w[32m¦@%d¦¸[36m¢w¢w[m\n",totalcount);
  for(n=0;n<100 && songs[n].name[0];n++)
    {
        fprintf(fo,"      %5d. %-38.38s %4d [32m[%.2f][m\n",n+1,
           songs[n].name,songs[n].count,(float) songs[n].count/totalcount);
    }
  fclose(fp);
  fclose(fo);
}

char *onlydate(time_t clock)
{
  static char foo[9];
  struct tm *mytm = localtime(&clock);

  strftime(foo, 9, "%D", mytm);
  return (foo);
}

int
ordersong()
{
  char destid[IDLEN+1],buf[256],genbuf[256],filename[256],say[51];
  char receiver[60];
  FILE *fp,*fp1;
  fileheader mail;
  time_t now=time(NULL);
  int lines = 0;

  if(answer("¬O§_­n¹w¥ýÂsÄýºq¥»? (y/N)") == 'y')
    a_menu("ÂIºqºq¥»",KTV,0);
  strcpy(buf,onlydate(now));
  if(check_money(5,GOLD)) return 0;
  if(lockutmpmode(OSONG)) return 0;
  move(12,0);
  clrtobot();
  sprintf(buf,"¿Ë·Rªº %s Åwªï¨Ï¥Î­·¹ÐÂIºq¨t²Î\n",cuser.userid);
  outs(buf);
  getdata(13, 0,"­nÂIµ¹½Ö©O:",destid, IDLEN+1, DOECHO,0);

  if(!destid[0]) { unlockutmpmode(); return 0; }

  getdata(14, 0,"·Q­n­n¹ï¥L(¦o)»¡..:",say, 51, DOECHO,"§Ú·R©p..");
  sprintf(save_title,"%s:%s",cuser.userid,say);
  getdata(16, 0, "±H¨ì½Öªº«H½c(¥i¥ÎE-mail)?", receiver, 45,LCECHO,destid);

  pressanykey("±µµÛ­n¿ïºqÅo..¶i¤Jºq¥»¦n¦nªº¿ï¤@­ººq§a..^o^");
  a_menu("ÂIºqºq¥»",SONGBOOK,0);

  if(!trans_buffer[0] || !(fp = fopen(trans_buffer, "r"))) 
    {unlockutmpmode(); return 0;}

  strcpy(filename, SONGPATH);
  stampfile(filename, &mail);
  unlink(filename);
  if(!(fp1 = fopen(filename, "w"))) 
  {
    fclose(fp); 
    unlockutmpmode(); 
    return;
  }

  strcpy(mail.owner, "ÂIºq¾÷");
  sprintf(mail.title,"¡º %s ÂIµ¹ %s ",cuser.userid,destid);
  mail.savemode = 0;

  while (lines++ <= 11)
  {
     char *po;

     if(!fgets(buf,256,fp)) strcpy(buf,"\n");
     if(lines == 11)   // mdify by wildcat:§â³o¦æª½±µ¥[¶iºq¦±ªº³Ì«á¤@¦æ :)
       strcpy(buf,
         "  [1;36m<~Src~>[37m ­n¹ï [33m<~Des~> [37m»¡¡G[32m<~Say~>[m\n");
     while (po = strstr(buf, "<~Src~>"))
        {
           po[0] = 0;
           sprintf(genbuf,"%s%s%s",buf,cuser.userid,po+7);
           strcpy(buf,genbuf);
        }
     while (po = strstr(buf, "<~Des~>"))
        {
           po[0] = 0;
           sprintf(genbuf,"%s%s%s",buf,destid,po+7);
           strcpy(buf,genbuf);
        }
     while (po = strstr(buf, "<~Say~>"))
        {
           po[0] = 0;
           sprintf(genbuf,"%s%s%s",buf,say,po+7);
           strcpy(buf,genbuf);
        }        
     fputs(buf,fp1);
   }
  fclose(fp1);
  fclose(fp);

  if(rec_add(SONGPATH"/.DIR", &mail, sizeof(mail))!=-1)
    degold(5);
  strcpy(mail.owner, "ÂIºq¾÷");
  sprintf(save_title,"%s:%s",cuser.userid,say);
  hold_mail(filename,destid);
  if(receiver[0])
    bbs_sendmail(filename, save_title, receiver);
  clear();
  outs(
   "\n\n  ®¥³ß±zÂIºq§¹¦¨Åo..\n"
       "  ¤@¤p®É¤º°ÊºA¬ÝªO·|¦Û°Ê­«·s§ó·s\n"
       "  ¤j®a´N¥i¥H¬Ý¨ì±zÂIªººqÅo\n\n"
       "  ÂIºq¦³¥ô¦ó°ÝÃD¥i¥H¨ìNoteªOªººëµØ°Ï§äµª®×\n"
       "  ¤]¥i¦bNoteªOºëµØ°Ï¬Ý¨ì¦Û¤vªºÂIºq°O¿ý\n"
       "  ¦³¥ô¦ó«O¶Qªº·N¨£¤]Åwªï¨ìNoteªO¯d¸Ü\n"
       "  Åý¿Ë¤Áªº¯¸ªø¬°±zªA°È\n");
  pressanykey(NULL);
  sortsong();
  topsong();
  unlockutmpmode();
  return 1;
}
