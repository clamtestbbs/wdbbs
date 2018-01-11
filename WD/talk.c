/*-------------------------------------------------------*/
/* talk.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : talk/quety/friend routines                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#define _MODES_C_

#include "bbs.h"
#include "type.h"

#ifdef lint
#include <sys/uio.h>
#endif

#define IRH 1
#define HRM 2

struct talk_win
{
  int curcol, curln;
  int sline, eline;
};


typedef struct
{
  user_info *ui;
  time_t idle;
  usint friend;
}      pickup;


extern int bind();
extern char* getuserid();
extern struct UTMPFILE *utmpshm;
extern char watermode, no_oldmsg, oldmsg_count;
extern msgque oldmsg[MAX_REVIEW];
extern int cmpuname();
/* extern char currdirect[]; */

/* -------------------------- */
/* °O¿ý friend ªº user number */
/* -------------------------- */

#define PICKUP_WAYS     6
int pickup_way = 0;
int friendcount;
int friends_number;
int override_number;
int rejected_number;
int bfriends_number;
char *fcolor[11] = {"","[36m","[32m","[1;32m",
                   "[33m","[1;33m","[1;37m" ,"[1;37m",
                   "[1;31m", "[1;35m", "[1;36m"};
char *talk_uent_buf;
char save_page_requestor[40];
char page_requestor[40];
static FILE* flog;

void friend_load();

int is_rejected(user_info *ui);

char *
modestring(uentp, simple)
  user_info *uentp;
  int simple;
{
  static char modestr[40];
  static char *notonline="¤£¦b¯¸¤W";
  register int mode = uentp->mode;
  register char *word;

  word = ModeTypeTable[mode];

  if (!(HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_SEECLOAK)) &&
      (uentp->invisible || (is_rejected(uentp) & HRM)))
    return (notonline);
/*
woju
*/
  else if (mode == EDITING) {
     sprintf(modestr, "E:%s",
         ModeTypeTable[uentp->destuid < EDITING ? uentp->destuid : EDITING]);
     word = modestr;
  }
  else if (!mode && *uentp->chatid == 1)
  {
     if (!simple)
        sprintf(modestr, "¦^À³ %s", getuserid(uentp->destuid));
     else
        sprintf(modestr, "¦^À³©I¥s");
  }
  else if (!mode && *uentp->chatid == 3)
     sprintf(modestr, "¤ô²y·Ç³Æ¤¤");
  else if (!mode)
    return (uentp->destuid == 6) ? uentp->chatid :
      IdleTypeTable[(0 <= uentp->destuid & uentp->destuid < 6) ?
                    uentp->destuid: 0];
  else if (simple)
    return (word);

  else if (uentp->in_chat & mode == CHATING)
    sprintf(modestr, "%s (%s)", word, uentp->chatid);
  else if (mode == TALK)
   {
    if (is_hidden(getuserid(uentp->destuid)))    /* Leeym ¹ï¤è(µµ¦â)Áô§Î */
      sprintf(modestr, "%s", "¦Û¨¥¦Û»y¤¤"); /* Leeym ¤j®a¦Û¤vµo´§§a¡I */
    else
      sprintf(modestr, "%s %s", word, getuserid(uentp->destuid));
   }
  else if (mode != PAGE && mode != QUERY)
    return (word);
  else
    sprintf(modestr, "%s %s", word, getuserid(uentp->destuid));

  return (modestr);
}


int
cmpuids(uid, urec)
  int uid;
  user_info *urec;
{
  return (uid == urec->uid);
}


int     /* Leeym ±q FireBird ²¾´Ó§ï¼g¹L¨Óªº */
is_hidden(user)
char *user;
{
    int tuid;
    user_info *uentp;

  if ((!(tuid = getuser(user)))
  || (!(uentp = (user_info *) search_ulist(cmpuids, tuid)))
  || ((!uentp->invisible|| HAS_PERM(PERM_SYSOP)||HAS_PERM(PERM_SEECLOAK))
        && (((!PERM_HIDE(uentp) && !PERM_HIDE(currutmp)) ||
        PERM_HIDE(currutmp))
        && !(is_rejected(uentp) & HRM && !(is_friend(uentp) & 5)))))
        return 0;       /* ¥æ½Í xxx */
  else
        return 1;       /* ¦Û¨¥¦Û»y */
}

/*
woju
*/
int
cmppids(pid, urec)
  pid_t pid;
  user_info *urec;
{
  return (pid == urec->pid);
}


/* ------------------------------------- */
/* routines for Talk->Friend             */
/* ------------------------------------- */


static int
can_override(userid, whoasks)
  char *userid;
  char *whoasks;
{
  char buf[STRLEN];

  sethomefile(buf, userid, fn_overrides);
  return belong(buf, whoasks);
}


int
is_friend(ui)
  user_info *ui;
{
  register ushort unum, hit, *myfriends;

  /* §PÂ_¹ï¤è¬O§_¬°§ÚªºªB¤Í ? */

  unum = ui->uid;
  myfriends = currutmp->friend;
  while (hit = *myfriends++)
  {
    if (unum == hit)
    {
      hit = 3;
      friends_number++;
      break;
    }
  }

  /* ¬ÝªO¦n¤Í */

  if(currutmp->brc_id && ui->brc_id == currutmp->brc_id)
    {
      hit |= 1;
      bfriends_number++;
    }

  /* §PÂ_§Ú¬O§_¬°¹ï¤èªºªB¤Í ? */

  myfriends = ui->friend;
  while (unum = *myfriends++)
  {
    if (unum == usernum)
    {
      override_number++;
      hit |= 5;
      break;
    }
  }
  return hit;
}



static int
be_rejected(userid)
  char *userid;
{
  char buf[STRLEN];

  sethomefile(buf, userid, fn_reject);
  return belong(buf, cuser.userid);
}

  /* ³Q©Úµ´ */

int
is_rejected(ui)
  user_info *ui;
{
  register ushort unum, hit, *myrejects;

  if (PERM_HIDE(ui))
     return 0;
  /* §PÂ_¹ï¤è¬O§_¬°§Úªº¤³¤H ? */

  unum = ui->uid;
  myrejects = currutmp->reject;
  while (hit = *myrejects++)
  {
    if (unum == hit)
    {
      hit = 1;
      rejected_number++;
      break;
    }
  }

  /* §PÂ_§Ú¬O§_¬°¹ï¤èªº¤³¤H ? */

  myrejects = ui->reject;
  while (unum = *myrejects++)
  {
    if (unum == usernum)
    {
      if (hit & IRH)
         --rejected_number;
      hit |= 2;
      break;
    }
  }
  return hit;
}


/* ------------------------------------- */
/* ¯u¹ê°Ê§@                              */
/* ------------------------------------- */

static void
my_kick(uentp)
  user_info *uentp;
{
  char genbuf[200];

  getdata(1, 0, msg_sure_ny, genbuf, 4, LCECHO,0);
  clrtoeol();
  if (genbuf[0] == 'y')
  {
    sprintf(genbuf, "%s (%s)", uentp->userid, uentp->username);
    log_usies("KICK ", genbuf);
    if ((kill(uentp->pid, SIGHUP) == -1) && (errno == ESRCH))
      memset(uentp, 0, sizeof(user_info));
    /* purge_utmp(uentp); */
    outz("½ð¥X¥hÅo");
  }
  else
    outz(msg_cancel);
}


