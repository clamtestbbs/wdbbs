/*-------------------------------------------------------*/
/* stuff.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : utility routines                             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"
#include <stdarg.h>

void
setuserfile(buf, fname)
  char *buf, *fname;
{
  sprintf(buf, "home/%s/%s", cuser.userid, fname);
}


void
setbdir(buf, boardname)
  char *buf, *boardname;
{
  sprintf(buf, "boards/%s/%s", boardname,
    currmode & MODE_DIGEST ? fn_mandex : ".DIR");
}


int
invalid_fname(str)
  char *str;
{
  char ch;

  if (strspn(str, ".") == strlen(str))
     return 1;

  while (ch = *str++)
  {
    if (not_alnum(ch) && !strchr("@[]-._", ch))
      return 1;
  }
  return 0;
}


int
invalid_pname(str)
  char *str;
{
  char *p1, *p2, *p3;

  p1 = str;
  while (*p1) 
  {
    if (!(p2 = strchr(p1, '/')))
      p2 = str + strlen(str);
    
    if (p1 + 1 > p2 || p1 + strspn(p1, ".") == p2)
      return 1;
    
    for (p3 = p1; p3 < p2; p3++)
      if (not_alnum(*p3) && !strchr("@[]-._", *p3))
        return 1;
    
    p1 = p2 + (*p2 ? 1 : 0);
  }
  return 0;
}



int
valid_ident(ident)
  char *ident;
{
  static char *invalid[] = {"unknown@", "root@", "gopher@", "bbs@",
  "@bbs", "guest@", "@ppp", "@slip", NULL};
  char buf[512];
  int i;

  str_lower(buf, ident);
  for (i = 0; invalid[i]; i++)
    if (strstr(buf, invalid[i]))
      return 0;
  return 1;
}


/*
woju
*/
int
userid_is_BM(userid, list)
  char *userid, *list;                  /* 板主：BM list */
{
  register int ch, len;

  ch = list[0];
  if ((ch > ' ') && (ch < 128))
  {
    len = strlen(userid);
    do
    {
      if (!ci_strncmp(list, userid, len))
      {
        ch = list[len];
        if ((ch == 0) || (ch == '/') || (ch == ']'))
          return 1;
      }
      while (ch = *list++)
      {
        if (ch == '/')
          break;
      }
    } while (ch);
  }
  return 0;
}


int
is_BM(list)
  char *list;
{
  return userid_is_BM(cuser.userid, list);
}


/* ----------------------------------------------------- */
/* 檔案檢查函數：檔案、目錄、屬於                        */
/* ----------------------------------------------------- */


off_t
dashs(fname)
  char *fname;
{
  struct stat st;

  if (!stat(fname, &st))
        return (st.st_size);
  else
        return -1;
}


long
dasht(fname)
  char *fname;
{
  struct stat st;

  if (!stat(fname, &st))
        return (st.st_mtime);
  else
        return -1;
}


int
dashl(fname)
  char *fname;
{
  struct stat st;

  return (lstat(fname, &st) == 0 && S_ISLNK(st.st_mode));
}


dashf(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}


int
dashd(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}


