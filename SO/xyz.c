/*-------------------------------------------------------*/
/* xyz.c        ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : system toolbox routines                      */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"

#define MAXPATHLEN	1024

/* ----------------------------------------------------- */
/* ¦UºØ²Î­p¤Î¬ÛÃö¸ê°T¦Cªí                                */
/* ----------------------------------------------------- */

int
show_hint_message()
{
        struct timeval  timep;
        struct timezone timezp;
        int     i, j, msgNum;
        FILE    *hintp;
        char    msg[136];

        if (!(hintp = fopen(BBSHOME"/etc/hint", "r")))
          return 0;
        fgets(msg, 135, hintp);
        msgNum = atoi(msg);
        gettimeofday(&timep, &timezp);
        i = (int) timep.tv_sec%(msgNum + 1); /* ³Ì·sªº¤@½g¾÷·|¥[­¿ */
        if (i == msgNum)
          i--;
        j = 0;

        while (j < i)
        {
          fgets(msg, 135, hintp);
          msg[1] = '\0';
          if (!strcmp(msg,"#"))
            j++;
        }
        move(12, 0);
        clrtobot();
        fgets(msg, 135, hintp);
        prints("[1;36m­·§j¨Óªº®ø®§¡G [1;31m±zª¾¹D¶Ü¡H[40;0m\n");
        prints("                   %s[0m", msg);
        fgets(msg, 135, hintp);
        prints("                   %s[0m", msg);
        pressanykey(NULL);
        fclose(hintp);
}

/* ----------------------------------------------------- */
/* ¤ä´©¥~±¾µ{¦¡ : tin¡Bgopher¡Bwww¡Bbbsnet¡Bgame¡Bcsh    */
/* ----------------------------------------------------- */


#ifdef HAVE_EXTERNAL
#define LOOKFIRST       (0)
#define LOOKLAST        (1)
#define QUOTEMODE       (2)
#define MAXCOMSZ        (1024)
#define MAXARGS         (40)
#define MAXENVS         (20)
#define BINDIR          BBSHOME"/bin/"

char *bbsenv[MAXENVS];
int numbbsenvs = 0;


int
bbssetenv(env, val)
  char *env, *val;
{
  register int i, len;
  extern char *malloc();

  if (numbbsenvs == 0)
    bbsenv[0] = NULL;
  len = strlen(env);
  for (i = 0; bbsenv[i]; i++)
    if (!ci_strncmp(env, bbsenv[i], len))
      break;
  if (i >= MAXENVS)
    return -1;
  if (bbsenv[i])
    free(bbsenv[i]);
  else
    bbsenv[++numbbsenvs] = NULL;
  bbsenv[i] = malloc(strlen(env) + strlen(val) + 2);
  strcpy(bbsenv[i], env);
  strcat(bbsenv[i], "=");
  strcat(bbsenv[i], val);
}
int
do_exec(com, wd)
  char *com, *wd;
{
  char genbuf[200];
  char path[MAXPATHLEN];
  char pcom[MAXCOMSZ];
  char *arglist[MAXARGS];
  char *tz;
  register int i, len;
  register int argptr;
  int status, w;
  pid_t pid;
  int pmode;
  void (*isig) (), (*qsig) ();
  extern int mbbsd;

  if (mbbsd)
  {
    pressanykey("­Y­n¨Ï¥Î¥~±¾µ{¦¡ , ½Ð telnet %s 3000 ¶i¤J¥»¯¸",MYHOSTNAME);
    return;
  }

  strncpy(path, BINDIR, MAXPATHLEN);
  strncpy(pcom, com, MAXCOMSZ);
  len = BMIN(strlen(com) + 1, MAXCOMSZ);
  pmode = LOOKFIRST;
  for (i = 0, argptr = 0; i < len; i++)
  {
    if (pcom[i] == '\0')
      break;
    if (pmode == QUOTEMODE)
    {
      if (pcom[i] == '\001')
      {
        pmode = LOOKFIRST;
        pcom[i] = '\0';
        continue;
      }
      continue;
    }
    if (pcom[i] == '\001')
    {
      pmode = QUOTEMODE;
      arglist[argptr++] = &pcom[i + 1];
      if (argptr + 1 == MAXARGS)
        break;
      continue;
    }
    if (pmode == LOOKFIRST)
      if (pcom[i] != ' ')
      {
        arglist[argptr++] = &pcom[i];
        if (argptr + 1 == MAXARGS)
          break;
        pmode = LOOKLAST;
      }
      else
        continue;
    if (pcom[i] == ' ')
    {
      pmode = LOOKFIRST;
      pcom[i] = '\0';
    }
  }
  arglist[argptr] = NULL;
  if (argptr == 0)
    return -1;
  if (*arglist[0] == '/')
    strncpy(path, arglist[0], MAXPATHLEN);
  else
    strncat(path, arglist[0], MAXPATHLEN);
  reset_tty();
  alarm(0);
  if ((pid = vfork()) == 0)
  {
    if (wd)
      if (chdir(wd))
      {
        fprintf(stderr, "Unable to chdir to '%s'\n", wd);
        exit(-1);
      }
    bbssetenv("PATH", "/bin:.");
    bbssetenv("TERM", "vt100"/* cuser.termtype */);
    bbssetenv("USER", cuser.userid);
    bbssetenv("USERNAME", cuser.username);
    /* added for tin's HOME and EDITOR */
/*
woju
*/
    sprintf(genbuf, BBSHOME"/home/%s", cuser.userid);
    bbssetenv("HOME", genbuf);
    bbssetenv("EDITOR", "/bin/ve");
    /* end */
    /* added for tin's reply to */
    bbssetenv("REPLYTO", cuser.email);
    bbssetenv("FROMHOST", fromhost);
    /* end of insertion */
    if ((tz = getenv("TZ")) != NULL)
      bbssetenv("TZ", tz);
    if (numbbsenvs == 0)
      bbsenv[0] = NULL;
    execve(path, arglist, bbsenv);
    fprintf(stderr, "EXECV FAILED... path = '%s'\n", path);
    exit(-1);
  }
  isig = signal(SIGINT, SIG_IGN);
  qsig = signal(SIGQUIT, SIG_IGN);
  while ((w = wait(&status)) != pid && w != 1)
     /* NULL STATEMENT */ ;
  signal(SIGINT, isig);
  signal(SIGQUIT, qsig);
  restore_tty();

#ifdef DOTIMEOUT
  alarm(IDLE_TIMEOUT);
#endif

  return ((w == -1) ? w : status);
}