my_query(uident)
  char *uident;
{
  extern char currmaildir[];
  int tuid,usize;
  user_info *uentp;
  userec muser;

  if (tuid = getuser(uident))
  {
    memcpy(&muser, &xuser, sizeof(userec));
    move(0, 0);
    clrtobot();
    move(1, 0);
    setutmpmode(QUERY);
    currutmp->destuid = tuid;

    prints("[ ±b  ¸¹ ]%-30.30s[ ¼Ê  ºÙ ]%s\n",muser.userid,muser.username);
    if (can_override(muser.userid, cuser.userid) || HAS_PERM(PERM_SYSOP)
        || !strcmp(muser.userid, cuser.userid) )
    {
      char *sex[8] = { MSG_BIG_BOY, MSG_BIG_GIRL,
                       MSG_LITTLE_BOY, MSG_LITTLE_GIRL,
                       MSG_MAN, MSG_WOMAN, MSG_PLANT, MSG_MIME };
      prints("[ ©Ê  §O ]%-30.30s",sex[muser.sex%8]);
    }

    prints("[ ¤ß  ±¡ ][1;33m%s[m\n",muser.feeling);
    uentp = (user_info *) search_ulist(cmpuids, tuid);
    if (uentp && !(PERM_HIDE(currutmp) ||
      is_rejected(uentp) & HRM && is_friend(uentp) & 5) && PERM_HIDE(uentp))
      prints("[¥Ø«e°ÊºA][1;30m¤£¦b¯¸¤W                      [m\n");
    else
      prints("[¥Ø«e°ÊºA][1;36m%-30.30s[m",
         uentp ? modestring(uentp, 0) : "[1;30m¤£¦b¯¸¤W");
    prints("[ «Ê  ¸¹ ][1;32m%s[m\n",muser.title);

    prints("[·s«H¥¼Åª]");
    sethomedir(currmaildir, muser.userid);
    usize = dashs(currmaildir);
    outs(chkmail(1) || muser.userlevel & PERM_SYSOP ? "[1;5;33m¦³" : "[1;30mµL");
    sethomedir(currmaildir, cuser.userid);
    chkmail(1);
// wildcat:realnameÀ³¸Ó¤£»Ý­n¤F(user¦Ñ¬O¥H¬°¯u¹ê©m¦W·|³Q§O¤H¬Ý¨ì)
//    if (HAS_PERM(PERM_SYSOP) || !strcmp(muser.userid, cuser.userid))
//      prints("[©m¦W]%s", muser.realname);
    prints("[1;36m%s[m\n", msg_seperator);
    prints("[¤W¯¸¦aÂI]%-30.30s",muser.lasthost[0] ? muser.lasthost : "(¤£¸Ô)");
    prints("[¤W¯¸®É¶¡]%s\n",Etime(&muser.lastlogin));
    prints("[¤W¯¸¦¸¼Æ]%-30d",muser.numlogins);
    prints("[µoªí¤å³¹]%d ½g\n",muser.numposts);
    prints("[¤H®ð«ü¼Æ]%-30d[¦n©_«ü¼Æ]%d\n",muser.bequery,muser.toquery);
    prints("[¤ô²y¶Ç±¡]¦¬ %d / µo %d \n",muser.receivemsg,muser.sendmsg);
    if(HAS_PERM(PERM_SYSOP))
    {
      prints("[¥Ø¿ý¤j¤p]%d\n",usize);
      prints("[«e¦¸¬d¸ß]%-30.30s[³Q¬d¸ß]%s\n",muser.toqid,muser.beqid);
    }
    prints("[1;36m%s[m\n", msg_seperator);

    if(strcmp(muser.beqid,cuser.userid))
    {
      strcpy(muser.beqid,cuser.userid);
      ++muser.bequery;
      substitute_record(fn_passwd, &muser,sizeof(userec), tuid);
    }
    if(strcmp(muser.userid,cuser.toqid))
    {
      update_data();
      strcpy(cuser.toqid,muser.userid);
      ++cuser.toquery;
      substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
    }
    pressanykey(NULL);
//    showplans(uident);
    return RC_FULL;
  }
  update_data();
  return RC_NONE;
  /* currutmp->destuid = 0; */
}

int
my_write(pid, hint)
  pid_t pid;
  char *hint;
{
  int len;
  char msg[80];
  FILE *fp;
  struct tm *ptime;
  time_t now;
  char genbuf[200];
  user_info *uin ;
  extern msgque oldmsg[MAX_REVIEW];
  int a;
/*
woju
*/
  uschar mode0 = currutmp->mode;
  char c0 = currutmp->chatid[0];
  int currstat0 = currstat;

  if(watermode > 0)
     {
       a = (no_oldmsg - watermode + MAX_REVIEW )%MAX_REVIEW;
       uin = (user_info*)search_ulist(cmppids, oldmsg[a].last_pid);
     }
  else
       uin = (user_info*)search_ulist(cmppids, pid);

  if (( !oldmsg_count || !isprint2(*hint)) && !uin ) {
     pressanykey("ÁV¿|! ¹ï¤è¤w¸¨¶]¤F(¤£¦b¯¸¤W)! ~>_<~");
     watermode = -1;
     return 0;
  }

  currutmp->mode = 0;
  currutmp->chatid[0] = 3;
  currstat = XMODE;


  time(&now);
  ptime = localtime(&now);

  if (isprint2(*hint))
  {
    char inputbuf[4];
    if (!(len = getdata(0, 0, hint, msg, 65, DOECHO,0))) {
      pressanykey("ºâ¤F! ©ñ§A¤@°¨...");
      currutmp->chatid[0] = c0;
      currutmp->mode = mode0;
      currstat = currstat0;
      watermode = -1;
      return 0;
  }
/* Ptt */
    if(watermode > 0)
      {
       a = (no_oldmsg - watermode + MAX_REVIEW )%MAX_REVIEW;
       uin = (user_info*)search_ulist(cmppids, oldmsg[a].last_pid);
      }

    strip_ansi(msg,msg,0);
    if (!uin  || !*uin->userid) {
       pressanykey("ÁV¿|! ¹ï¤è¤w¸¨¶]¤F(¤£¦b¯¸¤W)! ~>_<~");
       currutmp->chatid[0] = c0;
       currutmp->mode = mode0;
       currstat = currstat0;
       watermode = -1;
       return 0;
    }

    sprintf(genbuf, "¥á%s¤ô²y:%.40s....?[Y] ", uin->userid, msg);

    getdata(0, 0, genbuf, inputbuf, 3, LCECHO,0);
    genbuf[0] = '\0';
    watermode = -1;
    if (inputbuf[0] == 'n') {
      currutmp->chatid[0] = c0;
      currutmp->mode = mode0;
      currstat = currstat0;
      return 0;
    }
    if (!uin || !*uin->userid) {
       pressanykey("ÁV¿|! ¹ï¤è¤w¸¨¶]¤F(¤£¦b¯¸¤W)! ~>_<~");
       currutmp->chatid[0] = c0;
       currutmp->mode = mode0;
       currstat = currstat0;
       return 0;
    }
  }
  else {
     strcpy(msg, hint + 1);
     strip_ansi(msg,msg,0);
     len = strlen(msg);
     watermode = -1;
  }
   now = time(0);
   if (*hint != 1) {
      sethomefile(genbuf, uin->userid, fn_writelog);
      if (fp = fopen(genbuf, "a")) {
        fprintf(fp, COLOR2"[1m¡¹ [37m%s %s %s [0m[%s]\n",
          cuser.userid, (*hint == 2) ? "[33;41m¼s¼½" : "", msg, Cdatelite(&now));
        fclose(fp);
      }
      sethomefile(genbuf, cuser.userid, fn_writelog);
      if (fp = fopen(genbuf, "a")) 
      {
        fprintf(fp, "To %s: %s [%s]\n", uin->userid, msg, Cdatelite(&now));
        fclose(fp);
        update_data();
        ++cuser.sendmsg;
        substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
      }
   }
   if (*hint == 2 && uin->msgcount) {
      uin->destuip = currutmp;
      uin->sig = 2;
      kill(uin->pid, SIGUSR1);
   }
   else if (*hint != 1 && !HAS_PERM(PERM_SYSOP) && ( uin->pager == 3
       || uin->pager == 2 || (uin->pager == 4 && !(is_friend(uin) & 5)) ))
//       || uin->pager == 2 || (uin->pager == 4 && !(is_friend(uin) & 2)) ))
      pressanykey("ÁV¿|! ¹ï¤è¨¾¤ô¤F!");
   else {
//      if (uin->msgcount < MAXMSGS) {
         uschar pager0 = uin->pager;
         uin->msgcount=0;
         uin->pager = 2;
         uin->msgs[uin->msgcount].last_pid = currpid;
         strcpy(uin->msgs[uin->msgcount].last_userid, currutmp->userid);
         strcpy(uin->msgs[uin->msgcount++].last_call_in, msg);
         uin->pager = pager0;
//      }
      if (uin->msgcount  == 1 && kill(uin->pid, SIGUSR2) == -1 && *hint != 1)
         pressanykey("ÁV¿|! ¨S¥´¤¤! ~>_<~");
      else if (uin->msgcount == 1 && *hint != 1)
      {
         outz("[1m[44m¤ô²y¯{¹L¥h¤F! *^o^Y[m");
      }
   }
//   clrtoeol();
//   refresh();

   currutmp->chatid[0] = c0;
   currutmp->mode = mode0;
   currstat = currstat0;
   return 1;
}

static char t_display_new_flag =0;

void
t_display_new()
{
   int i;
   char buf[256];

   if(t_display_new_flag) return;

   else t_display_new_flag = 1;

   if(oldmsg_count && watermode > 0)
     {
         move(1,0);
         clrtoeol();
         outs(
" [1;34m¢w¢w¢w¢w¢w¢w¢w[37m¤ô[34m¢w[37m²y[34m¢w[37m¦^[34m¢w[37mÅU[34m¢w¢w¢w¢w¢w¢w¢w¢w¢w"COLOR1" [Ctrl-R]©¹¤U¤Á´« [34;40m¢w¢w¢w¢w¢w¢w [m");
         for(i=0 ; i < oldmsg_count ;i++)
                {
                 int a = (no_oldmsg - i - 1 + MAX_REVIEW )%MAX_REVIEW;
                 move(i+2,0);
                 clrtoeol();
                 if(watermode-1 != i)
                    sprintf(buf,"[1;37m %s %s[m",
                         oldmsg[a].last_userid,oldmsg[a].last_call_in);
                 else
                    sprintf(buf,"[1m[36m>%s %s[m",
                         oldmsg[a].last_userid,oldmsg[a].last_call_in);
                 outs(buf);
                }
          move(i+2,0);
          clrtoeol();
          outs(
" [1;34m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w"COLOR1" [Ctrl-T]©¹¤W¤Á´« [40;34m¢w¢w¢w¢w¢w¢w[m ");
     }
  t_display_new_flag =0;
}

/* Thor: for ask last call-in message */

