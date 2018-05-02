/*-------------------------------------------------------*/
/* main.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : BBS main/login/top-menu routines             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#define _MAIN_C_

#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h> 
#include <sys/socket.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <arpa/telnet.h>
#include <syslog.h>
#include "bbs.h"

#define SOCKET_QLEN	4
#define PID_FILE	BBSHOME"/run/mbbsd.pid" 

#define BANNER  "¡i"BOARDNAME"¡j¡· ¹q¤l¥¬§iÄæ¨t²Î ¡· ("MYIP")\r\nPowered by FreeBSD\r\n\n"

jmp_buf byebye;

static uschar enter_uflag;
char genbuf[1024];
char tty_name[20] = "\0";
char fromhost[STRLEN] = "\0";
char remoteusername[40];
int mbbsd = 1; 

static int use_shell_login_mode=0;
void check_register();

/* ----------------------------------------------------- */
/* Â÷¶} BBS µ{¦¡                                         */
/* ----------------------------------------------------- */

void
log_usies(mode, msg)
  char *mode, *msg;
{
  char buf[512], data[256];
  time_t now;
  
  time(&now);
  if (!msg)
  {
    sprintf(data, "Stay: %d (%s)", 
      (now - login_start_time) / 60, cuser.username);
    msg = data;
  }
  sprintf(buf, "%s %s %-13s%s", Etime(&now), mode, cuser.userid, msg);
  f_cat(FN_USIES, buf);
}
                          

static void
setflags(mask, value)
  int mask, value;
{
  if (value)
    cuser.uflag |= mask;
  else
    cuser.uflag &= ~mask;
}


void
u_exit(mode)
  char *mode;
{
  extern void auto_backup();    /* ½s¿è¾¹¦Û°Ê³Æ¥÷ */
  userec xuser;
  int diff = (time(0) - login_start_time) / 60;

  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  auto_backup();
  setflags(PAGER_FLAG, currutmp->pager != 1);
  setflags(CLOAK_FLAG, currutmp->invisible);
  /* alan.010307: «e4bitsÃ±¦W¬ö¿ý, «á4bits pagerª¬ºA */
  xuser.pager = xuser.pager & 0xF0 + currutmp->pager;
//  xuser.pager = currutmp->pager;	 /* °O¿ýpagerª¬ºA, add by wisely */
  xuser.invisible = currutmp->invisible; /* ¬ö¿ýÁô§Îª¬ºA by wildcat */
  xuser.totaltime += time(0) - update_time;
  xuser.numposts = cuser.numposts;
  xuser.feeling[4] = '\0';
  xuser.pagernum[6] = '\0';

  if (!HAS_PERM(PERM_DENYPOST) && !currutmp->invisible)
  {
    char buf[256];
    time_t now;
    
    time(&now);
    sprintf(buf,"<<¤U¯¸³qª¾>> -- §Ú¨«Åo¡I - %s",Etime(&now));
    do_aloha(buf);
  }

  purge_utmp(currutmp);
  if(!diff && cuser.numlogins > 1 && strcmp(cuser.userid,STR_GUEST))
    xuser.numlogins = --cuser.numlogins; /* Leeym ¤W¯¸°±¯d®É¶¡­­¨î¦¡ */
  substitute_record(fn_passwd, &xuser, sizeof(userec), usernum);
  log_usies(mode, NULL);
}


void
system_abort()
{
  if (currmode)
    u_exit("ABORT");

  printf("ÁÂÁÂ¥úÁ{, °O±o±`¨Ó³á !\n");
  sleep(1);
  exit(0);
}


void
abort_bbs()
{
  if (currmode)
    u_exit("AXXED");
  exit(0);
}


void
leave_bbs()
{
   reset_tty();
}


/* ----------------------------------------------------- */
/* µn¿ý BBS µ{¦¡                                         */
/* ----------------------------------------------------- */


int
dosearchuser(userid)
  char *userid;
{
  if (usernum = getuser(userid))
    memcpy(&cuser, &xuser, sizeof(cuser));
  else
    memset(&cuser, 0, sizeof(cuser));
  return usernum;
}


static void
talk_request()
{
#if defined ( __linux__ ) || defined ( __CYGWIN__ )
  /*
   * Linux ¤U³sÄò page ¹ï¤è¨â¦¸´N¥i¥H§â¹ï¤è½ð¥X¥h¡G ³o¬O¥Ñ©ó¬Y¨Ç¨t²Î¤@ nal
   * ¶i¨Ó´N·|±N signal handler ³]©w¬°¤º©wªº handler, ¤£©¯ªº¬O default ¬O±Nµ{
   * erminate. ¸Ñ¨M¤èªk¬O¨C¦¸ signal ¶i¨Ó´N­«³] signal handler
   */

  signal(SIGUSR1, talk_request);
#endif
  bell();
  if (currutmp->msgcount) {
     char buf[512];
     time_t now = time(0);

     sprintf(buf, "[33;41m¡¹%s[34;47m [%s] %s [0m",
        (currutmp->destuip)->userid,  my_ctime(&now),
        (currutmp->sig == 2) ? "­«­n®ø®§¼s¼½¡I(½ÐCtrl-U,l¬d¬Ý¼ö°T°O¿ý)" : "©I¥s¡B©I¥s¡AÅ¥¨ì½Ð¦^µª");
     move(0, 0);
     clrtoeol();
     outs(buf);
     refresh();
  }
  else {
     uschar mode0 = currutmp->mode;
     char c0 = currutmp->chatid[0];
     screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
     currutmp->mode = 0;
     currutmp->chatid[0] = 1;
     vs_save(screen);
     talkreply();
     vs_restore(screen);
     currutmp->mode = mode0;
     currutmp->chatid[0] = c0;
   }
}