int
belong(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[80], *ptr;

    while (fgets(buf, 80, fp))
    {
      if ((ptr = strtok(buf, str_space)) && !strcasecmp(ptr, key))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}


int
belong_spam(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[STRLEN], *ptr;

    while (fgets(buf, STRLEN, fp))
    {
      if(buf[0] == '#') continue;
      if ((ptr = strtok(buf, " \t\n\r")) && strstr(key, ptr))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}


char *
Cdatelite(clock)
  time_t *clock;
{
  static char foo[18];
  struct tm *mytm = localtime(clock);

  strftime(foo, 18, "%D %T", mytm);
  return (foo);
}


char *
Cdate(clock)
  time_t *clock;
{
  static char foo[18];
  struct tm *mytm = localtime(clock);

  strftime(foo, 18, "%D %T %a", mytm);
  return (foo);
}

#if 0
void
pressanykey(va_alist)
  va_dcl
{
  va_list ap;
  char msg[512], *fmt;
  int ch;

  msg[0]=0;
  va_start(ap);
  fmt = va_arg(ap, char *);
  if(fmt) vsprintf(msg, fmt, ap);
  va_end(ap);
  if (msg[0])
  {
    move(b_lines, 0); clrtoeol();
#ifdef HYPER_BBS
    prints(COLOR1"\x1b[200m\x1b[300m\x1b[302m\x1b[445m\x1b[1m★ \x1b[37m%-54s  "COLOR2"[空白]或 ESC_c暫存 \x1b[m\x1b[201m", msg);
#else
    prints(COLOR1"\x1b[1m★ \x1b[37m%-54s  "COLOR2"[空白]或 ESC_c暫存 \x1b[m", msg);
#endif
  }
  else
#ifdef HYPER_BBS
    outmsg(COLOR1"\x1b[200m\x1b[300m\x1b[302m\x1b[445m\x1b[1m                        ★ 請按 \x1b[37m(Space/Return)"
    COLOR1" 繼續 ★                         \x1b[m\x1b[201m");
#else
    outmsg(COLOR1"\x1b[1m                        ★ 請按 \x1b[37m(Space/Return)"
    COLOR1" 繼續 ★                         \x1b[m");
#endif
  do
  {
    ch = igetkey();
    if (ch == KEY_ESC && KEY_ESC_arg == 'c')
      capture_screen();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  move(b_lines, 0);
  clrtoeol();
  refresh();
}
#endif

void
pressanykey(char *fmt, ...)
{
  va_list ap;
  char msg[512];
  int ch;

  msg[0]=0;

  va_start(ap, fmt);
  if(fmt)
    vsprintf(msg, fmt, ap);
  va_end(ap);
  if (msg[0])
  {
    move(b_lines, 0); clrtoeol();
#ifdef HYPER_BBS
    prints(COLOR1"\x1b[200m\x1b[300m\x1b[302m\x1b[445m\x1b[1m★ \x1b[37m%-54s  "COLOR2"[空白]或 ESC_c暫存 \x1b[m\x1b[201m", msg);
#else
    prints(COLOR1"\x1b[1m★ \x1b[37m%-54s  "COLOR2"[空白]或 ESC_c暫存 \x1b[m", msg);
#endif
  }
  else
#ifdef HYPER_BBS
    outmsg(COLOR1"\x1b[200m\x1b[300m\x1b[302m\x1b[445m\x1b[1m                        ★ 請按 \x1b[37m(Space/Return)"
    COLOR1" 繼續 ★                         \x1b[m\x1b[201m");
#else
    outmsg(COLOR1"\x1b[1m                        ★ 請按 \x1b[37m(Space/Return)"
    COLOR1" 繼續 ★                         \x1b[m");
#endif
  do
  {
    ch = igetkey();
    if (ch == KEY_ESC && KEY_ESC_arg == 'c')
      capture_screen();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  move(b_lines, 0);
  clrtoeol();
  refresh();
}


void
bell()
{
  char sound[3], *ptr;

  ptr = sound;
  memset(ptr, Ctrl('G'), sizeof(sound));
  if (HAS_HABIT(HABIT_BELL))
    write(1, ptr, sizeof(sound));
}


int
search_num(ch, max)
{
  int clen = 1;
  int x, y;
  extern unsigned char scr_cols;
  char genbuf[10];

  outmsg("\x1b[7m 跳至第幾項：\x1b[0m");
  outc(ch);
  genbuf[0] = ch;
  getyx(&y, &x);
  x--;
  while ((ch = igetch()) != '\r')
  {
    if (ch == 'q' || ch == 'e')
      return -1;
    if (ch == '\n')
      break;
    if (ch == '\177' || ch == Ctrl('H'))
    {
      if (clen == 0)
      {
        bell();
        continue;
      }
      clen--;
      move(y, x + clen);
      outc(' ');
      move(y, x + clen);
      continue;
    }
    if (!isdigit(ch))
    {
      bell();
      continue;
    }
    if (x + clen >= scr_cols || clen >= 6)
    {
      bell();
      continue;
    }
    genbuf[clen++] = ch;
    outc(ch);
  }
  genbuf[clen] = '\0';
  move(b_lines, 0);
  clrtoeol();
  if (genbuf[0] == '\0')
    return -1;
  clen = atoi(genbuf);
  if (clen == 0)
    return 0;
  if (clen > max)
    return max;
  return clen - 1;
}


void
stand_title(title)
  char *title;
{
  clear();
  prints(COLOR1"\x1b[1m【 %s 】\x1b[m\n", title);
}


/* opus : cursor position */

void
cursor_show(row, column)
  int row, column;
{
  FILE *fp;
  int i;
  char *CUR="喵咩魚妖夢",buf[512];

  sethomefile(buf,cuser.userid,"cursor");
  if(dashf(buf))
  {
    if (fp = fopen(buf,"r"))
      fscanf(fp, "%s", buf);
    if (strlen(buf) >= 2)
      CUR = buf;
    fclose(fp);
  }
  i = (rand()%(strlen(CUR)/2))*2;
  move(row, column);
  prints("%c%c", CUR[i], CUR[i+1]);
  move(row, column + 1);
}


void
cursor_clear(row, column)
  int row, column;
{
  move(row, column);
  outs(STR_UNCUR);
}


int
cursor_key(row, column)
  int row, column;
{
  int ch;

  cursor_show(row, column);
  ch = igetkey();
  move(row, column);
  outs(STR_UNCUR);
  return ch;
}


void
printdash(mesg)
  char *mesg;
{
  int head = 0, tail;

  if (mesg)
    head = (strlen(mesg) + 1) >> 1;

  tail = head;

  while (head++ < 38)
    outch('-');

  if (tail)
  {
    outch(' ');
    outs(mesg);
    outch(' ');
  }

  while (tail++ < 38)
    outch('-');
  outch('\n');
}


// wildcat : 兩種貨幣
int
check_money(unsigned long int money,int mode)
{
  unsigned long int usermoney;

  usermoney = mode ? cuser.goldmoney : cuser.silvermoney;
  if(usermoney<money)
  {
    move(1,0);
    clrtobot();
    move(10,10);
    pressanykey("抱歉！您身上只有 %d 元，不夠唷！",usermoney);
    return 1;
  }
  return 0;
}


// wildcat : 經驗值
int
check_exp(unsigned long int exp)
{
  if(cuser.exp<exp)
  {
    move(1,0);
    clrtobot();
    move(10,10);
    pressanykey("抱歉！您的經驗值只有 %d 點，不夠唷！",cuser.exp);
    return 1;
  }
  return 0;
}

// wildcat : 千禧獎金 :p
void
get_bonus()
{
  int money;
  time_t now = time(0);
  char buf[512];
  
  money= random()%2001;
  xuser.silvermoney +=money;
  sprintf(buf,"\x1b[1;31m恭喜 \x1b[33m%s \x1b[31m獲得千禧獎金 \x1b[36m%d 元銀幣 , %s\x1b[m",
    cuser.userid,money,Etime(&now));
  f_cat(BBSHOME"/log/y2k_bonus",buf);

  pressanykey("恭喜你獲得 %d 元銀幣的千禧獎金",money);
}  

/* wildcat 981218 */
#define INTEREST_TIME	86400*7	// wildcat:7天發放一次利息
#define BANK_RATE	1.06	// wildcat:銀行利率 1.06 
void
update_data()
{
  int add = (time(0) - update_time)/30;

  getuser(cuser.userid);

  if((time(0) - xuser.dtime) >= INTEREST_TIME && xuser.silvermoney)
  {
    xuser.silvermoney *= BANK_RATE;
    xuser.dtime = time(0);
  }  
  if(add)
  {
    rpgtmp.age += add;
    xuser.exp += rpgtmp.race != 3 ? add*5 : add*5*rpgtmp.level;
    xuser.silvermoney += rpgtmp.race != 3 ? add*5 : add*10;
    xuser.totaltime += (time(0)-update_time);
    if(rpgtmp.hp && rpgtmp.hp < rpgtmp.con*30)
      rpgtmp.hp += 3*add;
    if(rpgtmp.hp > rpgtmp.con*30)
      rpgtmp.hp = rpgtmp.con*30;  
    if(rpgtmp.mp < rpgtmp.wis*10)
      rpgtmp.mp += add;
    update_time = time(0);
  }
  cuser = xuser;
  rpguser = rpgtmp;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  rpg_rec(xuser.userid,rpguser);
}


int
show_file(char *filename, int y, int lines, int mode)
{
  FILE *fp;
  char buf[512];
  clrchyiuan(y, y + lines);
  move(y, 0);
  if ((fp = fopen(filename, "r")))
  {
    while(fgets(buf,512,fp) && lines--)
      outs(Ptt_prints(buf,mode));
    fclose(fp);
  }
  else 
    return 0;

  return 1;
}


/*---------------------------------------------------------------------*/
/* int chyiuan_ansi(buf,str,max)的用法:自畫像用                        */
/* buf:chyiuan_ansi過後的string                                        */
/* str:chyiuan_ansi之前的string                                        */
/* count:傳回move時應該shift的數值                                     */
/* 備註:如果是彩色模式, 超過限制值時, 會砍掉字串超過部份, 但保留color  */
/*---------------------------------------------------------------------*/

int
chyiuan_ansi(buf,str,max)
  char *buf,*str;
  int max;
{
  int count = 0;
  int count0 = 0;
  int count1 = 0;
  char buf0[512];
  count0 = strip_ansi(buf0,str,0);
  if((cuser.uflag & COLOR_FLAG) && count0 <= max)
  {
    count1=strip_ansi(NULL,str,1);
    count=count1-count0;
    strcpy(buf, str);
  }
  else if((cuser.uflag & COLOR_FLAG) && count0 > max)
  {
    count0 = cut_ansistr(buf0,str,max);
    count1 = strip_ansi(NULL,buf0,1);
    count=count1-count0;
    strcpy(buf, buf0);
  }
  else
  {
    count=0;
    strcpy(buf, buf0);
  }
  return count;
}


int
answer(char *s)
{
  char ch;
  outmsg(s);
  ch = igetch ();
  if (ch == 'Y')
    ch = 'y';
  if (ch == 'N')
    ch = 'n';
  return ch;
}

#if defined(SunOS) || defined(SOLARIS)

#include <syslog.h>

void
setenv(name, value, overwrite)
  const char *name;
  const char *value;
  int overwrite;
{
  if (overwrite || (getenv(name) == 0))
  {
    char *p;

    if ((p = malloc(strlen(name) + strlen(value) + 2)) == 0)
    {
      syslog(LOG_ERR, "out of memory\n");
      exit(1);
    }
    sprintf(p, "%s=%s", name, value);
    putenv(p);
  }
}

atexit(procp)
void (*procp)();
{
   on_exit(procp, 0);
}

#endif

capture_screen()
{
   char fname[512];
   FILE* fp;
   extern screenline *big_picture;
   extern uschar scr_lns;
   int i;

   setuserfile(fname, "buf.0");
   if (fp = fopen(fname, "w")) {
      for (i = 0; i < scr_lns; i++)
         fprintf(fp, "%.*s\n", big_picture[i].len, big_picture[i].data);
      fclose(fp);
   }
}


edit_note()
{
   char fname[512];
   int mode0 = currutmp->mode;
   int stat0 = currstat;
   char c0 = *quote_file;

   *quote_file = 0;
   setutmpmode(NOTE);
   setuserfile(fname, "note");
   vedit(fname, 0);
   currutmp->mode = mode0;
   currstat = stat0;
   *quote_file = c0;
}


char*
my_ctime(const time_t *t)
{
  struct tm *tp;
  static char ans[100];

  tp = localtime(t);
  sprintf(ans, "%d/%02d/%02d %02d:%02d:%02d", tp->tm_year % 100,
     tp->tm_mon + 1,tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
  return ans;
}
#if 0
/* ----------------------------------------------------- */
/* 暫存檔 TBF (Temporary Buffer File) routines           */
/* ----------------------------------------------------- */

char *
tbf_ask()
{
  static char fn_tbf[10] = "buf.0";
  getdata(b_lines, 0, "請選擇暫存檔(0-9)：", fn_tbf + 4, 2, DOECHO,"0");
  return fn_tbf;
}
#endif


int
inexp(unsigned long int exp)
{
  update_data();
  if(belong(BBSHOME"/game/rpg/baduser",cuser.userid)) return -1;  
  cuser.exp = xuser.exp + exp;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  return cuser.exp;
}

int
deexp(unsigned long int exp)
{
  update_data();
  if(xuser.exp <= exp) cuser.exp=0;
  else cuser.exp = xuser.exp - exp;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  return cuser.exp;
}


/*
 * check_personal_note() 的功用跟 chkmail() 一樣..
 * 所以可以加在 my_query() 中:這樣別人query時就可以看到有沒有新留言
 * 還有可以加在 show_title() 中:這樣有新留言時就會像有新信件一樣在title提示喔!
 */

int
check_personal_note(int newflag, char* userid) {
 char fpath[512];
 FILE *fp;
 int  total = 0;
 notedata myitem;
 char *fn_note_dat      = "pnote.dat";

 if (userid == NULL)
   setuserfile(fpath, fn_note_dat);
 else
   sethomefile(fpath, userid, fn_note_dat);

 if ((fp = fopen(fpath, "r")) != NULL) {
   while (fread(&myitem, sizeof(myitem), 1, fp) == 1) {
     if (newflag)
       if (myitem.buf[0][0] == 0) total++;
     else
       total++;
   }
   fclose(fp);
   return total;
 }
 return 0;
}

game_log(char *fmt,...)
{
 va_list ap;
 int file;
 char msg[512], ff[40];
 time_t now;
 FILE *fs;

 va_start(ap, file);
  vsprintf(msg, fmt, ap);
 va_end(ap);

 switch(file) /* 這一段可以自己改! */
 {
  case XAXB:
    strcpy(ff,"log/ab.log");
    log_usies("XAXB",NULL);
    break;
  case CHICKEN: 
    strcpy(ff,"log/pip.log"); 
    log_usies("CHICKEN",NULL);
    break;
  case BLACKJACK: 
    strcpy(ff,"log/bj.log"); 
    log_usies("BJ",NULL);
    break;
  case STOCK: 
    strcpy(ff,"log/stock.log"); 
    log_usies("STOCK",NULL);
    break;
  case DICE: 
    strcpy(ff,"log/dice.log"); 
    log_usies("DICE",NULL);
    break;
  case GP: 
    strcpy(ff,"log/gp.log"); 
    log_usies("GP",NULL);
    break;
  case MARIE: 
    strcpy(ff,"log/marie.log"); 
    log_usies("MARIE",NULL);
    break;
  case RACE: 
    strcpy(ff,"log/race.log"); 
    log_usies("RACE",NULL);
    break;
  case BINGO: 
    strcpy(ff,"log/bingo.log"); 
    log_usies("BINGO",NULL);
    break; 
  case NINE: 
    strcpy(ff,"log/nine.log"); 
    log_usies("NINE",NULL);
    break;
  case NumFight: 
    strcpy(ff,"log/fightnum.log"); 
    log_usies("NumFight",NULL);
    break;
  case CHESSMJ: 
    strcpy(ff,"log/chessmj.log"); 
    log_usies("CHESSMJ",NULL);
    break;
  case SEVENCARD: 
    strcpy(ff,"log/seven.log"); 
    log_usies("SEVENCARD",NULL);
    break;
 }
 fs=fopen(ff,"a+");
 now=time(0);
 fprintf(fs,"\x1b[1;33m%s \x1b[32m%s \x1b[36m%s\x1b[m\n", Etime(&now),cuser.userid,msg);
 fclose(fs);
}

int
show_help(mode)
  int mode;
{
  if(inmore)
    more(BBSHOME"/etc/help/MORE.help",YEA);
  else if(mode == LUSERS)
    more(BBSHOME"/etc/help/LUSERS.help",YEA);
  else if(mode == READBRD || mode == READNEW)
    more(BBSHOME"/etc/help/BOARD.help",YEA);
  else if(mode == READING)
    more(BBSHOME"/etc/help/READ.help",YEA);
  else if(mode == ANNOUNCE)
    more(BBSHOME"/etc/help/ANNOUNCE.help",YEA);
  else if(mode == RMAIL)
    more(BBSHOME"/etc/help/MAIL.help",YEA);
  else if(mode == EDITING)
    more(BBSHOME"/etc/help/EDIT.help",YEA);
  else
    HELP();
  return 0;
}
void
sysop_bbcall(char *fmt,...)
{
  va_list ap;
  FILE *fp;
  char NumMode[10][5],PagerNum[10][10];
  int i=0,j;
  char msg[512], buf[512];

  msg[0]=0;
  va_start(ap, fmt);
  if(fmt)
    vsprintf(msg, fmt, ap);
  va_end(ap);
    
  if(fp=fopen(BBSHOME"/etc/sysop_bbcall","r"))
  {
    while(fgets(buf,512,fp))
    {
      if(buf[0] == '#') continue; 
      buf[strlen(buf) - 1] = '\0';   
      strncpy(NumMode[i],buf+1,3);
      NumMode[i][3] = '\0';  
      strncpy(PagerNum[i],buf+4,6);
      PagerNum[i][6] = '\0';  
      i++;
    }
    fclose(fp);
  }
  for(j = 0 ; i > j; j++)
  {
    DL_func("SO/bbcall.so:bbcall",atoi(NumMode[j]),1,PagerNum[j],0,msg);
  }
}

void
user_bbcall(char *fmt,...)
{
  va_list ap;
  char NumMode[10],PagerNum[10];
  char msg[512];

  msg[0]=0;
  va_start(ap, fmt);
  if(fmt) 
    vsprintf(msg, fmt, ap);
  va_end(ap);
    
  sprintf(NumMode,"0%d",cuser.pagermode);
  strncpy(PagerNum,cuser.pagernum,6);
  PagerNum[6] = '\0';  

  DL_func("SO/bbcall.so:bbcall",atoi(NumMode),1,PagerNum,0,msg);
}

int
mail2user(userec muser,char *title,char *fname)
{
  fileheader mhdr;
  char buf[512], buf1[80];

  sethomepath(buf1, muser.userid);
  stampfile(buf1, &mhdr);
  strcpy(mhdr.owner, cuser.userid);
  strcpy(mhdr.title, title);
  mhdr.savemode = 0;
  mhdr.filemode = 0;
  sethomedir(buf, muser.userid);
  rec_add(buf, &mhdr, sizeof(mhdr));
  f_cp(fname, buf1, O_TRUNC);
  return 0;
}

void
debug(mode)
  char *mode;
{
  time_t now = time(0);
  char buf[512];

  sprintf(buf, "%s %s %s\n", Etime(&now), mode, cuser.userid);      
  f_cat("debug",buf);
}

void
Security (x, y, sysopid, userid)
     int x, y;
     char *sysopid, *userid;
{
  FILE *fp = fopen ("etc/security", "w");
  fileheader fhdr;
  time_t now = time (0);
  char genbuf[512], reason[30];
  int i, flag = 0;
  for (i = 4; i < NUMPERMS; i++)
    {
      if (((x >> i) & 1) != ((y >> i) & 1))
	{
	  if (!flag)
	    {
	      now = time (NULL) - 6 * 60;
	      sprintf (genbuf, "作者: [系統安全局] 看板: Security\n");
	      strcat (genbuf, "標題: [公安報告] 站長修改權限報告\n");
	      fputs (genbuf, fp);
	      sprintf (genbuf, "時間: %s\n", ctime (&now));
	      fputs (genbuf, fp);
	    }
	  sprintf (genbuf, "   站長\x1b[1;32m%s%s%s%s\x1b[m的權限\n", sysopid, (((x >> i) & 1) ? "\x1b[1;33m關閉" : "\x1b[1;33m開啟"), userid, permstrings[i]);
	  fputs (genbuf, fp);
	  flag++;
	}
    }
  if (flag)
    {
      clrtobot ();
      clear ();
      while (!getdata (5, 0
		       ,"請輸入理由以示負責：", reason, 60, DOECHO
		       ,"看版版主:"));
      sprintf (genbuf, "\n   \x1b[1;37m站長%s修改權限理由是：%s\x1b[m", cuser.userid, reason);
      fputs (genbuf, fp);
      fclose (fp);
      sprintf (genbuf, "boards/%s", "Security");
      stampfile (genbuf, &fhdr);
//      rename ("etc/security", genbuf);
      f_mv ("etc/security", genbuf);

      sprintf (fhdr.title, "[公安報告] 站長修改%s權限報告", userid);
      strcpy (fhdr.owner, "[系統安全局]");
      setbdir(genbuf, "Security");
//      sprintf (genbuf, "boards/%s/.DIR", "Security");
      rec_add (genbuf, &fhdr, sizeof (fhdr));
    }
}