int
t_display()
{
  char genbuf[64];

  setuserfile(genbuf, fn_writelog);
  if (more(genbuf, YEA) != -1)
  {
    switch (getans("²M°£(C) ²¾¦Ü³Æ§Ñ¿ý(M) «O¯d(R) (C/M/R)?[R]"))
    {
    case 'm':
      mail2user(cuser, "¼ö½u[37;41m°O¿ý[m", genbuf);
      /* shakalaca.000814: ¤£¥Î break ¬O¦]¬° mail2user() ¥Î¤F f_cp,
      	©Ò¥H±µµÛ 'c' ±N­ì©lÀÉ®× unlink */

    case 'c':
      unlink(genbuf);

    default:
      break;
    }
    return RC_FULL;
  }
  return RC_NONE;
}


/* ----------------------------------------------------- */

static void
do_talk_nextline(twin)
  struct talk_win *twin;
{
   twin->curcol = 0;
   if (twin->curln < twin->eline)
      ++(twin->curln);
   else
      region_scroll_up(twin->sline, twin->eline);
   move(twin->curln, twin->curcol);
}


static void
do_talk_char(twin, ch)
  struct talk_win *twin;
  int ch;
{
  extern int dumb_term;
  extern screenline* big_picture;
  screenline* line;
  int i;
  char ch0, buf[81];

  if (isprint2(ch))
  {
    ch0 = big_picture[twin->curln].data[twin->curcol];
    if (big_picture[twin->curln].len < 79)
       move(twin->curln, twin->curcol);
    else
       do_talk_nextline(twin);
    outc(ch);
    ++(twin->curcol);
    line =  big_picture + twin->curln;
    if (twin->curcol < line->len) {      /* insert */
       ++(line->len);
       memcpy(buf, line->data + twin->curcol, 80);
       save_cursor();
       do_move(twin->curcol, twin->curln);
       ochar(line->data[twin->curcol] = ch0);
       for (i = twin->curcol + 1; i < line->len; i++)
          ochar(line->data[i] = buf[i - twin->curcol - 1]);
       restore_cursor();
    }
    line->data[line->len] = 0;
    return;
  }

  switch (ch)
  {
  case Ctrl('H'):
  case '\177':
    if (twin->curcol == 0)
    {
      return;
    }
    line =  big_picture + twin->curln;
    --(twin->curcol);
    if (twin->curcol < line->len) {
       --(line->len);
       save_cursor();
       do_move(twin->curcol, twin->curln);
       for (i = twin->curcol; i < line->len; i++)
          ochar(line->data[i] = line->data[i + 1]);
       line->data[i] = 0;
       ochar(' ');
       restore_cursor();
    }
    move(twin->curln, twin->curcol);
    return;

  case Ctrl('D'):
     line =  big_picture + twin->curln;
     if (twin->curcol < line->len) {
        --(line->len);
        save_cursor();
        do_move(twin->curcol, twin->curln);
        for (i = twin->curcol; i < line->len; i++)
           ochar(line->data[i] = line->data[i + 1]);
        line->data[i] = 0;
        ochar(' ');
        restore_cursor();
     }
     return;
  case Ctrl('G'):
    bell();
    return;
  case Ctrl('B'):
     if (twin->curcol > 0) {
        --(twin->curcol);
        move(twin->curln, twin->curcol);
     }
     return;
  case Ctrl('F'):
     if (twin->curcol < 79) {
        ++(twin->curcol);
        move(twin->curln, twin->curcol);
     }
     return;
  case Ctrl('A'):
     twin->curcol = 0;
     move(twin->curln, twin->curcol);
     return;
  case Ctrl('K'):
     clrtoeol();
     return;
  case Ctrl('Y'):
     twin->curcol = 0;
     move(twin->curln, twin->curcol);
     clrtoeol();
     return;
  case Ctrl('E'):
     twin->curcol = big_picture[twin->curln].len;
     move(twin->curln, twin->curcol);
     return;
  case Ctrl('M'):
  case Ctrl('J'):
     line =  big_picture + twin->curln;
     strncpy(buf, line->data, line->len);
     buf[line->len] = 0;
     if (dumb_term)
       outc('\n');
     do_talk_nextline(twin);
     break;
  case Ctrl('P'):
     line =  big_picture + twin->curln;
     strncpy(buf, line->data, line->len);
     buf[line->len] = 0;
     if (twin->curln > twin->sline) {
        --(twin->curln);
        move(twin->curln, twin->curcol);
     }
     break;
  case Ctrl('N'):
     line =  big_picture + twin->curln;
     strncpy(buf, line->data, line->len);
     buf[line->len] = 0;
     if (twin->curln < twin->eline) {
        ++(twin->curln);
        move(twin->curln, twin->curcol);
     }
     break;
  }
  str_trim(buf);
  if (*buf)
     fprintf(flog, "%s%s: %s%s\n",
        (twin->eline == b_lines - 1) ? "[1;33m" : "",
        (twin->eline == b_lines - 1) ?
        getuserid(currutmp->destuid) : cuser.userid, buf,
        (ch == Ctrl('P')) ? "[37;45m(Up)[m" : "[m");
}


static
do_talk(fd)
   int fd;
{
   struct talk_win mywin, itswin;
   time_t talkstart;
   usint myword = 0,itword = 0;
   char mid_line[128], data[200];
   int i, ch, datac;
   int im_leaving = 0;
   struct tm *ptime;
   time_t now;
   char genbuf[200], fpath[100];

   time(&now);
   ptime = localtime(&now);

   sethomepath(fpath, cuser.userid);
   strcpy(fpath, tempnam(fpath, "talk_"));
   flog = fopen(fpath, "w");

   setutmpmode(TALK);

   ch = 58 - strlen(save_page_requestor);
   sprintf(genbuf, "%s¡i%s", cuser.userid, cuser.username);
   i = ch - strlen(genbuf);
   if (i >= 0)
   {
      i = (i >> 1) + 1;
   }
   else
   {
     genbuf[ch] = '\0';
     i = 1;
   }
   memset(data, ' ', i);
   data[i] = '\0';

   sprintf(mid_line, COLOR2"  ­·¹Ð¹ï¸Ü  [1m"COLOR1"%s%s¡j [37m»P  "COLOR1"%s%s[m",
     data, genbuf, save_page_requestor,  data);

   memset(&mywin, 0, sizeof(mywin));
   memset(&itswin, 0, sizeof(itswin));

   i = b_lines >> 1;
   mywin.eline = i - 1;
   itswin.curln = itswin.sline = i + 1;
   itswin.eline = b_lines - 1;

   clear();
   move(i, 0);
   outs(mid_line);
   move(0, 0);

   add_io(fd, 0);
   talkstart = time(0);

   while (1)
   {
     ch = igetkey();

     if (ch == I_OTHERDATA)
     {
       datac = recv(fd, data, sizeof(data), 0);
       if (datac <= 0)
         break;
/* shakalaca.000808: ¥ý mark °_¨Ó.. :p */
#if 0
       if(data[0] == Ctrl('A'))
       {
//         extern int BWboard();
//         if(BWboard(fd,1)==-2)
         if(DL_func("SO/bwboard.so:vaBWboard",fd,1)==-2)
           break;
         continue;
       }
#endif

       itword++;
       for (i = 0; i < datac; i++)
         do_talk_char(&itswin, data[i]);
     }
     else
     {
#if 0
       if(ch == Ctrl('A'))
       {
//         extern int BWboard();
         data[0] = (char) ch;
         if(send(fd, data, 1, 0) != 1)
           break;
//         if (BWboard(fd,0)==-2)
         if(DL_func("SO/bwboard.so:vaBWboard",fd,0)==-2)
           break;
       }
#endif

       if (ch == Ctrl('C'))
       {
         if (im_leaving)
           break;
         move(b_lines, 0);
         clrtoeol();
         outs("¦A«ö¤@¦¸ Ctrl-C ´N¥¿¦¡¤¤¤î½Í¸ÜÅo¡I");
         im_leaving = 1;
         continue;
       }
       if (im_leaving)
       {
         move(b_lines, 0);
         clrtoeol();
         im_leaving = 0;
       }
       switch(ch)
       {
        case KEY_LEFT:
          ch = Ctrl('B');
          break;
        case KEY_RIGHT:
          ch = Ctrl('F');
          break;
        case KEY_UP:
          ch = Ctrl('P');
          break;
        case KEY_DOWN:
          ch = Ctrl('N');
          break;
       }
       myword++;
       data[0] = (char) ch;
       if (send(fd, data, 1, 0) != 1)
         break;
       do_talk_char(&mywin, *data);
     }
   }

   add_io(0, 0);
   close(fd);

   if (flog) 
   {
#if 0
     extern screenline *big_picture;
     extern uschar scr_lns;
#endif
     char buf[512];

     time(&now);
#if 0
     fprintf(flog, "\n[33;44mÂ÷§Oµe­± [%s] ...     [m\n", Cdatelite(&now));
     for (i = 0; i < scr_lns; i++)
       fprintf(flog, "%.*s\n", big_picture[i].len, big_picture[i].data);
#endif
     fclose(flog);

     more(fpath, NA);

     sprintf(buf, "¹ï¸Ü°O¿ý [1;36m(%s)[m", getuserid(currutmp->destuid));
          
     if (getans("²M°£(C) ²¾¦Ü³Æ§Ñ¿ý(M) (C/M)?[C]") == 'm')
       mail2user(cuser, buf, fpath);
     unlink(fpath);
     flog = 0;
   }
   setutmpmode(XINFO);
}