show_last_call_in()
{
   char buf[256];
   sprintf(buf, COLOR2"[1m¡¹ [37m%s %s [m",
      currutmp->msgs[0].last_userid,
      currutmp->msgs[0].last_call_in);

   move(b_lines, 0);
   clrtoeol();
   refresh();
   outmsg(buf);
}

msgque oldmsg[MAX_REVIEW];   /* ¥á¹L¥hªº¤ô²y */
char   no_oldmsg=0,oldmsg_count=0;            /* pointer */

static void
write_request()
{
  time_t now;
  extern char watermode;
/*  Half-hour remind  */
  if(*currmsg) {
    outmsg(currmsg);
    refresh();
    bell();
    *currmsg = 0;
    return;
  }

  time(&now);

#if defined ( __linux__ ) || defined ( __CYGWIN__ ) 
  signal(SIGUSR2, write_request);
#endif

  update_data();
  ++cuser.receivemsg;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  bell();
  show_last_call_in();
  memcpy(&oldmsg[no_oldmsg],&currutmp->msgs[0],sizeof(msgque));
  ++no_oldmsg;
  no_oldmsg %= MAX_REVIEW;
  if(oldmsg_count < MAX_REVIEW) oldmsg_count++;
  if(watermode)
  {
    if(watermode<oldmsg_count) watermode++;
    t_display_new();
  }
  refresh();
  currutmp->msgcount = 0;
}

static void
multi_user_check()
{
  register user_info *ui;
  register pid_t pid;
  int cmpuids();

  if (HAS_PERM(PERM_SYSOP))
    return;         /* wildcat:¯¸ªø¤£­­¨î */

  if (cuser.userlevel)
  {
    if (!(ui = (user_info *) search_ulist(cmpuids, usernum)))
      return;                   /* user isn't logged in */

    pid = ui->pid;
    if (!pid || (kill(pid, 0) == -1))
      return;                   /* stale entry in utmp file */

    if (getans("±z·Q§R°£¨ä¥L­«½Æªº login (Y/N)¶Ü¡H[Y] ") != 'n')
    {
      kill(pid, SIGHUP);
      log_usies("KICK ", cuser.username);
    }
    else
    {
      int nums = MULTI_NUMS;
      if(HAS_PERM(PERM_BM))
        nums += 2;
      if (count_multi() >= nums)
        system_abort();
    }
  } 
  else  /* guestªº¸Ü */
  {
    if (count_multi() > 32)
    {
      pressanykey("©êºp¡A¥Ø«e¤w¦³¤Ó¦h guest, ½Ðµy«á¦A¸Õ¡C");
      oflush();
      exit(1);
    }
  }
}

/* --------- */
/* bad login */
/* --------- */
static char str_badlogin[] = "logins.bad";


static void
logattempt(uid, type)
  char *uid;
  char type;                    /* '-' login failure   ' ' success */
{
  char fname[40];
  char genbuf[256];

  sprintf(genbuf, "%c%-12s[%s] %s@%s\n", type, uid,
    Etime(&login_start_time), remoteusername, fromhost);
  f_cat(str_badlogin, genbuf);
  
  if (type == '-')
  {
    sprintf(genbuf, "[%s] %s\n", Etime(&login_start_time), fromhost);
    sethomefile(fname, uid, str_badlogin);

    f_cat(fname, genbuf);
  }
}

#ifdef __FreeBSD__
static int
over_load()
{
  double cpu_load[3];

  getloadavg(cpu_load, 3);

  if(cpu_load[0] > MAX_CPULOAD)
    pressanykey("¥Ø«e¨t²Î­t²ü %f ¹L°ª,½Ðµy«á¦A¨Ó", cpu_load[0]);

  return cpu_load[0];
}
#endif

