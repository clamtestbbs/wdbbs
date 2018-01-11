/*-------------------------------------------------------*/
/* user.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : user configurable setting routines           */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#define _USER_C_

#include "bbs.h"

extern int numboards;
extern boardheader *bcache;
extern void resolve_boards();

char *sex[8] = { MSG_BIG_BOY, MSG_BIG_GIRL, MSG_LITTLE_BOY, MSG_LITTLE_GIRL,
                 MSG_MAN, MSG_WOMAN, MSG_PLANT, MSG_MIME };

void
user_display(u, real)
  userec *u;
  int real;
{
  int diff; 
  int day = 0, hour = 0, min = 0;
  char genbuf[128];

  if (u->sex >= 8) 
    u->sex = 7;

  clrtobot();
  sethomedir(genbuf, u->userid);
  outs("[1;33m¡´¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[42m   ¨Ï¥ÎªÌ¸ê®Æ   [40m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¡´\n");
  prints(""
"  [32m­^¤å¥N¸¹¡G[37m%-16.16s[32m¼ÊºÙ¡G[37m%-20.20s[32m©Ê§O¡G[37m%-8.8s"
"  [32m¯u¹ê©m¦W¡G[37m%-16.16s[32m¦í§}¡G[37m%-40s"
"  [32m¥X¥Í¤é´Á¡G[37m19%02i¦~%02i¤ë%02i¤é  [32m¢ÓMail¡G[37m%-40s\n",
    u->userid,u->username,sex[u->sex],
    u->realname, u->address,
    u->year, u->month, u->day, u->email); 
  prints("  [32m¤W¯¸¦¸¼Æ¡G[37m%-16d[32mµù¥U¤é´Á¡G[37m%s"
    ,u->numlogins,ctime(&u->firstlogin)); 
  prints("  [32m¤å³¹¼Æ¥Ø¡G[37m%-16d[32m«e¦¸¤W¯¸¡G[37m%s"
    ,u->numposts,ctime(&u->lastlogin)); 
  prints("  [32m¨p¤H«H½c¡G[37m%-4d «Ê         [32m§Q®§µo©ñ¡G[37m%s"
    ,rec_num(genbuf, sizeof(fileheader)),ctime(&u->dtime));
  prints(
"  [32m«H½c¤W­­¡G[37m%d «Ê\n"
"  [32m¤H®ð«ü¼Æ¡G[37m%-16ld[32m¦n©_«ü¼Æ¡G[37m%-16ld[32m¤ß±¡¡G[37m%-4.4s\n"
"  [32mµo°T®§¼Æ¡G[37m%-16d[32m¦¬°T®§¼Æ¡G[37m%d\n"
"  [32m¤W¯¸¦aÂI¡G[37m%s \n"
"  [32m¶Ç©I¸¹½X¡G[37m0%d-%s \n"
,(u->exmailbox+MAXKEEPMAIL)
,u->bequery,u->toquery,u->feeling,u->sendmsg,u->receivemsg
,u->lasthost,u->pagermode,u->pagernum);

  if (real)
  {
    strcpy(genbuf, "bTCPRp#@XWBA#VSA?crFG??????????");
    for (diff = 0; diff < 31; diff++)
      if (!(u->userlevel & (1 << diff)))
        genbuf[diff] = '-';
    prints("  [32m»{ÃÒ¸ê®Æ¡G[37m%-50.50s\n",u->justify);
    prints("  [32m¨Ï¥ÎÅv­­¡G[37m%-32s\n",genbuf);
  }
  diff = u->totaltime / 60;
  day = diff / 1440;
  hour = (diff/60)%24;
  min = diff%60;
  prints("  [32m¤W¯¸®É¶¡¡G[37m%d¤Ñ %d¤p®É %d¤À\n",day,hour,min);

  if (u->userlevel >= PERM_BM)
  {
    int i;
    boardheader *bhdr;
    resolve_boards();

    outs("  [32m¾á¥ôªO¥D¡G[37m");

    for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
    {
      if(userid_is_BM(u->userid, bhdr->BM))
          {
              outs(bhdr->brdname);
              outc(' ');
          }
    }
    outc('\n');
  }
  prints("[33m¡´¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¡´[m\n");

  if (!real)
    outs((u->userlevel & PERM_LOGINOK) ?
      "±zªºµù¥Uµ{§Ç¤w¸g§¹¦¨¡AÅwªï¥[¤J¥»¯¸" :
      "¦pªG­n´£ª@Åv­­¡A½Ð°Ñ¦Ò¥»¯¸¤½§GÄæ¿ì²zµù¥U");

#ifdef  NEWUSER_LIMIT
  if (!real)
    if ((u->lastlogin - u->firstlogin < 3 * 86400) && !HAS_PERM(PERM_POST))
      outs("·s¤â¤W¸ô¡A¤T¤Ñ«á¶}©ñÅv­­");
#endif
}