static void
my_talk(uin)
  user_info *uin;
{
  int sock, msgsock, length, ch;
  struct sockaddr_in sin;
  pid_t pid;
  char c;
  char genbuf[4];
  uschar mode0 = currutmp->mode;

  ch = uin->mode;
  strcpy(currutmp->chatid,uin->userid);
  strcpy(currauthor, uin->userid);

  if (ch == EDITING || ch == TALK || ch == CHATING
      || ch == PAGE || ch == MAILALL || !ch && (uin->chatid[0] == 1 || uin->chatid[0] == 3))
    pressanykey("¤H®a¦b¦£°Õ");
  else if (!HAS_PERM(PERM_SYSOP) && (be_rejected(uin->userid) ||
      (!uin->pager && !can_override(uin->userid, cuser.userid))))
    pressanykey("¹ï¤èÃö±¼©I¥s¾¹¤F");
  else if (!HAS_PERM(PERM_SYSOP) &&
           be_rejected(uin->userid) || uin->pager == 2)
    pressanykey("¹ï¤è©Þ±¼©I¥s¾¹¤F");
  else if (!HAS_PERM(PERM_SYSOP) &&
           !(is_friend(uin) & 5) && uin->pager == 4)
    pressanykey("¹ï¤è¥u±µ¨ü¦n¤Íªº©I¥s");
  else if (!(pid = uin->pid) || (kill(pid, 0) == -1))
  {
    resetutmpent();
    pressanykey(msg_usr_left);
  }
  else
  {
//      showplans(uin->userid);
      getdata(2, 0, "§ä¥L [y]²á¤Ñ [N] ", genbuf, 4, LCECHO,0);

    if (*genbuf == 'y' )
    {
      uin->turn = 0;
      log_usies("TALK ", uin->userid);
    }
    else
      return;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
      return;

#if     defined(__OpenBSD__)                    /* lkchu */

    if (!(h = gethostbyname(MYHOSTNAME)))
      return -1;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = 0;
    memcpy(&sin.sin_addr, h->h_addr, h->h_length);

#else

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = 0;
    memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

    length = sizeof(sin);
    if (bind(sock, (struct sockaddr *) &sin, length) < 0 || getsockname(sock, (struct sockaddr *) &sin, &length) < 0)
    {
      close(sock);
      return;
    }

    currutmp->sockactive = YEA;
    currutmp->sockaddr = sin.sin_port;
    currutmp->destuid = uin->uid;
    setutmpmode(PAGE);

    uin->destuip = currutmp;
    kill(pid, SIGUSR1);
    clear();
    prints("¥¿©I¥s %s.....\nÁä¤J Ctrl-D ¤¤¤î....", uin->userid);

    listen(sock, 1);
    add_io(sock, 20);
    while (1)
    {
      ch = igetch();
      if (ch == I_TIMEOUT)
      {
        ch = uin->mode;
        if (!ch && uin->chatid[0] == 1 && uin->destuip == currutmp)
        {
          bell();
          outmsg("¹ï¤è¦^À³¤¤...");
          refresh();
        }
        else if (ch == EDITING || ch == TALK || ch == CHATING
             || ch == PAGE || ch == MAILALL || !ch && (uin->chatid[0] == 1 || uin->chatid[0] == 3))
        {
          add_io(0, 0);
          close(sock);
          currutmp->sockactive = currutmp->destuid = 0;
          pressanykey("¤H®a¦b¦£°Õ");
          return;
        }
        else
        {
#ifdef LINUX
          add_io(sock, 20);       /* added 4 linux... achen */
#endif
          move(0, 0);
          outs("¦A");
          bell();
          uin->destuip = currutmp;

          if (kill(pid, SIGUSR1) == -1)
          {
#ifdef LINUX
            add_io(sock, 20);       /* added 4 linux... achen */
#endif
            pressanykey(msg_usr_left);
            refresh();
            return;
          }
          continue;
        }
      }

      if (ch == I_OTHERDATA)
        break;

      if (ch == '\004')
      {
        add_io(0, 0);
        close(sock);
        currutmp->sockactive = currutmp->destuid = 0;
        return;
      }
    }

    msgsock = accept(sock, (struct sockaddr *) 0, (int *) 0);

    if (msgsock == -1)
    {
      perror("accept");
      return;
    }
    add_io(0, 0);
    close(sock);
    currutmp->sockactive = NA;
    /* currutmp->destuid = 0 ; */
    read(msgsock, &c, sizeof c);

    if (c == 'y')
    {
      sprintf(save_page_requestor, "%s (%s)", uin->userid, uin->username);
      do_talk(msgsock);
    }
    else
    {
      move(9, 9);
      outs("¡i¦^­µ¡j ");
      switch (c)
      {
      case 'a':
        outs("§Ú²{¦b«Ü¦£¡A½Ðµ¥¤@·|¨à¦A call §Ú¡A¦n¶Ü¡H");
        break;
      case 'b':
        outs("¹ï¤£°_¡A§Ú¦³¨Æ±¡¤£¯à¸ò§A talk....");
        break;
      case 'c':
        outs("½Ð¤£­n§n§Ú¦n¶Ü¡H");
        break;
      case 'd':
        outs("§ä§Ú¦³¨Æ¶Ü¡H½Ð¥ý¨Ó«H­ò....");
        break;
      case 'e':
      {
        char msgbuf[60];
        read(msgsock, msgbuf, 60);
        outs("¹ï¤£°_¡A§Ú²{¦b¤£¯à¸ò§A talk¡A¦]¬°\n");
        move(10,18);
        outs(msgbuf);
      }
      break;
      default:
        outs("§Ú²{¦b¤£·Q talk °Õ.....:)");
      }
    }
    close(msgsock);
  }
  currutmp->mode = mode0;
  currutmp->destuid = 0;
}


/* ------------------------------------- */
/* ¿ï³æ¦¡²á¤Ñ¤¶­±                        */
/* ------------------------------------- */


#define US_PICKUP       1234
#define US_RESORT       1233
#define US_ACTION       1232
#define US_REDRAW       1231

static int
search_pickup(num, actor, pklist)
  int num;
  int actor;
  pickup pklist[];
{
  char genbuf[IDLEN + 2];

  getdata(b_lines - 1, 0, "½Ð¿é¤J¨Ï¥ÎªÌ©m¦W¡G", genbuf, IDLEN + 1, DOECHO,0);
  move(b_lines - 1, 0);
  clrtoeol();

  if (genbuf[0])
  {
    int n = (num + 1) % actor;
    str_lower(genbuf, genbuf);
    while (n != num)
    {
      if (strstr_lower(pklist[n].ui->userid, genbuf))
        return n;
      if (++n >= actor)
        n = 0;
    }
  }
  return -1;
}


static int
pickup_cmp(i, j)
  pickup *i, *j;
{
  switch (pickup_way)
  {
  case 0:
    {
      register int friend;

      if (friend = j->friend - i->friend)
        return friend;
    }
  case 1:
    return strcasecmp(i->ui->userid, j->ui->userid);
  case 2:
    return (i->ui->mode - j->ui->mode);
  case 3:
    return (i->idle - j->idle);
  case 4:
    return strcasecmp(i->ui->from, j->ui->from);
  case 5:
    return (j->ui->brc_id - i->ui->brc_id);
  }
}

int
pal_type(userid, whoask)
/* return value :
 * 0  : no such user
 * 1  : friend
 * 2  : bad user
 * 4  : aloha
 */
  char *userid;
  char *whoask;
{
  char buf[STRLEN];
  int fd, can = 0;
  PAL pal;

  sethomefile(buf, userid, FN_PAL);
  if ((fd = open(buf, O_RDONLY)) >= 0)
  {
    while (read(fd, &pal, sizeof(pal)) == sizeof(pal))
    {
      if (!strcmp(pal.userid, whoask))
      {
        can = pal.ftype;
        break;
      }
    }
    close(fd);
  }

  return can;
}