int
exec_cmd(umode, pager, cmdfile, mesg)
  char *cmdfile, *mesg;
{
  char buf[64];
  int save_pager;

  if (!dashf(cmdfile))
  {
    move(2, 0);
    prints("«Ü©êºp, ¥»¯¸¤£´£¨Ñ %s (%s) ¥\\¯à.", mesg, cmdfile);
    return 0;
  }
  save_pager = currutmp->pager;
  if (pager == NA)
  {
    currutmp->pager = pager;
  }
  setutmpmode(umode);
  sprintf(buf, "/bin/sh %s", cmdfile);
  reset_tty();
  do_exec(buf, NULL);
  restore_tty();
  currutmp->pager = save_pager;
  clear();
  return 0;
}

#ifdef  HAVE_ADM_SHELL
x_csh()
{
  int save_pager;

  save_pager = currutmp->pager;
  currutmp->pager = 2;
  clear();
  refresh();
  reset_tty();
#ifdef SYSV
  do_exec("sh", NULL);
#else
  do_exec("tcsh", NULL);
#endif

  restore_tty();
  currutmp->pager = save_pager;
  clear();
  return 0;
}
#endif                          /* NO_ADM_SHELL */



#ifdef  HAVE_ARCHIE
x_archie()
{
    char buf[STRLEN], ans[4];
    char genbuf1[100], genbuf2[200];
    char *s;
    setutmpmode(ARCHIE);
    clear();
    outs("\nÅwªï¥úÁ{¡i[1;33;44m§å½ð½ð¹ê·~§{[m¡j¨Ï¥Î [32mARCHIE[m ¥\\¯à\n");
    outs("\n¥»¥\\¯à±N¬°±z¦C¥X¦b­þ­Ó FTP ¯¸¦s¦³±z±ý´M§äªºÀÉ®×.\n");
    outs("\n½Ð¿é¤J±ý·j´Mªº¦r¦ê, ©Îª½±µ«ö <ENTER> ¨ú®ø¡C\n");
    outs("\n                            coder by Harimau\n");
    outs("                              modified by Leeym\n");
    getdata(13,0,"·j´M¦r¦ê¡G",buf,20,DOECHO,0);
    if (buf[0]=='\0')
      {
      pressanykey("¨ú®ø·j´M.....");
      return;
      }
    for(s=buf;*s != '\0';s++)
        {
        if(isspace(*s))
            {
            pressanykey("¤@¦¸¥u¯à·j´M¤@­Ó¦r¦ê°Õ, ¤£¯à¤Ó³g¤ß³á!!");
            return;
            }
        }
    bbssetenv("ARCHIESTRING",buf);
    exec_cmd( ARCHIE, YEA, BBSHOME"/bin/archie.sh",ARCHIE);
    log_usies("ARCHIE","");
    strcpy(genbuf1, buf);
    sprintf(buf,BBSHOME"/tmp/archie.%s",cuser.userid);
    if (dashf(buf))
       {
       getdata(0, 0, "­n±Nµ²ªG±H¦^«H½c¶Ü(Y/N)¡H[N]", ans, 3, DOECHO,0);
       if (*ans == 'y')
        {
          fileheader mhdr;
          char title[128], buf1[80];
          FILE* fp;
          sethomepath(buf1, cuser.userid);
          stampfile(buf1, &mhdr);
          strcpy(mhdr.owner, cuser.userid);
          sprintf(genbuf2, "Archie ·j´MÀÉ®×: %s µ²ªG", genbuf1);
          strcpy(mhdr.title, genbuf2);
          mhdr.savemode = 0;
          mhdr.filemode = 0;
          sethomedir(title, cuser.userid);
          rec_add(title, &mhdr, sizeof(mhdr));
          f_cp(buf, buf1,O_TRUNC);
          }
       more( buf, YEA);
       unlink (buf);
       }
}
#endif                          /* HAVE_ARCHIE */
#endif                          /* HAVE_EXTERNAL */