static void
login_query()
{
  char uid[IDLEN + 1], passbuf[PASSLEN];
  int attempts;
  char genbuf[512];
  extern struct UTMPFILE *utmpshm;

  signal(SIGALRM, abort_bbs);

  resolve_utmp();
  attempts = utmpshm->number;
  clear();
  show_file("etc/Welcome_title",0,10,ONLY_COLOR);
  move(0,0);
  counter(BBSHOME"/log/counter/¤W¯¸¤H¦¸","¥úÁ{¥»¯¸",1);
  show_file("etc/Welcome_news",12,12,ONLY_COLOR);
  signal(SIGALRM, abort_bbs);

  if (attempts >= MAXACTIVE)
  {
    pressanykey("¥Ø«e¯¸¤W¤H¼Æ¤w¹F¤W­­¡A½Ð±zµy«á¦A¨Ó¡C");
    oflush();
    sleep(1);
    exit(1);
  }
#ifdef __FreeBSD__
  if(over_load() > MAX_CPULOAD) 
  {
    oflush();
    sleep(1);
    exit(1); 
  }
#endif
  attempts = 0;
  while (1)
  {
    if (attempts++ >= LOGINATTEMPTS)
    {
      pressanykey("¿ù»~¤Ó¦h¦¸,ÙTÙT~~~~~");
      oflush();
      exit(1);
    }
    uid[0] = '\0';
    getdata(16, 54, "[±zªº±b¸¹] ",uid, IDLEN + 1 , DOECHO, NULL);
    if (ci_strcmp(uid, str_new) == 0)
    {
#ifdef LOGINASNEW
      new_register();
      break;
#else
      outs("¥»¨t²Î¥Ø«eµLªk¥H new µù¥U, ½Ð¥Î guest ¶i¤J\n");
      continue;
#endif
    }
    else if (uid[0] == '\0' || !dosearchuser(uid))
    {
      pressanykey(err_uid);
    }
    else if (strcmp(uid, STR_GUEST))
    {
      getdata(18, 54, "[±zªº±K½X] ", passbuf, PASSLEN, PASS, NULL);
      passbuf[8] = '\0';
      if (!chkpasswd(cuser.passwd, passbuf))
      {
        logattempt(cuser.userid, '-');
        pressanykey(ERR_PASSWD);
      }
      else
      {
        /* SYSOP gets all permission bits */
        if (!ci_strcmp(cuser.userid, str_sysop))
          cuser.userlevel = ~0;
        logattempt(cuser.userid, ' ');
        break;
      }
    }
    else
    {                           /* guest */
      cuser.userlevel = 0;
      cuser.uflag = COLOR_FLAG | PAGER_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
      break;
    }
  }
  signal(SIGALRM, SIG_IGN);

  multi_user_check();
  sethomepath(genbuf, cuser.userid);
  mkdir(genbuf, 0755);
  srand(time(0) ^ getpid() ^ (getpid() << 10));
  srandom(time(0) ^ getpid() ^ (getpid() << 10));
}


add_distinct(char* fname, char* line)
{
   FILE *fp;
   int n = 0;

   if (fp = fopen(fname, "a+")) {
      char buffer[80];
      char tmpname[100];
      FILE *fptmp;

      strcpy(tmpname, fname);
      strcat(tmpname, "_tmp");
      if (!(fptmp = fopen(tmpname, "w"))) {
         fclose(fp);
         return 0;
      }

      rewind(fp);
      while (fgets(buffer, 80, fp)) {
         char* p = buffer + strlen(buffer) - 1;

         if (p[-1] == '\n' || p[-1] == '\r')
            p[-1] = 0;
         if (!strcmp(buffer, line))
            break;
         sscanf(buffer + strlen(buffer) + 2, "%d", &n);
         fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
      }

      if (feof(fp))
         fprintf(fptmp, "%s%c#1\n", line, 0);
      else {
         sscanf(buffer + strlen(buffer) + 2, "%d", &n);
         fprintf(fptmp, "%s%c#%d\n", buffer, 0, n + 1);
         while (fgets(buffer, 80, fp)) {
            sscanf(buffer + strlen(buffer) + 2, "%d", &n);
            fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
         }
      }
      fclose(fp);
      fclose(fptmp);
      unlink(fname);
      rename(tmpname, fname);
   }
}


del_distinct(char* fname, char* line)
{
   FILE *fp;
   int n = 0;

   if (fp = fopen(fname, "r")) {
      char buffer[80];
      char tmpname[100];
      FILE *fptmp;

      strcpy(tmpname, fname);
      strcat(tmpname, "_tmp");
      if (!(fptmp = fopen(tmpname, "w"))) {
         fclose(fp);
         return 0;
      }

      rewind(fp);
      while (fgets(buffer, 80, fp)) {
         char* p = buffer + strlen(buffer) - 1;

         if (p[-1] == '\n' || p[-1] == '\r')
            p[-1] = 0;
         if (!strcmp(buffer, line))
            break;
         sscanf(buffer + strlen(buffer) + 2, "%d", &n);
         fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
      }

      if (!feof(fp))
         while (fgets(buffer, 80, fp)) {
            sscanf(buffer + strlen(buffer) + 2, "%d", &n);
            fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
         }
      fclose(fp);
      fclose(fptmp);
      unlink(fname);
      rename(tmpname, fname);
   }
}

#ifdef WHERE
/* Jaky and Ptt*/
int where (char *from)
{
   extern struct FROMCACHE *fcache;
   register int i,count,j;

   resolve_fcache();
   for (j=0;j<fcache->top;j++)
   {
      char *token=strtok(fcache->domain[j],"&");
      i=0;count=0;
      while(token)
      {
         if (strstr(from,token)) count++;
         token=strtok(NULL, "&");
         i++;
      }
      if (i==count) break;
   }
   if (i!=count) return 0;
   return j;
}
#endif