void
friend_add(uident)
  char *uident;
{
  time_t now = time(NULL);
  struct tm *ptime = localtime(&now);

  if (uident[0] > ' ')
  {
    char fpath[80];
    char buf[22];
    PAL pal;

/* itoc.010529: ¦n¤Í¦W³æÀË¬d¤H¼Æ¤W­­ */
    sethomefile(fpath, cuser.userid, FN_PAL);
    if (rec_num(fpath, sizeof(fileheader)) >= MAX_FRIEND)
    {
      pressanykey("¦n¤Í¤H¼Æ¶W¹L¤W­­");
      return;
    }

    pal.ftype = 0;
    strcpy(pal.userid, uident);
    sprintf(fpath, "¹ï©ó %s ªº´y­z¡G", uident); /* ­É fpath ¥Î¤@¤U */
    getdata(2, 0, fpath, buf, 22, DOECHO, 0);
    strncpy(pal.desc, buf, 21);
    getdata(2, 0, "Ãa¤H¶Ü (Y/N) ? [N]", buf, 3, DOECHO, 0);
    if (*buf != 'y')
    {
      pal.ftype |= M_PAL;
/* shakalaca.000120: ÁÙ¦³¤W¯¸³qª¾¥¼§ï.. */
#if 1
      if (strcmp(uident, cuser.userid))
      {
        getdata(2, 0, "¥[¤J¤W¯¸³qª¾¶Ü (Y/N) ? [Y]", buf, 3, DOECHO, 0);
        if (*buf != 'n')
        {
          pal.ftype |= M_ALOHA;
          {
            PAL aloha;

            strcpy(aloha.userid, cuser.userid);
            sethomefile(fpath, uident, FN_ALOHA);
            rec_add(fpath, &aloha, sizeof(aloha));
          }
        }
      }
#endif
    }
    else
      pal.ftype |= M_BAD;

    sprintf(pal.date, "%02d/%02d",  ptime->tm_mon + 1, ptime->tm_mday);
/*  CityLion 000326, ³o¸Ì§ï¨ìcurrdirect, ¬Ý¤å³¹®É, «ö^u¶i¨Ï¥Î¦W³æ¦W³æ */
/*  ¼W¥[¦n¤Í·|¨Ï¤å³¹¦Cªí¶Ã±¼ */
/*  if (currstat == LUSERS)
      sethomefile(currdirect, cuser.userid, FN_PAL);
    rec_add(currdirect, &pal, sizeof(pal)); */
    sethomefile(fpath, cuser.userid, FN_PAL);
    rec_add(fpath, &pal, sizeof(pal));
  }
}


int
cmpuname(userid, pal)
  char *userid;
  PAL *pal;
{
  return (!str_ncmp(userid, pal->userid, sizeof(pal->userid)));
}


static void
friend_delete(uident)
  char *uident;
{
  char fpath[80], ans[4];
  PAL pal;
  int pos;

  setuserfile(fpath, FN_PAL);
  pos = rec_search(fpath, &pal, sizeof(pal), cmpuname, (int) uident);

  if (pos)
  {
    getdata(1, 0, msg_sure_ny, ans, 3, DOECHO, 0);
    if (*ans ==  'y')
      rec_del(fpath, sizeof(PAL), pos, NULL, NULL);
#if 1
    sethomefile(fpath, uident, FN_ALOHA);
    pos = rec_search(fpath, &pal, sizeof(pal), cmpuname, (int) cuser.userid);
    while (pos)
    {
      rec_del(fpath, sizeof(PAL), pos, NULL, NULL);
      pos = rec_search(fpath, &pal, sizeof(pal), cmpuname, (int) cuser.userid);
    }
#endif
  }
}


void
friend_load()
{
  ushort myfriends[MAX_FRIEND];
  ushort myrejects[MAX_REJECT];
  char genbuf[200];
  PAL pal;
  int fd;

  memset(myfriends, 0, sizeof(myfriends));
  memset(myrejects, 0, sizeof(myrejects));
  friendcount = rejected_number = 0;

  setuserfile(genbuf, FN_PAL);
  if ((fd = open(genbuf, O_RDONLY)) > 0)
  {
    ushort unum;

    while (read(fd, &pal, sizeof(pal)))
    {
      if (unum = searchuser(pal.userid))
      {
        if (pal.ftype & M_PAL)
          myfriends[friendcount++] = (ushort) unum;
        else if (pal.ftype & M_BAD)
          myrejects[rejected_number++] = (ushort) unum;
      }
    }
    close(fd);
  }
  memcpy(currutmp->friend, myfriends, sizeof(myfriends));
  memcpy(currutmp->reject, myrejects, sizeof(myrejects));
}

static char *       /* Kaede show friend description */
friend_descript(uident)
  char *uident;
{
  static char *space_buf="                    ";
  static char desc_buf[80];
  char fpath[80];
  int pos;
  PAL pal;

  setuserfile(fpath, FN_PAL);
  pos = rec_search(fpath, &pal, sizeof(pal), cmpuname, (int) uident);

  if (pos)
  {
    strcpy (desc_buf, pal.desc);
    return desc_buf;
  }
  else
    return space_buf;
}