void
uinfo_query(u, real, unum)
  userec *u;
  int real, unum;
{
  userec x;
  register int i, fail, mail_changed;
  char ans, buf[STRLEN];
  char genbuf[200];
  int flag=0,temp;

  user_display(u, real);

  fail = mail_changed = 0;

  memcpy(&x, u, sizeof(userec));
  ans = getans(real ?
    "(1)­×§ï¸ê®Æ (2)³]©w±K½X (3)³]©wÅv­­ (4)²M°£±b¸¹ (5)§ïID (6)°e«Ê¸¹ [0]µ²§ô " :
    "½Ð¿ï¾Ü (1)­×§ï¸ê®Æ (2)³]©w±K½X ==> [0]µ²§ô ");

  if (ans > '2' && !real)
  {
    ans = '0';
  }

  if (ans == '1' || ans == '3')
  {
    clear();
    i = 2;
    move(i++, 0);
    outs(msg_uid);
    outs(x.userid);
  }

  switch (ans)
  {
  case '1':
    move(0, 0);
    outs("½Ð³v¶µ­×§ï¡C");
    getdata(i++, 0," ¼Ê ºÙ  ¡G",x.username, 24, DOECHO,x.username);
    strip_ansi(x.username,x.username,STRIP_ALL);
    getdata(i++, 0,"©~¦í¦a§}¡G",x.address, 50, DOECHO,x.address);
    strip_ansi(x.address,x.address,STRIP_ALL);
    getdata(i++, 0,"²{¦b¤ß±¡¡G",x.feeling, 5, DOECHO,x.feeling);
    x.feeling[4] = '\0';
    getdata(i++, 0,"¹q¤l«H½c¡G",buf, 50, DOECHO,x.email);
    if(belong_spam(BBSHOME"/etc/spam-list",x.email))
    {
      pressanykey("©êºp,¥»¯¸¤£±µ¨ü§Aªº E-Mail «H½c¦ì¸m");
      strcpy(buf,x.email);
    }
    if (strcmp(buf,x.email) && !not_addr(buf))
    {
      strcpy(x.email,buf);
      mail_changed = 1 - real;
    }
    strip_ansi(x.email,x.email,STRIP_ALL);

    sprintf(buf, "%04d", u->pagermode);
    getdata(i, 0,"©I¥s¾¹ªù¸¹¡G",buf, 5, DOECHO, buf );
    if(!strncmp(buf,"094",3) && buf[3] != '2' && buf[3] != '4')
      x.pagermode = atol(buf);
    else
      x.pagermode = u->pagermode;      

    getdata(i++, 30,"©I¥s¾¹¸¹½X¡G",x.pagernum, 7, DOECHO,x.pagernum);

    sprintf(genbuf,
"©Ê§O (1)¸¯®æ (2)©j±µ (3)©³­} (4)¬ü¬Ü (5)Á¦¨û (6)ªü«¼ (7)´Óª« (8)Äqª« [%i]:",
      u->sex + 1);
    getdata(i++, 0, genbuf,
      buf, 3, DOECHO, 0);
    if (buf[0] >= '1' && buf[0] <= '8')
      x.sex = buf[0] - '1';

    while (1)
    {
      sprintf(genbuf,"%02i/%02i/%02i",u->year,u->month,u->day);
      getdata(i, 0, "¥X¥Í¦~¥÷ 19", buf, 3, DOECHO, genbuf);
      x.year  = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(i, 0, "¥X¥Í¤ë¥÷   ", buf, 3, DOECHO, genbuf+3);
      x.month = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(i, 0, "¥X¥Í¤é´Á   ", buf, 3, DOECHO, genbuf+6);
      x.day   = (buf[0] - '0') * 10 + (buf[1] - '0');
      if (!real && (x.month > 12 || x.month < 1 ||
        x.day > 31 || x.day < 1 || x.year > 90 || x.year < 40))
        continue;
      i++;
      break;
    }
    if (real)
    {
      unsigned long int l;

// wildcat:¤£­nÅý user µù¥U§¹¶Ã§ï¦W¦r? 
      getdata(i++, 0,"¯u¹ê©m¦W¡G",x.realname, 20, DOECHO,x.realname);
      strip_ansi(x.realname,x.realname,STRIP_ALL);
      sprintf(genbuf, "%d", x.numlogins);
      if (getdata(i, 0,"¤W½u¦¸¼Æ¡G", buf, 10, DOECHO,genbuf))
        if ((l = atoi(buf)) >= 0)
          x.numlogins = (int) l;
      sprintf(genbuf,"%d", u->numposts);
      if (getdata(i++, 25, "¤å³¹¼Æ¥Ø¡G", buf, 10, DOECHO,genbuf))
        if ((l = atoi(buf)) >= 0)
          x.numposts = l;
      sprintf(genbuf, "%ld", x.sendmsg);
      if (getdata(i, 0,"µo¤ô²y¼Æ¡G", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.sendmsg = l;
      sprintf(genbuf, "%ld", x.receivemsg);
      if (getdata(i++, 25,"¦¬¤ô²y¼Æ¡G", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.receivemsg = l;
      sprintf(genbuf, "%ld", x.bequery);
      if (getdata(i, 0,"¤H®ð«×¡G", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.bequery = l;
      sprintf(genbuf, "%ld", x.toquery);
      if (getdata(i++, 25,"¦n©_«×¡G", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.toquery = l;
      sprintf(genbuf, "%ld", x.totaltime);
      if (getdata(i++, 0,"¤W¯¸®É¼Æ¡G", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.totaltime = l;
      sprintf(genbuf, "%d", x.exmailbox);
      if (getdata(i++, 0,"ÁÊ¶R«H½c¼Æ¡G", buf, 4, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.exmailbox = (int) l;

      getdata(i++, 0,"»{ÃÒ¸ê®Æ¡G", x.justify, 39, DOECHO, x.justify);
      getdata(i++, 0,"³Ìªñ¥úÁ{¾÷¾¹¡G", x.lasthost, 24, DOECHO,x.lasthost);


      fail = 0;
    }
    break;

  case '2':
    i = 19;
    if (!real)
    {
      if (!getdata(i++, 0, "½Ð¿é¤J­ì±K½X¡G", buf, PASSLEN, PASS,0) ||
        !chkpasswd(u->passwd, buf))
      {
        outs("\n\n±z¿é¤Jªº±K½X¤£¥¿½T\n");
        fail++;
        break;
      }
    }

    if (!getdata(i++, 0, "½Ð³]©w·s±K½X¡G", buf, PASSLEN, PASS,0))
    {
      outs("\n\n±K½X³]©w¨ú®ø, Ä~Äò¨Ï¥ÎÂÂ±K½X\n");
      fail++;
      break;
    }
    strncpy(genbuf, buf, PASSLEN);

    getdata(i++, 0, "½ÐÀË¬d·s±K½X¡G", buf, PASSLEN, PASS,0);
    if (strncmp(buf, genbuf, PASSLEN))
    {
      outs("\n\n·s±K½X½T»{¥¢±Ñ, µLªk³]©w·s±K½X\n");
      fail++;
      break;
    }
    buf[8] = '\0';
    strncpy(x.passwd, genpasswd(buf), PASSLEN);
    break;

  case '3':
    i = setperms(x.userlevel);
    if (i == x.userlevel)
      fail++;
    else {
      flag=1;
      temp=x.userlevel;
      x.userlevel = i;
    }
    break;

  case '4':
    i = QUIT;
    break;

  case '5':
    if (getdata(b_lines - 3, 0, "·sªº¨Ï¥ÎªÌ¥N¸¹¡G", genbuf, IDLEN + 1,
        DOECHO,x.userid))
    {
      if (searchuser(genbuf))
      {
        outs("¿ù»~! ¤w¸g¦³¦P¼Ë ID ªº¨Ï¥ÎªÌ");
        fail++;
      }
      else
      {
        strcpy(x.userid, genbuf);
      }
    }
    break;
  case '6':
    if (getdata(b_lines - 3, 0, "§A­nµ¹¥L¤°»ò«Ê¸¹¡G", genbuf, 18,
        DOECHO,x.title))
    {
      time_t now=time(0);
      strcpy(x.title, genbuf);
      sprintf(genbuf,"%s ­×§ï %s ªº«Ê¸¹¬° [%s] %s",cuser.userid,x.userid,
        x.title,ctime(&now));
      f_cat("log/chtitle.log",genbuf);
    }
    break;

  default:
    return;
  }

  if (fail)
  {
    pressanykey(NULL);
    return;
  }

  if (getans(msg_sure_ny) == 'y')
  {
    if (flag) Security(temp,i,cuser.userid,x.userid);
    if (strcmp(u->userid, x.userid))
    {
      char src[STRLEN], dst[STRLEN];

      sethomepath(src, u->userid);
      sethomepath(dst, x.userid);
      f_mv(src, dst);
      setuserid(unum, x.userid);
    }
    memcpy(u, &x, sizeof(x));
    substitute_record(fn_passwd, u, sizeof(userec), unum);
    if (mail_changed)
    {
#ifdef  REG_EMAIL
      x.userlevel &= ~PERM_LOGINOK;
      mail_justify(cuser);
#endif
    }

    if (i == QUIT)
    {
      char src[STRLEN], dst[STRLEN];

      sprintf(src, "home/%s", x.userid);
      sprintf(dst, "tmp/%s", x.userid);
// wildcat : Â²³æ¦h¤F , ¤£­n¥Î rm home/userid
      if(dashd(src))
        f_mv(src , dst);
/*
woju
*/
      log_usies("KILL", x.userid);
      x.userid[0] = '\0';
      setuserid(unum, x.userid);
    }
    else
       log_usies("SetUser", x.userid);
    substitute_record(fn_passwd, &x, sizeof(userec), unum);
  }
}


int
u_info()
{
  move(1, 0);
  update_data(); 
  uinfo_query(&cuser, 0, usernum);
  strcpy(currutmp->username, cuser.username);
  strcpy(currutmp->feeling, cuser.feeling);
  return 0;
}


int
u_cloak()
{
  pressanykey((currutmp->invisible ^= 1) ? MSG_CLOAKED : MSG_UNCLOAK);
  return XEASY;
}


unsigned
u_habit()
{
  unsigned fbits;
  register int i;
  char choice[4];
  fbits = cuser.habit;
  stand_title("¨Ï¥ÎªÌ³ß¦n³]©w");
  move(2, 0);
  outs("½Ð¨Ì·Ó¨Ï¥Î²ßºD¦Û¦æ½Õ¾ã(¡´¬°¶},   ¬°Ãö)¡G\n");
  move(4, 0);
  for (i = 0; i < NUMHABITS; i++)
  {
    prints("%c. %-28s %-7s\n"
       , 'A' + i, habitstrings[i],((fbits >> i) & 1 ? "¡´" : "  "));
  }
  clrtobot();
  while (getdata(b_lines - 1, 0, "½Ð«ö [A-N] ¤Á´«³]©w¡A«ö [Return] µ²§ô¡G",
      choice, 3, LCECHO, 0))
  {
    i = choice[0] - 'a';
    if (i < 0 || i >= NUMHABITS)
      bell();
    else
    {
      fbits ^= (1 << i);
      move( i%14 + 4, i <= 14 ? 32 : 72);
      prints((fbits >> i) & 1 ? "¡´" : "  ");
    }
    /* for HABIT_COLOR, ±M¬°¤F±m¦â¼Ò¦¡³]ªº, i==1¬Oªí¥Ü²Ä¤G­ÓHABIT,
       ·íHABIT_COLOR¤£¦b²Ä¤G­ÓHABIT®É, ³oÃä¤]­n¬ÛÀ³½Õ¾ã.
                                                - add by wisely - */
    if( i == 1) showansi ^= 1;
  }
  update_data(); /* SiE: sync data prev bug user */
  cuser.habit = fbits;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* °O¿ý */
  return RC_FULL;
}


void
showplans(char* uid)
{
  char genbuf[200];

  if (!strcmp(uid, STR_GUEST))	/* alan.000330: guest¤£¶·­n¦³­Ó¤HºëµØ°Ï */
  {
    pressanykey("guest µL¦¹¥\\¯à¡I");
    return;
  }

  sethomefile(genbuf, uid, fn_plans);
  if (dashf(genbuf))
    more(genbuf,YEA);
  else
    pressanykey("%s ¥Ø«e¨S¦³¦W¤ù", uid);

  my_query(uid);

  switch (getans("[1]°ò¥»¸ê®Æ [2]Ã±¦WÀÉ [3]ºëµØ¤å³¹ [4]­Ó¤H§@«~ [q]Â÷¶}? "))
  {
    case '1':    
      my_query(uid);
      break;
    case '2':
      break;
    case '3':
      user_gem(uid);
      break;
    case '4':
      user_allpost(uid);
      break;
    default:
      break;
  }
  return;
}


int
u_editfile()
{
  int mode;
  char ans, buf[128], msg1[64], msg2[16];
  show_file("etc/userfile", 3, 11, ONLY_COLOR);
  ans = getans(" ­n¬Ý­þ­ÓÀÉ®× ? ");

  switch(ans)
  {
    case '0':
      clear();
      setuserfile(buf, fn_plans);
      more(buf, YEA);
      mode = EDITPLAN;
      strcpy(msg2, "¦W¤ùÀÉ");
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
      setuserfile(buf, "sig.0");
      buf[strlen(buf) - 1] = ans;
      move(3, 0);
      prints("      [1;33m¡´¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[42m   Ã±¦WÀÉ %c   [40m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¡´\033[m", ans);
//    clear();
      show_file(buf, 4, MAXSIGLINES, ONLY_COLOR);
      move(4 + MAXSIGLINES, 0);
      prints("      [1;33m¡´¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¡´\033[m", ans);
      mode = EDITSIG;
      strcpy(msg2, "Ã±¦WÀÉ");
      break;

    case 'c':
      setuserfile(buf,"cursor");
      clear();
      show_file(buf, 2, 2, STRIP_ALL);
      strcpy(msg2, "´å¼Ð");
      break;

    case 's':
      setuserfile(buf,"spam-list");
      clear();
      show_file(buf, 2, 2, STRIP_ALL);
      strcpy(msg2, "©Ú¦¬¶l¥ó¦W³æ");
      break;

    default:
      return;
  }
    
  sprintf(msg1, "%s ½s¿è(E) §R°£(D) ¨S¨Æ[Q] ", msg2);
  ans = getans(msg1);
  if (ans == 'e')
  {
    setutmpmode(mode);
    if (vedit(buf, NA) == -1)
      pressanykey("%s ¨S¦³§ó°Ê", msg2);
    else
      pressanykey("%s §ó·s§¹²¦", msg2);
  }
  else if (ans == 'd')
    unlink(buf);

  return 0;
}


#ifdef REG_FORM
/* --------------------------------------------- */
/* ¨Ï¥ÎªÌ¶ñ¼gµù¥Uªí®æ                            */
/* --------------------------------------------- */

static void
getfield(line, info, desc, buf, len)
  int line, len;
  char *info, *desc, *buf;
{
  char prompt[STRLEN];
  char genbuf[200];

  sprintf(genbuf, "­ì¥ý³]©w¡G%-30.30s (%s)", buf, info);
  move(line, 2);
  outs(genbuf);
  sprintf(prompt, "%s¡G", desc);
  if (getdata(line + 1, 2, prompt, genbuf, len, DOECHO,0))
    strcpy(buf, genbuf);
  move(line, 2);
  prints("%s¡G%s", desc, buf);
  clrtoeol();
}


int
u_register()
{
  char rname[20], addr[50] ,howto[50]="½Ð½T¹ê¶ñ¼g";
  char phone[20], career[40], email[50],birthday[9],sex_is[2],year,mon,day;
  char ans[3], *ptr;
  FILE *fn;
  time_t now;
  char genbuf[200];
  
  if (cuser.userlevel & PERM_LOGINOK)
  {
    pressanykey("±zªº¨­¥÷½T»{¤w¸g§¹¦¨¡A¤£»Ý¶ñ¼g¥Ó½Ðªí");
    return XEASY;
  }
  if (fn = fopen(fn_register, "r"))
  {
    while (fgets(genbuf, STRLEN, fn))
    {
      if (ptr = strchr(genbuf, '\n'))
        *ptr = '\0';
      if (strncmp(genbuf, "uid: ", 5) == 0 &&
        strcmp(genbuf + 5, cuser.userid) == 0)
      {
        fclose(fn);
        pressanykey("±zªºµù¥U¥Ó½Ð³æ©|¦b³B²z¤¤¡A½Ð­@¤ßµ¥­Ô");
        return XEASY;
      }
    }
    fclose(fn);
  }

  move(2, 0);
  clrtobot();
  strcpy(rname, cuser.realname);
  strcpy(addr, cuser.address);
  strcpy(email, cuser.email);
  sprintf(birthday, "%02i/%02i/%02i",
        cuser.year, cuser.month, cuser.day);
  sex_is[0]=(cuser.sex >= '0' && cuser.sex <= '7') ? cuser.sex+'1': '1';sex_is[1]=0;
  career[0] = phone[0] = '\0';
  while (1)
  {
    clear();
    move(3, 0);
    prints("%s[1;32m¡i[m%s[1;32m¡j[m ±z¦n¡A½Ð¾Ú¹ê¶ñ¼g¥H¤Uªº¸ê®Æ:(µLÅÜ§ó½Ð«öenter¸õ¹L)",
      cuser.userid, cuser.username);
    getfield(6, "½Ð½T¹ê¶ñ¼g¤¤¤å©m¦W", "¯u¹ê©m¦W", rname, 20);
    getfield(8, "¾Ç®Õ¨t¯Å©Î³æ¦ìÂ¾ºÙ", "ªA°È³æ¦ì", career, 40);
    getfield(10, "¥]¬A¹ì«Ç©ÎªùµP¸¹½X", "¥Ø«e¦í§}", addr, 50);
    getfield(12, "¥]¬Aªø³~¼·¸¹°Ï°ì½X", "³sµ¸¹q¸Ü", phone, 20);
    while (1)
    {
    int len;
    getfield(14, " 19xx/¤ë/¤é ¦p: 77/12/01","¥Í¤é",birthday,9);
    len = strlen(birthday);
    if(!len)
       {
         sprintf(birthday, "%02i/%02i/%02i",
         cuser.year, cuser.month, cuser.day);
         year=cuser.year;
         mon=cuser.month;
         day=cuser.day;
       }
    else if (len==8)
       {
        year  = (birthday[0] - '0') * 10 + (birthday[1] - '0');
        mon = (birthday[3] - '0') * 10 + (birthday[4] - '0');
        day   = (birthday[6] - '0') * 10 + (birthday[7] - '0');
       }
    else
        continue;
    if (mon > 12 || mon < 1 || day > 31 || day < 1 || year > 90 || year < 40)
        continue;
    break;
    }
    getfield(16,"1.¸¯®æ 2.©j±µ 3.©³­} 4.¬ü¬Ü","©Ê§O",sex_is,2);
    getfield(18, "¨­¤À»{ÃÒ¥Î", "E-Mail Address", email, 50);
    getfield(20, "±q­þÃäª¾¹D³o­Ó¯¸ªº", "±q¦ó±oª¾", howto, 50);

    getdata(b_lines - 1, 0, "¥H¤W¸ê®Æ¬O§_¥¿½T(Y/N)¡H(Q)¨ú®øµù¥U [N] ", ans, 3, LCECHO,0);
    if (ans[0] == 'q')
      return 0;
    if (ans[0] == 'y')
      break;
  }
  cuser.rtimes++;
  strcpy(cuser.realname, rname);
  strcpy(cuser.address, addr);
  strcpy(cuser.email, email);
  cuser.sex= sex_is[0]-'1';
  cuser.month=mon;cuser.day=day;cuser.year=year;
  if (fn = fopen(fn_register, "a"))
  {
    now = time(NULL);
    str_trim(career);
    str_trim(addr);
    str_trim(phone);
    fprintf(fn, "num: %d, %s", usernum, ctime(&now));
    fprintf(fn, "uid: %s\n", cuser.userid);
    fprintf(fn, "name: %s\n", rname);
    fprintf(fn, "howto: %s\n", howto);
    fprintf(fn, "career: %s\n", career);
    fprintf(fn, "addr: %s\n", addr);
    fprintf(fn, "phone: %s\n", phone);
    fprintf(fn, "email: %s\n", email);
    fprintf(fn, "----\n");
    fclose(fn);
  }
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* °O¿ý */
  return 0;
}
#endif

/* --------------------------------------------- */
/* ¦C¥X©Ò¦³µù¥U¨Ï¥ÎªÌ                            */
/* --------------------------------------------- */


extern struct UCACHE *uidshm;
int usercounter, totalusers, showrealname;
ushort u_list_special;

extern int
bad_user_id(char userid[]);

static int
u_list_CB(uentp)
  userec *uentp;
{
  static int i;
  char permstr[8], *ptr;
  register int level;

  if (uentp == NULL)
  {
    move(2, 0);
    clrtoeol();
    prints("[7m  ¨Ï¥ÎªÌ¥N¸¹   %-25s   ¤W¯¸  ¤å³¹  %s  ³Ìªñ¥úÁ{¤é´Á     [0m\n",
      showrealname ? "¯u¹ê©m¦W" : "ºï¸¹¼ÊºÙ"
      ,HAS_PERM(PERM_SEEULEVELS) ? "µ¥¯Å" : "");
    i = 3;
    return 0;
  }

  if (bad_user_id(uentp->userid))  /* Ptt */
    return 0;

  if (uentp->userlevel < u_list_special)
    return 0;

  if (i == b_lines)
  {
    prints(COLOR1"  ¤wÅã¥Ü %d/%d ¤H(%d%%)  "COLOR2"  (Space)[30m ¬Ý¤U¤@­¶  [32m(Q)[30m Â÷¶}  [0m",
      usercounter, totalusers, usercounter * 100 / totalusers);
    i = igetch();
    if (i == 'q' || i == 'Q')
      return QUIT;
    i = 3;
  }
  if (i == 3)
  {
    move(3, 0);
    clrtobot();
  }

  level = uentp->userlevel;
  strcpy(permstr, "----");
  if (level & PERM_SYSOP)
    permstr[0] = 'S';
  else if (level & PERM_ACCOUNTS)
    permstr[0] = 'A';
  else if (level & PERM_DENYPOST)
    permstr[0] = 'p';

  if (level & (PERM_BOARD))
    permstr[1] = 'B';
  else if (level & (PERM_BM))
    permstr[1] = 'b';

  if (level & (PERM_XEMPT))
    permstr[2] = 'X';
  else if (level & (PERM_LOGINOK))
    permstr[2] = 'R';

  if (level & (PERM_CLOAK | PERM_SEECLOAK))
    permstr[3] = 'C';

  ptr = (char *) Etime(&uentp->lastlogin);
  ptr[18] = '\0';
  prints("%-14s %-27.27s%5d %5d  %s  %s\n",
    uentp->userid, showrealname ? uentp->realname : uentp->username
    ,uentp->numlogins, uentp->numposts,
    HAS_PERM(PERM_SEEULEVELS) ? permstr : "", ptr);
  usercounter++;
  i++;
  return 0;
}


int
u_list()
{
  setutmpmode(LAUSERS);
  showrealname = u_list_special = usercounter = 0;
  totalusers = uidshm->number;
  if (HAS_PERM(PERM_SEEULEVELS))
  {
    if (getans("Æ[¬Ý [1]¯S®íµ¥¯Å (2)¥þ³¡¡H") != '2')
      u_list_special = 32;
  }
  if (HAS_PERM(PERM_CHATROOM) || HAS_PERM(PERM_SYSOP))
  {
    if (getans("Åã¥Ü [1]¯u¹ê©m¦W (2)¼ÊºÙ¡H") != '2')
      showrealname = 1;
  }
  u_list_CB(NULL);
  if (rec_apply(fn_passwd, u_list_CB, sizeof(userec)) == -1)
  {
    outs(msg_nobody);
    return XEASY;
  }
  move(b_lines, 0);
  clrtoeol();
  prints(COLOR1"  ¤wÅã¥Ü %d/%d ªº¨Ï¥ÎªÌ(¨t²Î®e¶qµL¤W­­)  "COLOR2"  (½Ð«ö¥ô·NÁäÄ~Äò)  [m",
    usercounter, totalusers);
  igetkey();
  return 0;
}


#ifdef POSTNOTIFY
int
m_postnotify()
{
  FILE *f1;
  int n=0, i;
  char ans[4];
  char genbuf[200],fname[200];
  char id[64][IDLEN+1];

  sethomefile(genbuf, cuser.userid, "postnotify"); 
  if ((f1 = fopen (genbuf, "r")) == NULL)
    return XEASY;

  stand_title ("¼f®Ö·s¤å³¹³qª¾");
  i = 2;

  while (fgets (genbuf, STRLEN, f1))
  {
    move (i++, 0);
    outs (genbuf);
    strcpy(id[n],genbuf);
    n++;
  }
  sethomefile(genbuf, cuser.userid, "postnotify"); 
  if(!isprint(id[0][0]))
  {
    unlink(genbuf);
    return 0;
  } 
  fclose(f1); 
  getdata (b_lines - 1, 0, "¶}©l¼f®Ö¶Ü(Y/N)¡H[Y] ", ans, 3, LCECHO, "Y");
  if (ans[0] == 'y')
  {
    sethomefile(fname,cuser.userid,"postnotify.ok"); 
    for(i = n-1; i>= 0; i--)
    {
      move(1,0);
      clrtobot();  
      if(!getuser(id[i]))
      {
        pressanykey("¬dµL¦¹¤H %s",id[i]);
        sethomefile(genbuf, cuser.userid, "postnotify");
        del_distinct(genbuf, id[i]);
        continue;
      }  
      move(2,0); 
      prints("[1;32m­^¤å¥N¸¹ : [37m%-13.13s   [32m¼ÊºÙ: [37m%s\n",xuser.userid,xuser.username);
      prints("[1;32m¤W¯¸¦¸¼Æ : [37m%-13d   [32m¤å³¹: [37m%d\n",xuser.numlogins,xuser.numposts);
      prints("[1;32m¹q¤l«H½c : [37m%s[m\n",xuser.email);
      getdata(10,0,"¬O§_­nÅý¥L¥[¤J? (y/n/Skip)¡H[S]",ans,3,LCECHO,0);
      if(ans[0] == 'y' || ans[0] == 'Y')
      {
        add_distinct(fname, xuser.userid);
        sethomefile(genbuf, cuser.userid, "postnotify"); 
        del_distinct(genbuf, xuser.userid); 
        mail2user(xuser,"[·s¤å³¹³qª¾] ¦P·N¥[¤J",BBSHOME"/etc/pn_agree"); 
      } 
      if(ans[0] == 'n' || ans[0] == 'N')
      {
        sethomefile(genbuf, cuser.userid, "postnotify"); 
        del_distinct(genbuf, xuser.userid); 
        mail2user(xuser,"[·s¤å³¹³qª¾] ©Úµ´¥[¤J",BBSHOME"/etc/pn_dissent"); 
//        sethomefile(genbuf, xuser.userid, "postlist"); 
//        del_distinct(genbuf, cuser.userid); 
      } 
    }
  }
  return 0;
}


int
re_m_postnotify()
{
  char genbuf[200], buf[200], buf2[200];

  sethomefile(buf, cuser.userid, "postnotify.ok");
  sethomefile(buf2, cuser.userid, "postnotify");
  if (!dashf(buf) && !dashf(buf2))
  {
    pressanykey("¥Ø«e¨S¦³¥ô¦ó¤H³]©w§Aªº·s¤å³¹³qª¾"); 
    return 0;
  } 

  if (dashf(buf2))
    m_postnotify();

  if (answer("¬O§_­n­«·s¼f®Ö? (y/N)") == 'y')
  {  
    if (dashf(buf))
    {
      sprintf(genbuf,"/bin/cat %s >> %s",buf,buf2); 
      system(genbuf); 
      unlink(buf);
      m_postnotify();
    } 
  }
}
#endif

#ifdef REG_MAGICKEY
/* shakalaca.000712: new justify */
int
u_verify()
{
  char keyfile[80], buf[80], inbuf[15], *key;
  FILE *fp;

  if (HAS_PERM(PERM_LOGINOK))
  {
    pressanykey("±z¤w¸g³q¹L»{ÃÒ, ¤£¶·­n¿é¤J MagicKey");
    return XEASY;
  }

  sethomefile(keyfile, cuser.userid, "MagicKey");
  if (!dashf(keyfile))
  {
    pressanykey("±z©|¥¼µo¥X»{ÃÒ«H.. -_-");
    return XEASY;
  }

  if (!(fp = fopen(keyfile, "r")))
  {
    pressanykey("¶}±ÒÀÉ®×¦³°ÝÃD, ½Ð³qª¾¯¸ªø");
    fclose(fp);
    return XEASY;
  }

  while (fgets(buf, 80, fp))
  {
    key = strtok(buf, "\n");
  }
  fclose(fp);

  getdata(b_lines, 0, "½Ð¿é¤J MagicKey¡G", inbuf, 14, DOECHO, 0);
  if (*inbuf)
  {
    if (strcmp(key, inbuf))
    {
      pressanykey("¿ù»~, ½Ð­«·s¿é¤J.");
    }
    else
    {
      int unum = getuser(cuser.userid);
      
      unlink(keyfile);
      pressanykey("®¥³ß±z³q¹L»{ÃÒ, Åwªï¥[¤J :)");
      cuser.userlevel |= (PERM_PAGE | PERM_POST | PERM_CHAT | PERM_LOGINOK);
      mail2user(cuser, "[µù¥U¦¨¥\\Åo]", BBSHOME"/etc/registered");
      substitute_record (fn_passwd, &cuser, sizeof (cuser), unum);
    }
  }

  return RC_FULL;
}
#endif