void
check_BM()      /* Ptt ¦Û°Ê¨ú¤UÂ÷Â¾ªO¥DÅv¤O */
{
  int i;
  boardheader *bhdr;
  extern boardheader *bcache;
  extern int numboards;

  resolve_boards();
  cuser.userlevel &= ~PERM_BM;
  for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
      is_BM(bhdr->BM);
}

void
setup_utmp(mode)
  int mode;
{
  user_info uinfo;
  char buf[80];

  memset(&uinfo, 0, sizeof(uinfo));
  uinfo.pid = currpid = getpid();
  uinfo.uid = usernum;
  uinfo.mode = currstat = mode;
  uinfo.msgcount = 0;
  if(cuser.userlevel & PERM_BM) check_BM(); /* Ptt ¦Û°Ê¨ú¤UÂ÷Â¾ªO¥DÅv¤O */
  uinfo.userlevel = cuser.userlevel;

  strcpy(uinfo.userid, cuser.userid);
  strcpy(uinfo.realname, cuser.realname);
  strcpy(uinfo.username, cuser.username);
  strcpy(uinfo.feeling, cuser.feeling);
  strncpy(uinfo.from, fromhost, 23);
  uinfo.invisible = cuser.invisible;
  uinfo.pager     = cuser.pager & 7;

/* Ptt WHERE*/

#ifdef WHERE
  uinfo.from_alias = where(fromhost);
#else
  uinfo.from_alias = 0;
#endif
  setuserfile(buf, "remoteuser");
  add_distinct(buf, getenv("RFC931"));

#ifdef SHOW_IDLE_TIME
  if (use_shell_login_mode)
    strcpy(uinfo.tty, tty_name);
#endif

  if (enter_uflag & CLOAK_FLAG)
      uinfo.invisible = YEA;
  
  getnewutmpent(&uinfo);
  friend_load();
}