static void
pickup_user()
{
  static int real_name = 0;
  static int show_friend = 0;
  static int show_board = 0;
  static int show_uid = 0;
  static int show_tty = 0;
  static int show_pid = 0;
  static int num = 0;
  char genbuf[200];

#ifdef WHERE
  extern struct FROMCACHE *fcache;
#endif

  register user_info *uentp;
  register pid_t pid0=0;  /* Ptt ©w¦ì                */
  register int   id0;   /*     US_PICKUP®Éªº´å¼Ð¥Î */
  register int state = US_PICKUP, hate, ch;
  register int actor, head, foot;
  int badman, unumber;
  int savemode = currstat;
  time_t diff, freshtime;
  pickup pklist[USHM_SIZE];                     /* parameter Pttµù */
                                                /* num : ²{¦bªº´å¼Ð¦ì */
  char buf[20];                                 /* actor:¦@¦³¦h¤Öuser */
  char pagerchar[4] = "* o ";                   /* foot: ¦¹­¶ªº¸}¸} */
  char *msg_pickup_way[PICKUP_WAYS] =
  { "¶Ù¡IªB¤Í",
    "ºô¤Í¥N¸¹",
    "ºô¤Í°ÊºA",
    "µo§b®É¶¡",
    "¨Ó¦Û¦ó¤è",
    "¨Ï¥Î¬ÝªO"
  };

#ifdef WHERE
  resolve_fcache();
#endif
  while (1)
  {
    if (state == US_PICKUP)
      freshtime = 0;

    if (utmpshm->uptime > freshtime)
    {
      time(&freshtime);
      unumber = count_ulist();
      bfriends_number =  friends_number = override_number =
      rejected_number = actor = ch = 0;

      while (ch < USHM_SIZE)
      {
        uentp = &(utmpshm->uinfo[ch++]);
        if (uentp->pid)
        {
          if ((uentp->invisible && !(HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_SEECLOAK)))
            || ((is_rejected(uentp) & HRM) && !HAS_PERM(PERM_SYSOP)))
            continue;           /* Thor: can't see anyone who rejects you. */
          if (uentp->userid[0] == 0) continue;  /* Ptt's bug */

          if (!PERM_HIDE(currutmp) && PERM_HIDE(uentp))
             continue;

          head = is_friend(uentp) ;
          if ( (cuser.uflag & FRIEND_FLAG)
                && (!head || is_rejected(uentp)) )
            continue;

#ifdef SHOW_IDLE_TIME
          {
            if(!uentp->lastact) uentp->lastact = time(0);
            diff = freshtime - uentp->lastact;

#ifdef DOTIMEOUT
            /* prevent fault /dev mount from kicking out users */

            if ((diff > IDLE_TIMEOUT) && (diff < 60 * 60 * 24 * 5))
            {
              if ((kill(uentp->pid, SIGHUP) == -1) && (errno == ESRCH))
                memset(uentp, 0, sizeof(user_info));
              continue;
            }
#endif
          }
          pklist[actor].idle = diff;
#endif

          pklist[actor].friend = head;
          pklist[actor].ui = uentp;

          actor++;
        }
      }
      badman = rejected_number;

      state = US_PICKUP;
      if (!actor)
      {
        if (getans("§AªºªB¤ÍÁÙ¨S¤W¯¸¡A­n¬Ý¬Ý¤@¯ëºô¤Í¶Ü(Y/N)¡H[Y]") != 'n')
        {
          cuser.uflag &= ~FRIEND_FLAG;
          continue;
        }
        return;
      }
    }

    if (state >= US_RESORT)
      qsort(pklist, actor, sizeof(pickup), pickup_cmp);

    if (state >= US_ACTION)
    {
      sprintf(tmpbuf,"%s [½u¤W %d ¤H]",BOARDNAME, unumber);
      showtitle((cuser.uflag & FRIEND_FLAG)? "¦n¤Í¦Cªí": "¥ð¶¢²á¤Ñ", tmpbuf);
      prints(" ±Æ§Ç¡G[%s]     [1;32m§ÚªºªB¤Í¡G%-3d "
        "[33m»P§Ú¬°¤Í¡G%-3d [36mªO¤Í¡G%-3d [31mÃa¤H¡G%-3d[m\n"
        COLOR1"[1m  %sTP%c¥N¸¹         %-17s%-17s%-13s%-10s[m\n",
        msg_pickup_way[pickup_way], 
        friends_number, override_number,bfriends_number, badman,
#ifdef SHOWUID
        show_uid ? "UID" :
#endif
        "No.",
        (HAS_PERM(PERM_SEECLOAK) || HAS_PERM(PERM_SYSOP)) ? 'C' : ' ',

#ifdef REALINFO
        real_name ? "©m¦W" :
#endif

        "¼ÊºÙ", show_friend ? "¦n¤Í´y­z" : show_board ? "¨Ï¥Î¬ÝªO" : "¬G¶m",

#ifdef SHOWTTY
        show_tty ? "TTY " :
#endif
        "°ÊºA",
#ifdef SHOWPID
        show_pid ? "       PID" :
#endif
#ifdef SHOW_IDLE_TIME
        " ¤ß±¡  µo§b"
#else
        " ¤ß±¡"
#endif

        );
    }
    else
    {
      move(3, 0);
      clrtobot();
    }

    if(pid0)
       for (ch = 0; ch < actor; ch++)
        {
          if(pid0 == (pklist[ch].ui)->pid &&
           id0  == 256 * pklist[ch].ui->userid[0] + pklist[ch].ui->userid[1])
            {
               num = ch;
            }
        }

    if (num < 0)
      num = 0;
    else if (num >= actor)
      num = actor - 1;

    head = (num / p_lines) * p_lines;
    foot = head + p_lines;
    if (foot > actor)
      foot = actor;


    for (ch = head; ch < foot; ch++)
    {
#ifdef HYPER_BBS
      char hbuf[512];      

      sprintf(hbuf,"\033[200m\033[400m\033[444m\033[300m\033[%dm\033[%dm\033[%dm\033[%dm\033[%dm\033[613m\033[713m",
	  ((ch+1)/10000)+648,
	  (((ch+1)%10000)/1000)+648,
	  (((ch+1)%1000)/100)+648,
	  (((ch+1)%100)/10)+648,
	  ((ch+1)%10)+648);
#endif

      uentp = pklist[ch].ui;
      if (!uentp->pid)
      {
        state = US_PICKUP;
        break;
      }
#ifdef SHOW_IDLE_TIME
      diff = pklist[ch].idle;
      if (diff > 0)
        sprintf(buf, "%3d'%02d", diff / 60, diff % 60);
      else
        buf[0] = '\0';
#else
      buf[0] = '\0';
#endif
#ifdef SHOWPID
      if (show_pid)
        sprintf(buf, "%6d", uentp->pid);
#endif
      state = (currutmp == uentp) ? 10 : pklist[ch].friend;
      if (PERM_HIDE(uentp) && HAS_PERM(PERM_SYSOP))
         state = 9;
      hate = is_rejected(uentp);
      diff = uentp->pager & !(hate & HRM);
#ifdef HYPER_BBS
      prints("%5d %c%c%s%-13s%-17.16s[m%-16.16s %-13.13s %s%-4.4s%s%s[201m[m\n",
#else
      prints("%5d %c%c%s%-13s%-17.16s[m%-16.16s %-13.13s %s%-4.4s%s[m\n",
#endif
#ifdef SHOWUID
      show_uid ? uentp->uid :
#endif
      (ch + 1),
      (hate & HRM)? 'X' :
      (uentp->pager == 4) ? 'f' : (uentp->pager == 3) ? 'W' :
      (uentp->pager == 2) ? '-' : pagerchar[(state & 2) | diff],
      (uentp->invisible ? ')' : ' '),
      (hate & IRH)? fcolor[8] : fcolor[state],
      uentp->userid,
#ifdef REALINFO
      real_name ? uentp->realname :
#endif
      uentp->username,
      show_friend ? friend_descript(uentp->userid) :
      show_board ? (char *)getbname(uentp->brc_id) :
      ((uentp->pager != 2 && uentp->pager != 3 && diff || HAS_PERM(PERM_SYSOP)) ?
#ifdef WHERE
      uentp->from_alias ? fcache->replace[uentp->from_alias] : uentp->from
#else
      uentp->from
#endif
      : "*" ),
#ifdef SHOWTTY
      show_tty ? uentp->tty :
#endif
      modestring(uentp, 0),
      uentp->birth ? fcolor[8] : fcolor[0],
      uentp->birth ? "Ã~¬P" : uentp->feeling[0] ? uentp->feeling : "¤£©ú" ,
      buf
#ifdef HYPER_BBS
      ,hbuf);
#else
      );
#endif
    }

    if (state == US_PICKUP)
      continue;

    move(b_lines, 0);
    outs(COLOR1"[1;33m (TAB/f)[37m±Æ§Ç/¦n¤Í [33m(t)[37m²á¤Ñ "
"[33m(a/d/o)[37m¥æ¤Í [33m(q)[37m¬d¸ß [33m(w)[37m¦©À³ "
"[33m(m)[37m±H«H [33m(Ctrl+Z)[37m½u¤W»²§U [m");
    state = 0;
    while (!state)
    {
      ch = cursor_key(num + 3 - head, 0);
      if (ch == KEY_RIGHT || ch == '\n' || ch == '\r')
        ch = 't';

      switch (ch)
      {
      case KEY_LEFT:
      case 'e':
      case 'E':
        return;

        case KEY_TAB:
        {
          char ans[2];
          getdata(b_lines-1, 0,"±Æ§Ç¤è¦¡ 1.¦n¤Í 2.¥N¸¹ 3.°ÊºA 4.µo§b 5.¬G¶m 6.¬ÝªO"
            ,ans, 2, LCECHO,"1");
            if(!ans[0])
            {
              state = US_REDRAW;
              break;
            }

          pickup_way = atoi(ans) - 1;
          if(pickup_way > 5 || pickup_way < 0) pickup_way = 0;
//          pickup_way = (pickup_way +1) %PICKUP_WAYS;
          state = US_PICKUP;
          num = 0;
          break;
        }

      case KEY_DOWN:
      case 'n':
        if (++num < actor)
        {
          if (num >= foot)
            state = US_REDRAW;
          break;
        }

      case '0':
      case KEY_HOME:
        num = 0;
        if (head)
          state = US_REDRAW;
        break;

      case 'N':
         if (HAS_PERM(PERM_BASIC)) {
            char buf[100];
            sprintf(buf, "¼ÊºÙ [%s]¡G", currutmp->username);
            if (!getdata(1, 0, buf, currutmp->username, 17, DOECHO,0))
               strcpy(currutmp->username, cuser.username);

            state = US_PICKUP;
         }
         break;

      case 'M':
         if (HAS_PERM(PERM_BASIC)) {
            char buf[64];
            sprintf(buf, "¤ß±¡ [%s]¡G", currutmp->feeling);
            if (!getdata(1, 0, buf, currutmp->feeling, 5, DOECHO, currutmp->feeling))
              strcpy(currutmp->feeling, cuser.feeling);
            state = US_PICKUP;
         }
         break;

      case 'H':
         if (HAS_PERM(PERM_SYSOP)) {
            currutmp->userlevel ^= PERM_DENYPOST;
            state = US_PICKUP;
         }
         break;

      case 'D':
         if (HAS_PERM(PERM_SYSOP)) {
            char buf[100];

            sprintf(buf, "¥N¸¹ [%s]¡G", currutmp->userid);
            if (!getdata(1, 0, buf, currutmp->userid, IDLEN + 1, DOECHO,0))
               strcpy(currutmp->userid, cuser.userid);

            state = US_PICKUP;
         }
         break;
      case 'F':
        {
            char buf[100];
            sprintf(buf, "¬G¶m [%s]¡G", currutmp->from);
            if (getdata(1, 0, buf, currutmp->from, 17, DOECHO,currutmp->from))
            currutmp->from_alias=0;
            state = US_PICKUP;
          }
         break;
      case ' ':
      case KEY_PGDN:
      case Ctrl('F'):
        if (foot < actor)
        {
          num += p_lines;
          state = US_REDRAW;
          break;
        }
        if (head)
          num = 0;
        state = US_PICKUP;
        break;

      case KEY_UP:
        if (--num < head)
        {
          if (num < 0)
          {
            num = actor - 1;
            if (actor == foot)
              break;
          }
          state = US_REDRAW;
        }
        break;

      case KEY_PGUP:
      case Ctrl('B'):
      case 'P':
        if (head)
        {
          num -= p_lines;
          state = US_REDRAW;
          break;
        }

      case KEY_END:
      case '$':
        num = actor - 1;
        if (foot < actor)
          state = US_REDRAW;
        break;

      case '/':
        {
          int tmp;
          if ((tmp = search_pickup(num, actor, pklist)) >= 0)
            num = tmp;
          state = US_REDRAW;
        }
        break;

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        {                       /* Thor: ¥i¥H¥´¼Æ¦r¸õ¨ì¸Ó¤H */
          int tmp;
          if ((tmp = search_num(ch, actor - 1)) >= 0)
            num = tmp;
          state = US_REDRAW;
        }
        break;

#ifdef  REALINFO
      case 'R':         /* Åã¥Ü¯u¹ê©m¦W */
        if (HAS_PERM(PERM_SYSOP))
          real_name ^= 1;
        state = US_PICKUP;
        break;
#endif
#ifdef  SHOWUID
      case 'U':
        if (HAS_PERM(PERM_SYSOP))
          show_uid ^= 1;
        state = US_PICKUP;
        break;
#endif
#ifdef  SHOWTTY
      case 'Y':
        if (HAS_PERM(PERM_SYSOP))
          show_tty ^= 1;
        state = US_PICKUP;
        break;
#endif
#ifdef  SHOWPID
      case 'I':
        if (HAS_PERM(PERM_SYSOP))
          show_pid ^= 1;
        state = US_PICKUP;
        break;
#endif

      case 'b':         /* broadcast */
        if(HAS_PERM(PERM_SYSOP) || cuser.uflag & FRIEND_FLAG)
        {
          int actor_pos = actor;
          char ans[4];
          state = US_PICKUP;
          if (!getdata(0, 0, "¼s¼½°T®§:", genbuf + 1, 60, DOECHO,0))
            break;
          if (getdata(0, 0, "½T©w¼s¼½? [Y]", ans, 4, LCECHO,0) && *ans == 'n')
            break;
          genbuf[0] = HAS_PERM(PERM_SYSOP) ? 2 : 0;
          while (actor_pos)
          {
            uentp = pklist[--actor_pos].ui;
            if (uentp->pid &&
                currpid != uentp->pid &&
                kill(uentp->pid, 0) != -1 &&
                (HAS_PERM(PERM_SYSOP) || (uentp->pager != 3 &&
                 (uentp->pager != 4 || is_friend(uentp) & 5))))
                my_write(uentp->pid, genbuf);
        }
      }
      break;

      case 's':         /* Åã¥Ü¦n¤Í´y­z */
        show_friend ^= 1;
        state = US_PICKUP;
        break;

      case 'B':         /* Åã¥Ü¥¿¦b¬ÝªºªO */
        if (!HAS_PERM(PERM_SYSOP))
          continue;
        show_board ^= 1;
        state = US_PICKUP;
        break;

      case 'u':         /* ½u¤W­×§ï¸ê®Æ */
        if (!HAS_PERM(PERM_ACCOUNTS) || !HAS_PERM(PERM_SYSOP))
          continue;
      case 'K':         /* §âÃa³J½ð¥X¥h */
      case 'g':         /* ­×§ïRPG¸ê®Æ */
        if (!HAS_PERM(PERM_SYSOP))
          continue;
        state = US_ACTION;
        break;

      case 't':
      case 'w':
        if (!HAS_PERM(PERM_PAGE))
          continue;
        state = US_ACTION;
        break;

      case 'i':
        if (!HAS_PERM(PERM_CLOAK))  /* wildcat:Áô§Î */
          break;
        state = US_ACTION;
        break;

      case 'a':
      case 'd':
      case 'o':
      case 'f':
        if (!HAS_PERM(PERM_LOGINOK))  /* µù¥U¤~¦³ Friend */
          break;
        if (ch == 'f')
        {
          cuser.uflag ^= FRIEND_FLAG;
          state = US_PICKUP;
          break;
        }
        state = US_ACTION;
        break;

      case 'q':
      case 'c':
      case 'm':
      case 'r':
      case 'l':
        if (!cuser.userlevel && ch != 'q' && ch != 'l') /* guest ¥u¯à query */
          break;
      case 'h':
        state = US_ACTION;
        break;
      case 'p':
         if (HAS_PERM(PERM_BASIC)) {
            t_pager();
            state = US_PICKUP;
         }
         break;
      case KEY_ESC:
         if (KEY_ESC_arg == 'c')
            capture_screen();
         else if (KEY_ESC_arg == 'n') {
            edit_note();
            state = US_PICKUP;
         }
         break;
      default:          /* refresh user state */
        state = US_PICKUP;
      }
    }

    if (state != US_ACTION)
        {
         pid0 = 0;
         continue;
        }
    uentp = pklist[num].ui;
    pid0 = uentp->pid;
    id0  = 256 * uentp->userid[0] + uentp->userid[1];

    if (ch == 'w')
    {
      if ((uentp->pid != currpid) &&
          (HAS_PERM(PERM_SYSOP) || uentp->pager != 3 ||
// §ï¿ù¤F? :p
/* ­×¥¿.. :p */
//          (can_override(uentp->userid, cuser.userid) && uentp->pager == 4) ))
            (is_friend(uentp) & 5 || uentp->pager != 4) ))
      {
        cursor_show(num + 3 - head, 0);
        my_write(uentp->pid, "¼ö½u Call-In¡G");
      }
      else
        state = 0;
    }
    else if (ch == 'l')
    {                           /* Thor: ¬Ý Last call in */
      t_display();
      state = US_PICKUP;
    }
    else
    {
      switch (ch)
      {
      case 'r':
        m_read();
        break;

      case 'a':
        if (!pal_type(cuser.userid, uentp->userid))
        {
          friend_add(uentp->userid, FRIEND_OVERRIDE);
          friend_load();
          state = US_PICKUP;
        }
        break;

      case 'd':
        friend_delete(uentp->userid);
        friend_load();
        state = US_PICKUP;
        break;
      case 'o':
        {
          char buf[80];

          setuserfile(buf, FN_PAL);
          ListEdit(buf);
          state = US_PICKUP;
        }
        break;

      case 'K':

        if (uentp->pid && (kill(uentp->pid, 0) != -1))
        {
          move(1, 0);
          clrtobot();
          move(2, 0);
          my_kick(uentp);
          state = US_PICKUP;
        }
        break;

      case 'm':
        if(!cuser.userlevel) break;
        stand_title("±H  «H");
        prints("¦¬«H¤H¡G%s", uentp->userid);
        my_send(uentp->userid);

      case 'q':
        strcpy(currauthor, uentp->userid);
        showplans(uentp->userid);
        break;

      case 'c':
        DL_func("SO/pip.so:pip_data_list_va", uentp->userid);
        /* shakalaca.990704: ¤pÂû­×¥¿ */
        break;

      case 'i':
        u_cloak();
        break;

      case 'u':         /* Thor: ¥i½u¤W¬d¬Ý¤Î­×§ï¨Ï¥ÎªÌ */
        {
          int id;
          userec muser;
           strcpy(currauthor, uentp->userid);
          stand_title("¨Ï¥ÎªÌ³]©w");
          move(1, 0);
          if (id = getuser(uentp->userid))
          {
            memcpy(&muser, &xuser, sizeof(muser));
            uinfo_query(&muser, 1, id);
          }
         }
        break;

      case 't':
        if (uentp->pid != currpid)
        {
          move(1, 0);
          clrtobot();
          move(3, 0);
          my_talk(uentp);
          state = US_PICKUP;
        }
      }
      state = US_PICKUP;
    }
    setutmpmode(savemode);
  }
}