void
x_cdict()
{
    char buf[STRLEN], ans[4];
    char genbuf1[100], genbuf2[200];
    char *s;

    clear();
    counter(BBSHOME"/log/counter/¹q¤l¦r¨å","¬d¶W¯Å¦r¨å",0);
    outs("\n[1mÅwªï¥úÁ{¡i"COLOR1"­·»P¹Ð®Jªº¹ï¸Ü[m¡j¨Ï¥Î [32m­^º~¦r¨å[m ¥\\¯à\n");
    outs("\n½Ð¿é¤J±ý¬d¸ßªº³æ¦r, ©Îª½±µ«ö <ENTER> ¨ú®ø¡C\n");
    getdata(13,0,"¬d¸ß³æ¦r¡G",buf,20,DOECHO,0);
    if (buf[0]=='\0')
      {
      pressanykey("¨ú®ø¬d¸ß.....");
      return;
      }
    for(s=buf;*s != '\0';s++)
        {
        if(isspace(*s))
            {
            pressanykey("¤@¦¸¥u¯à¬d¸ß¤@­Ó³æ¦r°Õ, ¤£¯à¤Ó³g¤ß³á!!");
            return;
            }
        }
    bbssetenv("WORD",buf);
    exec_cmd(CDICT, YEA, BBSHOME"/bin/cdict.sh","CDICT");
    log_usies("CDICT","");
    strcpy(genbuf1, buf);
    sprintf(buf,BBSHOME"/tmp/cdict.%s",cuser.userid);
    if (dashf(buf))
       {
       getdata(0, 0, "­n±Nµ²ªG±H¦^«H½c¶Ü(Y/N)¡H[N]", ans, 3, DOECHO,0);
       if (*ans == 'y')
        {
          fileheader mhdr;
          char title[128], buf1[80];
          sethomepath(buf1, cuser.userid);
          stampfile(buf1, &mhdr);
          strcpy(mhdr.owner, cuser.userid);
          sprintf(genbuf2, "·j´M³æ¦r: %s µ²ªG", genbuf1);
          strcpy(mhdr.title, genbuf2);
          mhdr.savemode = 0;
          mhdr.filemode = 0;
          sethomedir(title, cuser.userid);
          rec_add(title, &mhdr, sizeof(mhdr));
          f_cp(buf, buf1,O_TRUNC);
          }
       more( buf, YEA);
       unlink (buf);
       }
}

#define MAX_SCORE       20                     
struct tetrisrec          /*«XÃ¹´µ¤è¶ô*/       
{                                              
char userid[IDLEN + 1];                      
time_t playtime;                           
char lasthost[16];                       
int u_score;                           
int u_level;                         
int u_rkill;                       
};                                 
typedef struct tetrisrec tetrisrec;

x_tetris()
{
  char buf[64];
  int save_pager;
  clear();
  counter(BBSHOME"/log/counter/«XÃ¹´µ¤è¶ô","ª±«XÃ¹´µ¤è¶ô",0);
  pressanykey(NULL);
  save_pager = currutmp->pager;
  currutmp->pager = 2;
  setutmpmode(TETRIS);
  restore_tty();
  sprintf(buf, BBSHOME"/bin/tetris.sh");
  do_exec(buf, NULL);
  restore_tty();
  refresh();
  game_score(0);
  currutmp->pager = save_pager;
  return 0;
}

int
tetris_cmp(b, a)
struct tetrisrec *a, *b;
{
  return (a->u_score - b->u_score);
}