static void
user_login()
{
  char genbuf[256];
  struct tm *ptime,*tmp;
  time_t now = time(0);
  int a;

  extern struct FROMCACHE *fcache;
  extern struct UTMPFILE *utmpshm;
  extern int fcache_semid;
  
  log_usies("ENTER", getenv("RFC931")/* fromhost */);
  //setproctitle("%s: %s", cuser.userid, fromhost);

  /* ------------------------ */
  /* ªì©l¤Æ uinfo¡Bflag¡Bmode */
  /* ------------------------ */

  setup_utmp(LOGIN);
  currmode = MODE_STARTED;
  enter_uflag = cuser.uflag;

/* get local time */
  tmp = localtime(&cuser.lastlogin);

  update_data(); //wildcat: update user data
/*Ptt check ¦P®É¤W½u¤H¼Æ */
  resolve_fcache();
  resolve_utmp();

  if((a=utmpshm->number)>fcache->max_user)
    {
      sem_init(FROMSEM_KEY,&fcache_semid);
      sem_lock(SEM_ENTER,fcache_semid);
      fcache->max_user = a;
      fcache->max_time = now;
      sem_lock(SEM_LEAVE,fcache_semid);
    }

#ifdef  INITIAL_SETUP
  if (!getbnum(DEFAULT_BOARD))
  {
    strcpy(currboard, "©|¥¼¿ï©w");
  }
  else
#endif

  {
    brc_initial(DEFAULT_BOARD);
    set_board();
  }

  /* ------------ */
  /* µe­±³B²z¶}©l */
  /* ------------ */

  if (!(HAS_PERM(PERM_SYSOP) && HAS_PERM(PERM_DENYPOST)))
  {
    char buf[256];
    time_t now;
    
    time(&now);
    sprintf(buf,"<<¤W¯¸³qª¾>> -- §Ú¨ÓÅo¡I - %s",Etime(&now));
    do_aloha(buf);
  }

  time(&now);
  ptime = localtime(&now);

  if((cuser.day == ptime->tm_mday) && (cuser.month == (ptime->tm_mon + 1)))
  {
    currutmp->birth  = 1;
  }
  else
  {
    more("etc/Welcome_login", NA);
    currutmp->birth = 0 ;
  }
/* wildcat : ·h®a³qª¾¥Î
  if(belong(BBSHOME"/etc/oldip",fromhost))
  {
    more(BBSHOME"/etc/removal");
    abort_bbs();
  }    
*/
  if (cuser.userlevel)          /* not guest */
  {
    move(t_lines - 3, 0);
    prints("      Åwªï±z²Ä [1;33m%d[0;37m «×«ô³X¥»¯¸¡A\
¤W¦¸±z¬O±q [1;33m%s[0;37m ³s©¹¥»¯¸¡A\n\
     §Ú°O±o¨º¤Ñ¬O [1;33m%s[0;37m¡C\n",
      ++cuser.numlogins, cuser.lasthost,
      Etime(&cuser.lastlogin));
    pressanykey(NULL);


/* Ptt */
    ptime = localtime(&cuser.lastlogin);
    if(currutmp->birth == 1 
      && (ptime->tm_mday != cuser.day || ptime->tm_mon + 1 != cuser.month))
    {
      more("etc/Welcome_birth",YEA);
    }
    setuserfile(genbuf, str_badlogin);
    if (more(genbuf, NA) != -1)
    {
      if (getans("±z­n§R°£¥H¤W¿ù»~¹Á¸Õªº°O¿ý¶Ü(Y/N)?[Y]") != 'n')
        unlink(genbuf);
    }
    check_register();
    strncpy(cuser.lasthost, fromhost, 24);
    substitute_record(fn_passwd, &cuser, sizeof(cuser), usernum);
    cuser.lasthost[23] = '\0';
    restore_backup();
  }
/* ªO¥D´£°ª«H½c¤W­­ */
  if(HAS_PERM(PERM_BM) && cuser.exmailbox < 100)
    cuser.exmailbox = 100;
  else if (!strcmp(cuser.userid, STR_GUEST))
  {
    char *nick[10] = {"ÄÑ¥]", "¬ü¤k¹Ï", "FreeBSD", "DBT¥Ø¿ý", "mp3",
                      "¤k¤ýÀY", "¯f¬r", "µ£¦~", "¥Û¹³", "386 CPU"
                     };
    char *name[10] = {"¥d±þ¦Ì¨È", "¥Õ³¾¿AÄR¤l", "­·»P¹Ð®J¤p¯¸", "¨gx©Î«Ox±¶", 
                      "Wu Bai & China Blue","¥ì¤O¨F¥Õ", "C-brain", 
                      "®É¶¡", "§Ú", "Intel inside" 
                      };
    char *addr[10] = {"ªkÄõµ·", "www.***.or.jp", "M$", "¦a¤U¥úºÐ¤u¼t", 
                      "¦U¤j°Û¤ù¦æ","³¥¬h", "5.25¦TºÏºÐ¤ù", "seiko¤â¿ö", 
                      "¦a¤U°g®c15¼h", "¸ê·½¦^¦¬±í"};
    int sex[10] = {6, 4, 7, 7, 2, 6, 0, 7, 7, 0};

    int i = rand() % 10;
    sprintf(cuser.username, "³Q­·¤Æªº%s", nick[i]);
    sprintf(currutmp->username, cuser.username);
    sprintf(cuser.realname, name[i]);
    sprintf(currutmp->realname, cuser.realname);
    sprintf(cuser.address, addr[i]);
    cuser.sex = sex[i];
    cuser.habit = HABIT_GUEST;	/* guest's habit */
    currutmp->pager = 2;
    pressanykey(NULL);
  }
  if (bad_user(cuser.userid)) {
     sprintf(currutmp->username, "BAD_USER");
     cuser.userlevel = 0;
     cuser.uflag = COLOR_FLAG | PAGER_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
  }
  if (!PERM_HIDE(currutmp))
     cuser.lastlogin = login_start_time;
  substitute_record(fn_passwd, &cuser, sizeof(cuser), usernum);

   if(cuser.numlogins < 2)
   {
     more("etc/newuser",YEA);
     HELP();
     pressanykey("­ò?!·s¨Óªº­ò?µy«á¥i¥H¨ì Hi_All ªO¦Û§Ú¤¶²Ð¤@¤U­ò!");
  }
  if(HAS_HABIT(HABIT_NOTE))
    more("note.ans",YEA);
  if(HAS_HABIT(HABIT_SEELOG))
    Log();
}


do_aloha(char *hello)
{
   int fd;
   PAL pal;
   char genbuf[256];

   setuserfile(genbuf, FN_ALOHA);
   if ((fd = open(genbuf, O_RDONLY)) > 0)
   {
      sprintf(genbuf + 1, hello);
      *genbuf = 1;
      while (read(fd, &pal, sizeof(pal)) == sizeof(pal)) {
         user_info *uentp;
         extern cmpuids();
         int tuid;

         if ( (tuid = searchuser(pal.userid))  && tuid != usernum &&
             (uentp = (user_info *) search_ulistn(cmpuids, tuid, 1)) &&
             ((uentp->userlevel & PERM_SYSOP) || ((!currutmp->invisible || 
           uentp->userlevel & PERM_SEECLOAK) && !(is_rejected(uentp) & 1))))    
            my_write(uentp->pid, genbuf);
      }
      close(fd);
   }
}


void
check_max_online()
{
  FILE *fp;
  int maxonline=0;
  time_t now = time(NULL);
  struct tm *ptime;

  ptime = localtime(&now);

  if(fp = fopen(".maxonline", "r"))
  {
    fscanf(fp, "%d", &maxonline);
    fclose(fp);
  }

  if ((count_ulist() > maxonline) && (fp = fopen(".maxonline", "w")))
  {
    fprintf(fp, "%d", count_ulist());
    fclose(fp);
  }
  if(fp = fopen(".maxtoday", "r"))
  {
    fscanf(fp, "%d", &maxonline);
    if (count_ulist() > maxonline){
      fclose(fp);
      fp = fopen(".maxtoday", "w");
      fprintf(fp, "%d", count_ulist());
    }
    fclose(fp);
  }
}