int
t_users()
{
  int destuid0 = currutmp->destuid;

  if (chkmailbox())
    return;

  setutmpmode(LUSERS);
  pickup_user();
  currutmp->destuid = destuid0;
  return 0;
}


int
t_pager()
{
  currutmp->pager = (currutmp->pager + 1) % 5;
  return 0;
}


int
t_idle()
{
  int destuid0 = currutmp->destuid;
  int mode0 = currutmp->mode;
  int stat0 = currstat;
  char genbuf[20];

  setutmpmode(IDLE);

  genbuf[0] = getans("²z¥Ñ¡G[0]µo§b (1)±µ¹q¸Ü (2)³V­¹ (3)¥´½OºÎ (4)¸Ë¦º (5)Ã¹¤¦ (6)¨ä¥L (Q)¨S¨Æ¡H");
  if (genbuf[0] == 'q')
  {
    currutmp->mode = mode0;
    currstat = stat0;
    return 0;
  }
  else if (genbuf[0] >= '1' && genbuf[0] <= '6')
    currutmp->destuid = genbuf[0] - '0';
  else
    currutmp->destuid = 0;

  if (currutmp->destuid == 6)
    if (!cuser.userlevel || !getdata(b_lines, 0, "µo§bªº²z¥Ñ¡G", currutmp->chatid, 11, DOECHO, 0))
      currutmp->destuid = 0;
  {
    char buf[80], passbuf[PASSLEN];
    do
    {
      move(b_lines - 1, 0);
      clrtoeol();
      sprintf(buf, "(Âê©w¿Ã¹õ)µo§b­ì¦]: %s", (currutmp->destuid != 6) ?
         IdleTypeTable[currutmp->destuid] : currutmp->chatid);
      outs(buf);
      refresh();
      getdata(b_lines, 0, MSG_PASSWD, passbuf, PASSLEN, PASS, 0);
      passbuf[8]='\0';
    } while(!chkpasswd(cuser.passwd, passbuf) && strcmp(STR_GUEST,cuser.userid));
  }
  currutmp->mode = mode0;
  currutmp->destuid = destuid0;
  currstat = stat0;

  return 0;
}


int
t_query()
{
  char uident[STRLEN];

  stand_title("¬d¸ßºô¤Í");
  usercomplete(msg_uid, uident);
  if(!getuser(uident))
  {
    pressanykey("³o¸Ì¨S³o­Ó¤H");
    return 0;
  }
  else 
    showplans(uident);
  return 0;
}


#if 0
/* shakalaca.000813: user list ´N¥i¥H¥Î enter ¿ï¾Ü¤F.. */
int
t_talk()
{
  char uident[16];
  int tuid, unum, ucount;
  user_info *uentp;
  char genbuf[4];

  if (count_ulist() <= 1)
  {
    outs("¥Ø«e½u¤W¥u¦³±z¤@¤H¡A§ÖÁÜ½ÐªB¤Í¨Ó¥úÁ{¡i" BOARDNAME "¡j§a¡I");
    return XEASY;
  }
  stand_title("¥´¶}¸Ü§X¤l");
  creat_list();
  namecomplete(msg_uid, uident);
  if (uident[0] == '\0')
    return 0;

  move(3, 0);
  if (!(tuid = searchuser(uident)) || tuid == usernum)
  {
    pressanykey(err_uid);
    return 0;
  }

  /* ----------------- */
  /* multi-login check */
  /* ----------------- */

  unum = 1;
  while ((ucount = count_logins(cmpuids, tuid, 0)) > 1)
  {
    outs("(0) ¤£·Q talk ¤F...\n");
    count_logins(cmpuids, tuid, 1);
    getdata(1, 33, "½Ð¿ï¾Ü¤@­Ó²á¤Ñ¹ï¶H [0]¡G", genbuf, 4, DOECHO,0);
    unum = atoi(genbuf);
    if (unum == 0)
      return 0;
    move(3, 0);
    clrtobot();
    if (unum > 0 && unum <= ucount)
      break;
  }

  if (uentp = (user_info *) search_ulistn(cmpuids, tuid, unum))
    my_talk(uentp);

  return 0;
}
#endif


