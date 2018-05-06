#include "bbs.h"
#define ba rpguser.race == 6 ? 10*rpguser.level : 5
char *betname[8] = {"¯h­Â³¥¿ß","¨S¤O«§«§","¹C»î¤p­·","Å]¤O¤p¹Ú",
                    "¶Ô¾Ä¤j³½","½XÀYªü­Û","¤@°ï¿ÂÃÆ","¼T­ù©BÂP"};

/* ----------------------------------- */
/* gamble.c ­ì§@ªÌ: Ptt §ï¼gªÌ: yychen */
/*                                     */
/* ----------------------------------- */
show_bet()
{
  FILE *fp = fopen(FN_TICKET_RECORD,"r");
  int i, total=0, ticket[8] = {0,0,0,0,0,0,0,0};

  if(fp)
        {
          fscanf(fp,"%9d %9d %9d %9d %9d %9d %9d %9d\n",
                 &ticket[0],&ticket[1],&ticket[2],&ticket[3],
                 &ticket[4],&ticket[5],&ticket[6],&ticket[7]);
          for(i = 0; i < 8; i++)
                total += ticket[i];
          fclose(fp);
        }
  prints("[1;33m1.%-8s:%-9d2.%-8s:%-9d3.%-8s:%-9d4.%-8s:%-9d[m\n"
         "[1;33m5.%-8s:%-9d6.%-8s:%-9d7.%-8s:%-9d8.%-8s:%-9d[m\n"
         "[1;37;44m ¤Uª`©Ò¦³ª÷ÃB: [33m%d00[36m ª÷ [m",
         betname[0], ticket[0], betname[1], ticket[1],
         betname[2], ticket[2], betname[3], ticket[3],
         betname[4], ticket[4], betname[5], ticket[5],
         betname[6], ticket[6], betname[7], ticket[7], total);
}


show_ticket_data()
{
 clear();
 showtitle("¹ï¹ï¼Ö½ä½L", BoardName);
 move(2,0);
 outs(
        "[1;32m³W«h:[m 1.¥iÁÊ¶R¤KºØ¤£¦PÃþ«¬ªº±m²¼¡C¨C±i­nªá100¤¸¡C\n"
        "      2.¨C¤Q¤G¤p®É¶}¼ú¤@¦¸(0:30 12:30)¡C\n"
        "      3.¶}¼ú®É·|©â¥X¤@ºØ±m²¼, ¦³ÁÊ¶R¸Ó±m²¼ªÌ, «h¥i¨ÌÁÊ¶Rªº±i¼Æ§¡¤ÀÁ`½äª÷¡C\n"
        "      4.¨Cµ§¼úª÷¥Ñ¨t²Î©â¨ú5%¤§µ|ª÷¡C\n\n"
        "[1;32m«e´X¦¸¶}¼úµ²ªG:[m" );
 show_file(FN_TICKET, 8, 8,NO_RELOAD);
 move(15,0);
 outs("[1;36m¥Ø«e¤Uª`ª¬ªp:[m\n");
 show_bet();
 move(20,0);
 prints(COLOR1"[1m§A¨­¤W¦³¡G %-10d ¤¸ [m           [m\n",
   cuser.silvermoney);
 prints("[1m½Ð¿ï¾Ü­nÁÊ¶RªººØÃþ(1~8)[Q:Â÷¶}]:[m");
}