void
start_client()
{
  char cmd[80] = "?@";
  extern struct commands cmdlist[];
  extern char currmaildir[32];

  if (!getenv("RFC931"))
    setenv("RFC931", strcat(cmd, fromhost), 1);

  /* ----------- */
  /* system init */
  /* ----------- */
  currmode = 0;
  update_time = login_start_time;

  signal(SIGHUP, abort_bbs);
  signal(SIGBUS, abort_bbs);
  signal(SIGSEGV, abort_bbs);
#ifdef SIGSYS
  signal(SIGSYS, abort_bbs);
#endif
  signal(SIGTERM, SIG_IGN);

  signal(SIGUSR1, talk_request);
  signal(SIGUSR2, write_request);

  if (setjmp(byebye))
    abort_bbs();

  dup2(0, 1);
  
  term_init("vt100");
  initscr();
  login_query();
  user_login();
  check_max_online();
/* wildcat : ­×´_¤¤
  if(!HAVE_PERM(PERM_SYSOP))
  {
    pressanykey("¾ã­×¤¤ , ¸T¤î¯¸ªø¥H¥~ user µn¤J");
    abort_bbs();
  }
*/
  sethomedir(currmaildir, cuser.userid);
#ifdef DOTIMEOUT
  init_alarm();
#else
  signal(SIGALRM, SIG_IGN);
#endif
  if (HAVE_PERM(PERM_SYSOP | PERM_BM))
  {
    DL_func("SO/vote.so:b_closepolls");
  }
  if (!HAVE_HABIT(HABIT_COLOR))
    showansi = 0;
  while (chkmailbox())
     m_read();

#ifdef POSTNOTIFY
  m_postnotify(); 
#endif

  force_board("Announce");
  {
    char buf[80];
    setbfile(buf, "VIP", FN_LIST);
    if(belong_list(buf, cuser.userid) > 0)
      force_board("VIP");
    setbfile(buf, "WD_plan", FN_LIST);
    if(belong_list(buf,cuser.userid) > 0)
      force_board("WD_plan");
  }
  if(HAS_PERM(PERM_SYSOP))
    force_board("WIND");
  if(HAS_PERM(PERM_BM))
    force_board("BM");
//  force_board("Boards");

  if(HAS_HABIT(HABIT_FROM) && HAS_PERM(PERM_FROM))
  {
    char fbuf[50];
    sprintf(fbuf, "¬G¶m [%s]¡G", currutmp->from);
    if(getdata(b_lines, 0, fbuf, currutmp->from, 17, DOECHO,0))
      currutmp->from_alias=0;
  }
  
  if(HAS_HABIT(HABIT_FEELING))
  {
    getdata(b_lines ,0,"¤µ¤Ñªº¤ß±¡¦p¦ó©O¡H", cuser.feeling, 5 ,DOECHO,cuser.feeling);
    cuser.feeling[4] = '\0';
    strcpy(currutmp->feeling, cuser.feeling);
    substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  }

  if (HAS_PERM(PERM_ACCOUNTS) && dashf(fn_register))
    m_register();             
  domenu(MMENU, "¥D¥\\¯àªí", (chkmail(0) ? 'M' : 'B'), cmdlist);
}

/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol       */
/* ----------------------------------------------------- */


static void
telnet_init()
{
  static char svr[] = {
    IAC, DO, TELOPT_TTYPE,
    IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
    IAC, WILL, TELOPT_ECHO,
    IAC, WILL, TELOPT_SGA
  };

  register int n, len;
  register char *cmd, *data;
  int rset, oset;
  struct timeval to;
  char buf[256];

  data = buf;
  to.tv_sec = 1;
  rset = to.tv_usec = 0;
  FD_SET(0, (fd_set *) & rset);
  oset = rset;

  for (n = 0, cmd = svr; n < 4; n++)
  {
    len = (n == 1 ? 6 : 3);
    write(0, cmd, len);
    cmd += len;

    if (select(1, (fd_set *) & rset, NULL, NULL, &to) >0)
      read(0, data, sizeof(buf));
    rset = oset;
  }
}



/* ----------------------------------------------- */
/* ¨ú±o remote user name ¥H§P©w¨­¥÷                */
/* ----------------------------------------------- */


/*
 * rfc931() speaks a common subset of the RFC 931, AUTH, TAP, IDENT and RFC
 * 1413 protocols. It queries an RFC 931 etc. compatible daemon on a remote
 * host to look up the owner of a connection. The information should not be
 * used for authentication purposes. This routine intercepts alarm signals.
 *
 * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
 */

#include <setjmp.h>

#define STRN_CPY(d,s,l) { strncpy((d),(s),(l)); (d)[(l)-1] = 0; }
#define RFC931_TIMEOUT   10
#define RFC931_PORT     113     /* Semi-well-known port */
#define ANY_PORT        0       /* Any old port will do */


/* ------------------------- */
/* timeout - handle timeouts */
/* ------------------------- */