game_score(swi)
int swi;
{

  FILE *fp;
  int score=0,level=0,rkill=0;
  int i,j;
  struct tetrisrec alltetris[MAX_SCORE+1];

  switch(swi){

    case 0:
       if(fp=fopen(BBSHOME"/etc/tetris.tmp","r"))
       {
          fscanf(fp,"%d %d %d ",&score,&level,&rkill);
          fclose(fp);
       }
       unlink(BBSHOME"/etc/tetris.tmp");
       break;

  }

  switch(swi){
     char genbuf[100];

     case 0:
             if((fp=fopen(BBSHOME"/log/tetris.score","r+")) == NULL )
               fp=fopen(BBSHOME"/log/tetris.score","w");
             for(i=0;i<MAX_SCORE;i++)
                if(fread(&alltetris[i],sizeof(struct tetrisrec),1,fp) == 0)
                    break;

             strcpy(alltetris[i].userid, cuser.userid);
             alltetris[i].playtime = time(0);
             strcpy(alltetris[i].lasthost, cuser.lasthost);
             alltetris[i].u_score = score;
             alltetris[i].u_level = level;
             alltetris[i].u_rkill = rkill;

             qsort(alltetris, i+1, sizeof(struct tetrisrec), tetris_cmp);
             rewind(fp);
             for(j=0;j<i+1 && j<MAX_SCORE;j++)
               fwrite(&alltetris[j],sizeof(struct tetrisrec),1,fp);
             fclose(fp);
             clear();
             prints("yours final score: %d  level: %d  row: %d\n",score,level,rkill);
             prints("%-12s%-9.9s %-10.10s %-5.5s%-17.16s%s\n",
             "    ¨Ï¥ÎªÌ","    Score","    level","row","from","®É¶¡");
            for(j=0;j<i+1 && j<MAX_SCORE;j++)
             {
               sprintf(genbuf,"%-12s%-9.9d %-3.3d    %-4.4d %-17.16s%s",alltetris[j].userid,
               alltetris[j].u_score,alltetris[j].u_level,alltetris[j].u_rkill,
               alltetris[j].lasthost,ctime(&alltetris[j].playtime));
               prints("%02d. %s",j+1,genbuf);
             }
             pressanykey(NULL);
             break;


  }
  return 0;
}

#ifdef HAVE_MJ
void
x_mj()
{
  char buf[64];
  int save_pager;
  if(check_money(100,SILVER)) return;
  counter(BBSHOME"/log/counter/³Â±N","¥´³Â±N",0);
  save_pager = currutmp->pager;
  currutmp->pager = 2;
  setutmpmode(MJ);
  demoney(100);
  reset_tty();
  sprintf(buf, BBSHOME"/bin/qkmj95p4-freebsd 140.112.28.167 7001");
  do_exec(buf, NULL);
  restore_tty();
  currutmp->pager = save_pager;
  clear();
  pressanykey("Åwªï¤U¦¸¦A¨ÓºN¨â°é°Ú!! ^o^");
}
#endif

#ifdef HAVE_BIG2
void
x_big2()
{
  char buf[64];
  int save_pager;
  if(check_money(100,SILVER)) return;
  counter(BBSHOME"/log/counter/¤j¦Ñ¤G","¥´¤j¦Ñ¤G",0);
  save_pager = currutmp->pager;
  currutmp->pager = 2;
  setutmpmode(BIG2);
  demoney(100);
  reset_tty();
  sprintf(buf, BBSHOME"/bin/big2.sh");
  do_exec(buf, NULL);

  restore_tty();
  currutmp->pager = save_pager;
  pressanykey("Åwªï¤U¦¸¦A¨Ó´ê¥|¸}°Ú!! ^o^");
}
#endif

void
x_chess()
{
  char buf[64];
  int save_pager;
  if(check_money(100,SILVER)) return;
  counter(BBSHOME"/log/counter/¶H´Ñ","¤U¶H´Ñ",0);
  save_pager = currutmp->pager;
  currutmp->pager = 2;
  setutmpmode(CHESS);
  demoney(100);
  reset_tty();
#ifdef LINUX
  sprintf(buf, BBSHOME"/bin/ch4");
#else
  sprintf(buf, BBSHOME"/bin/chess");
#endif
  do_exec(buf, NULL);
  restore_tty();
  currutmp->pager = save_pager;
  pressanykey("Åwªï¤U¦¸¦A¨Ó¤U¤U´Ñ!! ^o^");
}

void
x_bbsnet()
{
  char buf[64];
  int save_pager;
  counter(BBSHOME"/log/counter/BBSNET","¥Î BBSNET ",0);
  save_pager = currutmp->pager;
  currutmp->pager = 2;
  setutmpmode(BBSNET);
  reset_tty();
  sprintf(buf, BBSHOME"/bin/bbsnet");
  system(buf);
  restore_tty();
  currutmp->pager = save_pager;
  pressanykey("Åwªï¤U¦¸¦A¨Ó!! ^o^");
}

void
x_logintest()
{
  char buf[64];
  int i;
  setutmpmode(BBSNET);
  demoney(100);
  reset_tty();
  i=rand()%5;
  sprintf(buf, "/bin/telnet -8 wd.twbbs.org 300%d",i+1);
  do_exec(buf, NULL);

  restore_tty();
}