int
append_ticket_record(ch,n)
{
  FILE *fp;
  int ticket[8] = {0,0,0,0,0,0,0,0};
  if(fp = fopen(FN_TICKET_USER,"a"))
        {
          fprintf(fp,"%s %d %d\n",cuser.userid,ch,n);
          fclose(fp);
        }

  if(fp = fopen(FN_TICKET_RECORD,"r+"))
        {
          fscanf(fp,"%9d %9d %9d %9d %9d %9d %9d %9d\n",
                    &ticket[0],&ticket[1],&ticket[2],&ticket[3],
                    &ticket[4],&ticket[5],&ticket[6],&ticket[7]);
          ticket[ch] += n;
          rewind(fp);
          fprintf(fp,"%9d %9d %9d %9d %9d %9d %9d %9d\n",
                    ticket[0],ticket[1],ticket[2],ticket[3],
                    ticket[4],ticket[5],ticket[6],ticket[7]);
          fclose(fp);
        }
  else if(fp = fopen(FN_TICKET_RECORD,"w"))
        {
          ticket[ch] += n;
          fprintf(fp,"%9d %9d %9d %9d %9d %9d %9d %9d\n",
                    ticket[0],ticket[1],ticket[2],ticket[3],
                    ticket[4],ticket[5],ticket[6],ticket[7]);
          fclose(fp);
        }
}

void
show_picture(char *filename)
{
 FILE *fp;
 char buf[256];
 move(5,0);
 if((fp=fopen(filename,"r")))
  {
    while(fgets(buf,256,fp))
        {
          outs(buf);
        }
    fclose(fp);
  }
}
void
clear_picture()
{
 char i;
 for (i=5;i<18;i++)
  {
        move(i,0);
        clrtoeol();
  }
}

void
ch_buyitem(money,picture,item)
int money;
char *picture;
int *item;
{
  int num=0;
  char buf[5];
  getdata(b_lines-1,0,"­n¶R¦h¤Ö¥÷©O:",buf,4,DOECHO,0);
  num = atoi(buf);
  if(num<0) num=0;
  if (cuser.silvermoney > money*num)
        {
          *item += num;
          demoney(money*num);
          clear_picture();
          show_picture(picture);
        }
  else
        {
          move(b_lines-1,0);
          clrtoeol();
          outs("²{ª÷¤£°÷ !!!");
        }
  pressanykey(NULL);
}


int
ticket_main()
{
 int ch,n;

 if (lockutmpmode(TICKET))  return 0;

 setutmpmode(TICKET);
 while(1)
  {
   show_ticket_data();
   ch = igetch();
   if(ch=='q' || ch == 'Q') break;
   ch -= '1';
   if(ch > 7 || ch < 0) continue;
   n = 0;
   ch_buyitem(100,"etc/buyticket",&n);
   if(n>0)  append_ticket_record(ch,n);
  }
 unlockutmpmode();
 return 0;
}

int
query_ticket()
{
  FILE *fp;
  int tickets[8] = {0,0,0,0,0,0,0,0};
  int num1, num2, i=0;
  char userid[IDLEN+1];

  if (fp = fopen(FN_TICKET_USER, "r"))
  {
      while ((fscanf(fp, "%s %d %d", userid, &num1, &num2))!=EOF)
      {
        if (!strcasecmp(userid, cuser.userid))
        {
          if (!i)
            i = 1;
          tickets[num1] += num2;
        }
      }
      fclose(fp);
  }
  else
  {
    pressanykey("Âd¥x¤p©j: ¨Ã¨S¦³¥ô¦ó¤@­Ó¤H(¥]¬A±z)©ãª`.");
    return;
  }
  if (!i)
  {
    pressanykey("Âd¥x¤p©j: ©êºp¡A±z¨Ã¨S¦³©ã¥ô¦ó¤@¶µ¡I");
    return;
  }
  clear();
  prints("[1;33mÂd¥x¤p©jµ¹±z¤@±i²M³æ¡G[m\n");
  prints("[32m´ËªL½ä½L¡G[1;32m%s[0;32m ¤Uª`¤@Äýªí[m\n\n",
    cuser.userid);
  for(i = 0; i < 8; i++)
  {
    prints("[1;3%dm±z(%s) ©ã¤F [%d. %-4s]¡G%d ±i[m\n", 
      i+1 <= 7 ? i+1 : i-6,
      cuser.userid, i+1, betname[i],
      tickets[i]);
  }
  pressanykey("Âd¥x¤p©j: ¥H¤W¬O±zªº©ãª`ª¬ªp¡C");
}