static void
timeout(sig)
  int sig;
{
  (void) longjmp(byebye, sig);
}


static void
getremotename(from, rhost, rname)
  struct sockaddr_in *from;
  char *rhost;
  char *rname;
{
  struct sockaddr_in our_sin;
  struct sockaddr_in rmt_sin;
  unsigned rmt_port, rmt_pt;
  unsigned our_port, our_pt;
  FILE *fp;
  char buffer[512], user[80], *cp;
  int s;
  struct hostent *hp;

  /* get remote host name */

  hp = NULL;
  if (setjmp(byebye) == 0)
  {
    signal(SIGALRM, timeout);
    alarm(3);
    hp = gethostbyaddr((char *) &from->sin_addr, sizeof(struct in_addr),
      from->sin_family);
    alarm(0);
  }
  strcpy(rhost, hp ? hp->h_name : (char *) inet_ntoa(from->sin_addr));

  /*
   * Use one unbuffered stdio stream for writing to and for reading from the
   * RFC931 etc. server. This is done because of a bug in the SunOS 4.1.x
   * stdio library. The bug may live in other stdio implementations, too.
   * When we use a single, buffered, bidirectional stdio stream ("r+" or "w+"
   * mode) we read our own output. Such behaviour would make sense with
   * resources that support random-access operations, but not with sockets.
   */

  s = sizeof our_sin;
  if (getsockname(0, (struct sockaddr*)&our_sin, &s) < 0)
    return;

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("bbsd: socket in rfc931");
    return;
  }

  if (!(fp = fdopen(s, "r+")))
  {
    (void) close(s);
    perror("bbsd:fdopen");
    return;
  }

  /*
   * Set up a timer so we won't get stuck while waiting for the server.
   */

  if (setjmp(byebye) == 0)
  {
    signal(SIGALRM, timeout);
    alarm(RFC931_TIMEOUT);

    /*
     * Bind the local and remote ends of the query socket to the same IP
     * addresses as the connection under investigation. We go through all
     * this trouble because the local or remote system might have more than
     * one network address. The RFC931 etc. client sends only port numbers;
     * the server takes the IP addresses from the query socket.
     */

    our_pt = ntohs(our_sin.sin_port);
    our_sin.sin_port = htons(ANY_PORT);

    rmt_sin = *from;
    rmt_pt = ntohs(rmt_sin.sin_port);
    rmt_sin.sin_port = htons(RFC931_PORT);

    setbuf(fp, (char *) 0);
    s = fileno(fp);

    if (bind(s, (struct sockaddr *) & our_sin, sizeof(our_sin)) >= 0 &&
      connect(s, (struct sockaddr *) & rmt_sin, sizeof(rmt_sin)) >= 0)
    {
      /*
       * Send query to server. Neglect the risk that a 13-byte write would
       * have to be fragmented by the local system and cause trouble with
       * buggy System V stdio libraries.
       */

      fprintf(fp, "%u,%u\r\n", rmt_pt, our_pt);
      fflush(fp);

      /*
       * Read response from server. Use fgets()/sscanf() so we can work
       * around System V stdio libraries that incorrectly assume EOF when a
       * read from a socket returns less than requested.
       */

      if (fgets(buffer, sizeof(buffer), fp) && !ferror(fp) && !feof(fp)
        && sscanf(buffer, "%u , %u : USERID :%*[^:]:%79s",
          &rmt_port, &our_port, user) == 3
        && rmt_pt == rmt_port && our_pt == our_port)
      {

        /*
         * Strip trailing carriage return. It is part of the protocol, not
         * part of the data.
         */

        if (cp = (char *) strchr(user, '\r'))
          *cp = 0;
        strcpy(rname, user);
      }
    }
    alarm(0);
  }
  fclose(fp);
}


/* ----------------------------------------------------- */
/* stand-alone daemon                                    */
/* ----------------------------------------------------- */


static int mainset;             /* read file descriptor set */
static struct sockaddr_in xsin;


static void
reapchild()
{
  int state, pid;

  while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0);
}


static void
start_daemon()
{
  int n;
//  char buf[80];

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

//  time_t dummy = time(NULL);
//  struct tm *dummy_time = localtime(&dummy);

//  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", dummy_time);

  if (n = fork())
  {
    fprintf(stdout, "MBBS Daemon Stard in PID[%d]\n",n);
    exit(0);
  }

  n = getdtablesize();

}


static void
close_daemon()
{
  exit(0);
}


static int
bind_port(port)
  int port;
{
  int sock, on;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  on = 1;
  (void) setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
  (void) setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof(on));

  on = 0;
  (void) setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &on, sizeof(on));

  xsin.sin_port = htons(port);
  if (bind(sock, (struct sockaddr *)&xsin, sizeof xsin) < 0) {
    syslog(LOG_INFO, "bbsd bind_port can't bind to %d",port);
    exit(1);
  }

  if (listen(sock, SOCKET_QLEN) < 0) {
    syslog(LOG_INFO, "bbsd bind_port can't listen to %d",port);
    exit(1);
  }

  /*(void)*/ FD_SET(sock, (fd_set *) & mainset);
  return sock;
}