/* ------------------------------------- */
/* ¦³¤H¨Ó¦êªù¤l¤F¡A¦^À³©I¥s¾¹            */
/* ------------------------------------- */


void
talkreply()
{
  int a;
  struct hostent *h;
  char hostname[STRLEN],buf[80];
  struct sockaddr_in sin;
  char genbuf[200];
  int pkmode = 0;  // ¬PªÅÂû¹ï¾Ô
  user_info *uip;

  uip = currutmp->destuip;
  sprintf(page_requestor, "%s (%s)", uip->userid, uip->username);
  currutmp->destuid = uip->uid;
  currstat = XMODE;             /* Á×§K¥X²{°Êµe */

  clear();
  outs("\n"
"       (Y) Åý§Ú­Ì talk §a¡I     (A) §Ú²{¦b«Ü¦£¡A½Ðµ¥¤@·|¨à¦A call §Ú"
"       (N) §Ú²{¦b¤£·Q talk      (B) ¹ï¤£°_¡A§Ú¦³¨Æ±¡¤£¯à¸ò§A talk"
"       (C) ½Ð¤£­n§n§Ú¦n¶Ü¡H     (D) ¦³¨Æ¶Ü¡H½Ð¥ý¨Ó«H"
"       (E) [1;33m§Ú¦Û¤v¿é¤J²z¥Ñ¦n¤F...[m\n\n");

  getuser(uip->userid);
  currutmp->msgs[0].last_pid = uip->pid;
  strcpy(currutmp->msgs[0].last_userid, uip->userid);
  strcpy(currutmp->msgs[0].last_call_in, "©I¥s¡B©I¥s¡AÅ¥¨ì½Ð¦^µª");
  prints("¹ï¤è¨Ó¦Û [%s]¡A¦@¤W¯¸ %d ¦¸¡A¤å³¹ %d ½g\n",
    uip->from, xuser.numlogins, xuser.numposts);
//  showplans(uip->userid);
  show_last_call_in();
  sprintf(genbuf, "§A·Q¸ò %s %s¶Ü¡H½Ð¿ï¾Ü(Y/N/A/B/C/D/E)[Y] ",
    page_requestor, "²á¤Ñ");
  getdata(0, 0, genbuf, buf, 4, LCECHO,0);

  if (uip->mode != PAGE) {
     sprintf(genbuf, "%s¤w°±¤î©I¥s¡A«öEnterÄ~Äò...", page_requestor);
     getdata(0, 0, genbuf, buf, 4, LCECHO,0);
     return;
  }

  currutmp->msgcount = 0;
  strcpy(save_page_requestor, page_requestor);
  memset(page_requestor, 0, sizeof(page_requestor));
  gethostname(hostname, STRLEN);
  if (!(h = gethostbyname(hostname)))
  {
    perror("gethostbyname");
    return;
  }
  memset(&sin, 0, sizeof sin);
  sin.sin_family = h->h_addrtype;
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);
  sin.sin_port = uip->sockaddr;
  a = socket(sin.sin_family, SOCK_STREAM, 0);
  if ((connect(a, (struct sockaddr *) & sin, sizeof sin)))
  {
    perror("connect err");
    return;
  }
  if (!buf[0] || !strchr("abcdefn", buf[0]))
  {
    if(pkmode)
      uip->destuip = currutmp;
    buf[0] = 'y';
  }
  write(a, buf, 1);
  if (buf[0] == 'e'/* || buf[0] == 'E' */)
  /* shakalaca.000813: ¤£¥i¯à·|¦³¤j¼g¶Ç¦^, ¦]¬° LCECHO ¥u·|¶Ç¦^¤p¼g¦r¥À */
   {
     if (!getdata(b_lines, 0, "¤£¯à talk ªº­ì¦]¡G", genbuf, 60, DOECHO,0))
       strcpy(genbuf, "¤£§i¶D§A«¨ !! ^o^");
     write(a, genbuf, 60);
   }

  if (buf[0] == 'y')
  {
    strcpy(currutmp->chatid, uip->userid);
    do_talk(a);
  }
  else
    close(a);

  clear();
}


void
t_aloha()
{
  int i;
  user_info *uentp;
  pid_t pid;
  char buf[100];

  sprintf(buf + 1, "[1;37;41m¡¸ %s(%s) ¤W¯¸¤F! [0m",
    cuser.userid, cuser.username);
  *buf = 0;

  /* Thor: ¯S§Oª`·N, ¦Û¤v¤W¯¸¤£·|³qª¾¦Û¤v... */

  for (i = 0; i < USHM_SIZE; i++) 
  {
    uentp = &utmpshm->uinfo[i];
    if ((pid = uentp->pid) && (kill(pid, 0) != -1) &&
        uentp->pager && (is_friend(uentp) & 5) &&
        strcmp(uentp->userid, cuser.userid))
      my_write(uentp->pid, buf);
  }
}



/* ------------------------------------- */
/* ºô¤Í°ÊºAÂ²ªí                          */
/* ------------------------------------- */

#if 0
int
shortulist(uentp)
  user_info *uentp;
{
  static int lineno, fullactive, linecnt;
  static int moreactive, page, num;
  char uentry[50];
  int state;

  if (!lineno)
  {
    lineno = 3;
    page = moreactive ? (page + p_lines * 3) : 0;
    linecnt = num = moreactive = 0;
    move(1, 70);
    prints("Page: %d", page / (p_lines) / 3 + 1);
    move(lineno, 0);
  }
  if (uentp == NULL)
  {
    int finaltally;

    clrtoeol();
    move(++lineno, 0);
    clrtobot();
    finaltally = fullactive;
    lineno = fullactive = 0;
    return finaltally;
  }
  if ((!HAS_PERM(PERM_SYSOP) && !HAS_PERM(PERM_SEECLOAK) && uentp->invisible) ||
      ((is_rejected(uentp) & HRM) && !HAS_PERM(PERM_SYSOP)))
  {
    if (lineno >= b_lines)
      return 0;
    if (num++ < page)
      return 0;
    memset(uentry, ' ', 25);
    uentry[25] = '\0';
  }
  else
  {
    fullactive++;
    if (lineno >= b_lines)
    {
      moreactive = 1;
      return 0;
    }
    if (num++ < page)
      return 0;

    state = (currutmp == uentp) ? 10 : is_friend(uentp);

    if (PERM_HIDE(uentp) && HAS_PERM(PERM_SYSOP))
       state = 9;

    sprintf(uentry, "%s%-13s%c%-10s%s ", fcolor[state],
      uentp->userid, uentp->invisible ? '#' : ' ',
      modestring(uentp, 1), state ? "[m" : "");
  }
  if (++linecnt < 3)
  {
    strcat(uentry, "¢x");
    outs(uentry);
  }
  else
  {
    outs(uentry);
    linecnt = 0;
    clrtoeol();
    move(++lineno, 0);
  }
  return 0;
}


static void
do_list(modestr)
  char *modestr;
{
  int count;

  showtitle(modestr, BoardName);

  outc('\n');
  outs(msg_shortulist);

  friends_number = override_number = 0;
  if (apply_ulist(shortulist) == -1)
    outs(msg_nobody);
  else
  {
    time_t thetime = time(NULL);

    count = shortulist(NULL);
    move(b_lines, 0);
    prints(COLOR1"[1m  ¤W¯¸Á`¤H¼Æ¡G%-7d[32m§ÚªºªB¤Í¡G%-6d"
      "[33m»P§Ú¬°¤Í¡G%-8d[30m%-23s[37;40;0m",
      count, friends_number, override_number, Etime(&thetime));
    refresh();
  }
}


int
t_list()
{
  setutmpmode(LUSERS);
  do_list("¨Ï¥ÎªÌª¬ºA");
  igetch();
  return 0;
}

#endif

/* shakalaca.000814: ¥H¤U³o¨â­Ó¨ç¦¡¦b .so ¤¤¦³¥Î¨ì :pp */
/* ¨¾°ô Multi play */
int
count_multiplay(int unmode)
{
  register int i, j;
  register user_info *uentp;
  extern struct UTMPFILE *utmpshm;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (uentp->uid == usernum)
     if(uentp->lockmode == unmode)
      j++;
  }
  return j;
}


int
lockutmpmode(int unmode)
{
  if (count_multiplay(unmode))
  {
   char buf[80];
   sprintf(buf,"©êºp! ±z¤w¦³¨ä¥L½u¬Û¦PªºID¥¿¦b%s",ModeTypeTable[unmode]);
   pressanykey(buf);
   return 1;
  }
  setutmpmode(unmode);
  currutmp->lockmode = unmode;
  return 0;
}


int
unlockutmpmode()
{
  currutmp->lockmode = 0;
}