int
bad_host(char* name)
{
   FILE* list;
   char buf[40];

  if (list = fopen(BBSHOME"/etc/bad_host", "r")) {
     while (fgets(buf, 40, list)) {
        buf[strlen(buf) - 1] = '\0';
        if (!strcmp(buf, name))
           return 1;
        if (buf[strlen(buf) - 1] == '.' && !strncmp(buf, name, strlen(buf)))
           return 1;
        if (*buf == '.' && strlen(buf) < strlen(name) &&
            !strcmp(buf, name + strlen(name) - strlen(buf)))
           return 1;
     }
     fclose(list);
  }
  return 0;
}


int
main(argc, argv,envp)
  int argc;
  char *argv[];
  char *envp[];
{
  extern int errno;
  register int msock, csock;    /* socket for Master and Child */
  register int nfds;            /* number of sockets */
  register pid_t pid;
  int readset;
  int value;
  struct timeval tv;

  /* avoid SIGPIPE */
  signal(SIGPIPE,SIG_IGN);
    
  /* avoid erroneous signal from other mbbsd */
  signal(SIGUSR1,SIG_IGN);
  signal(SIGUSR2,SIG_IGN);
  
  /* check if invoked as "bbs" */
  if(argc > 2 && !strcmp(argv[0], "mbbsd"))
  {
    /* Give up root privileges: no way back from here */
    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    use_shell_login_mode = 1;

    /* copy from the original "bbs" */
    if(argc > 1) {
      strcpy(fromhost, argv[1]);
#ifdef SHOW_IDLE_TIME
      if(argc > 2)
        strcpy(tty_name, argv[2]);
#endif
      if(argc > 3)
        strcpy(remoteusername, argv[3]);
    }
	
    close(2);
    init_tty();

    nice(3);      /*  Ptt lower priority */
    login_start_time = time(0);
    start_client();
  }
  else
  {
    time_t now;
    /* --------------------------------------------------- */
    /* setup standalone                                    */
    /* --------------------------------------------------- */
    start_daemon();

    signal(SIGCHLD, reapchild);
    signal(SIGTERM, close_daemon);

    /* --------------------------------------------------- */
    /* port binding                                        */
    /* --------------------------------------------------- */

    xsin.sin_family = AF_INET;
  
    time(&now);
    sprintf(genbuf, "%d\t%s", getpid(), Ctime(&now));

    if (argc > 1)
    {
      msock = -1;
      for (nfds = 1; nfds < argc; nfds++)
      {
        csock = atoi(argv[nfds]);
        if (csock > 0)
        {
           msock = bind_port(csock);
           sprintf(genbuf + 512, " %d", csock);
           strcat(genbuf, genbuf + 512);
           strcat(margs, genbuf + 512);
	}
        else
          break;
      }
      if (msock < 0)
        exit(1);
    }
    else
    {
      static int ports[] = {5757};

      for (nfds = 0; nfds < sizeof(ports) / sizeof(int); nfds++)
      {
        csock = ports[nfds];
        msock = bind_port(csock);

        sprintf(genbuf + 512, " %d", csock);
        strcat(genbuf, genbuf + 512);
        strcat(margs, genbuf + 512);
      }
    }
    nfds = msock + 1;

    /* --------------------------------------------------- */
    /* Give up root privileges: no way back from here      */
    /* --------------------------------------------------- */

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    f_cat(PID_FILE, genbuf); 

    //setproctitle("listening%s", margs);

    /* --------------------------------------------------- */
    /* main loop                                           */
    /* --------------------------------------------------- */



    for (;;)
    {
again:

      tv.tv_sec = 60 * 30;
      tv.tv_usec = 0;

      readset = mainset;
      msock = select(nfds, (fd_set *) & readset, NULL, NULL, &tv);

      if (msock < 0)
      {
        goto again;
      }
      else if (msock == 0)        /* No network traffic */
        continue;
 
      msock = 0;
      csock = 1;
      for (;;)
      {
        if (csock & readset)
          break;
        if (++msock >= nfds)
        {
          goto again;
        }
        csock <<= 1;
      }

      value = sizeof xsin;
      do
      {
        csock = accept(msock, (struct sockaddr *)&xsin, &value);
      } while (csock < 0 && errno == EINTR);

      if (csock < 0)
      {
        goto again;
      }


      pid = fork();

      if (!pid)
      {
        //setproctitle("...login...");
        nice(2);      /*  Ptt lower priority */
        write(csock, BANNER , strlen(BANNER));
        while (--nfds >= 0)
          close(nfds);
        dup2(csock, 0);
        close(csock);
        login_start_time = time(0);

        getremotename(&xsin, fromhost, remoteusername);   /* FC931 */

        /* ban ±¼ bad host / bad user  */
        if (bad_host(fromhost))
          exit(1);
          
        setenv("REMOTEHOST", fromhost, 1);
        setenv("REMOTEUSERNAME", remoteusername, 1);

        telnet_init();
        start_client();
      }
      close(csock);
    }
  }
}
