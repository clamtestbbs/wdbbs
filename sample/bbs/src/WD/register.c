/*-------------------------------------------------------*/
/* register.c   ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : user register routines                       */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"


/* ------------------- */
/* password encryption */
/* ------------------- */

char *crypt();
static char pwbuf[14];

int
chkpasswd(passwd, test)
  char *passwd, *test;
{
  char *pw;
  strncpy(pwbuf, test, 14);
  pw = crypt(pwbuf, passwd);
  return (!strncmp(pw, passwd, 14));
}

/* ------------------ */
/* ÀË¬d user µù¥U±¡ªp */
/* ------------------ */


int
bad_user_id(userid)
  char *userid;
{
  register char ch;

  if (belong("etc/baduser",userid))
    return 1;

  if (strlen(userid) < 2)
    return 1;

  if (not_alpha(*userid))
    return 1;

  if (!ci_strcmp(userid, str_new))
    return 1;

  while (ch = *(++userid))
  {
    if (not_alnum(ch))
      return 1;
  }
  return 0;
}


/* -------------------------------- */
/* New policy for allocate new user */
/* (a) is the worst user currently  */
/* (b) is the object to be compared */
/* -------------------------------- */

#undef VACATION     // ¬O§_¬°´H´»°²«O¯d±b¸¹´Á¶¡

static int
compute_user_value(urec, clock)
  userec *urec;
  time_t clock;
{
  int value;

  /* if (urec) has XEMPT permission, don't kick it */
  if ((urec->userid[0] == '\0') || (urec->userlevel & PERM_XEMPT))
    return 9999;

  value = (clock - urec->lastlogin) / 60;       /* minutes */

  /* new user should register in 60 mins */
  if (strcmp(urec->userid, str_new) == 0)
    return (60 - value);

#ifdef  VACATION
  return 180 * 24 * 60 - value; /* ´H´»°²«O¦s±b¸¹ 180 ¤Ñ */
#else
  if (!urec->numlogins)         /* ¥¼ login ¦¨¥\ªÌ¡A¤£«O¯d */
    return -1;
  else if (urec->numlogins <= 3)     /* #login ¤Ö©ó¤TªÌ¡A«O¯d 30 ¤Ñ */
    return 30 * 24 * 60 - value;

  /* ¥¼§¹¦¨µù¥UªÌ¡A«O¯d 30 ¤Ñ */
  /* ¤@¯ë±¡ªp¡A«O¯d 180 ¤Ñ */
  else
    return (urec->userlevel & PERM_LOGINOK ? 180 : 30) * 24 * 60 - value;
#endif
}


static int
getnewuserid()
{
  static char *fn_fresh = ".fresh";
  extern struct UCACHE *uidshm;
  userec utmp, zerorec;
  time_t clock;
  struct stat st;
  int fd, val, i;
  char genbuf[200];
  char genbuf2[200];

  memset(&zerorec, 0, sizeof(zerorec));
  clock = time(NULL);

  /* -------------------------------------- */
  /* Lazy method : ¥ý§ä´M¤w¸g²M°£ªº¹L´Á±b¸¹ */
  /* -------------------------------------- */

  if ((i = searchnewuser(0)) == 0)
  {

    /* ------------------------------- */
    /* ¨C 1 ­Ó¤p®É¡A²M²z user ±b¸¹¤@¦¸ */
    /* ------------------------------- */

    if ((stat(fn_fresh, &st) == -1) || (st.st_mtime < clock - 3600))
    {
      if ((fd = open(fn_fresh, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      write(fd, ctime(&clock), 25);
      close(fd);
      log_usies("CLEAN", "dated users");

      printf("´M§ä·s±b¸¹¤¤, ½Ðµy«Ý¤ù¨è...\n\r");
      if ((fd = open(fn_passwd, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      i = 0;  /* Ptt¸Ñ¨M²Ä¤@­Ó±b¸¹¦Ñ¬O³Q¬å°ÝÃD */
      while (i < MAXUSERS)
      {
        i++;
        if (read(fd, &utmp, sizeof(userec)) != sizeof(userec))
          break;
	if(i==1) continue;
/*
woju
*/
        if ((val = compute_user_value(&utmp, clock)) < 0) {
           sprintf(genbuf, "#%d %-12s %15.15s %d %d %d",
             i, utmp.userid, ctime(&(utmp.lastlogin)) + 4,
             utmp.numlogins, utmp.numposts, val);
           if (val > -1 * 60 * 24 * 365)
           {
             log_usies("CLEAN", genbuf);
             sprintf(genbuf, "home/%s", utmp.userid);
             sprintf(genbuf2, "tmp/%s", utmp.userid);
// wildcat : ª½±µ mv , ¤£¥Î¶] rm home/userid
             if (dashd(genbuf))
               f_mv(genbuf, genbuf2);
             lseek(fd, (off_t)((i - 1) * sizeof(userec)), SEEK_SET);
             write(fd, &zerorec, sizeof(utmp));
           }
           else
              log_usies("DATED", genbuf);
        }
      }
      close(fd);
      time(&(uidshm->touchtime));
    }
  }
  if ((fd = open(fn_passwd, O_RDWR | O_CREAT, 0600)) == -1)
    return -1;
  flock(fd, LOCK_EX);

  i = searchnewuser(1);
  if ((i <= 0) || (i > MAXUSERS))
  {
    flock(fd, LOCK_UN);
    close(fd);
    if (more("etc/user_full", NA) == -1)
      printf("©êºp¡A¨Ï¥ÎªÌ±b¸¹¤w¸gº¡¤F¡AµLªkµù¥U·sªº±b¸¹\n\r");
    val = (st.st_mtime - clock + 3660) / 60;
    printf("½Ðµ¥«Ý %d ¤ÀÄÁ«á¦A¸Õ¤@¦¸¡A¯¬§A¦n¹B\n\r", val);
    sleep(2);
    exit(1);
  }

  sprintf(genbuf, "uid %d", i);
  log_usies("APPLY", genbuf);

  strcpy(zerorec.userid, str_new);
  zerorec.lastlogin = clock;
  if (lseek(fd, (off_t)(sizeof(zerorec) * (i - 1)), SEEK_SET) == -1)
  {
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }
  write(fd, &zerorec, sizeof(zerorec));
  setuserid(i, zerorec.userid);
  flock(fd, LOCK_UN);
  close(fd);
  return i;
}

#ifdef REG_FORM
int u_register();
#endif

void
new_register()
{
  userec newuser;
  char passbuf[STRLEN];
  int allocid, try;

  memset(&newuser, 0, sizeof(newuser));

  more("etc/register", NA);
  try = 0;
  while (1)
  {
    if (++try >= 6)
    {
      refresh();

      pressanykey("±z¹Á¸Õ¿ù»~ªº¿é¤J¤Ó¦h¡A½Ð¤U¦¸¦A¨Ó§a");
      oflush();
      exit(1);
    }
    getdata(16, 0, msg_uid, newuser.userid, IDLEN + 1, DOECHO,0);

    if (bad_user_id(newuser.userid))
      outs("µLªk±µ¨ü³o­Ó¥N¸¹¡A½Ð¨Ï¥Î­^¤å¦r¥À¡A¨Ã¥B¤£­n¥]§tªÅ®æ\n");
    else if (searchuser(newuser.userid))
      outs("¦¹¥N¸¹¤w¸g¦³¤H¨Ï¥Î\n");
    else
      break;
  }

  try = 0;
  while (1)
  {
    if (++try >= 6)
    {
      pressanykey("±z¹Á¸Õ¿ù»~ªº¿é¤J¤Ó¦h¡A½Ð¤U¦¸¦A¨Ó§a");
      oflush();
      exit(1);
    }
    if ((getdata(17, 0, "½Ð³]©w±K½X¡G", passbuf, PASSLEN, PASS,0) < 4) ||
      !strcmp(passbuf, newuser.userid))
    {
      pressanykey("±K½X¤ÓÂ²³æ¡A©ö¾D¤J«I¡A¦Ü¤Ö­n 4 ­Ó¦r¡A½Ð­«·s¿é¤J");
      continue;
    }
    strncpy(newuser.passwd, passbuf, PASSLEN);
    getdata(18, 0, "½ÐÀË¬d±K½X¡G", passbuf, PASSLEN, PASS,0);
    if (strncmp(passbuf, newuser.passwd, PASSLEN))
    {
      outs("±K½X¿é¤J¿ù»~, ½Ð­«·s¿é¤J±K½X.\n");
      continue;
    }
    passbuf[8] = '\0';
    strncpy(newuser.passwd, genpasswd(passbuf), PASSLEN);
    break;
  }
  newuser.userlevel = PERM_DEFAULT;
  newuser.pager = 1;
  newuser.uflag = COLOR_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
  newuser.firstlogin = newuser.lastlogin = time(NULL);
  srandom(time(0));
  newuser.habit = HABIT_NEWUSER;	/* user.habit */
  allocid = getnewuserid();
  if (allocid > MAXUSERS || allocid <= 0)
  {
    fprintf(stderr, "¥»¯¸¤H¤f¤w¹F¹¡©M¡I\n");
    exit(1);
  }
  

  if (substitute_record(fn_passwd, &newuser, sizeof(userec), allocid) == -1)
  {
    fprintf(stderr, "«Èº¡¤F¡A¦A¨£¡I\n");
    exit(1);
  }

  setuserid(allocid, newuser.userid);
  if (!dosearchuser(newuser.userid))
  {
    fprintf(stderr, "µLªk«Ø¥ß±b¸¹\n");
    exit(1);
  }
}

/* origin: SOB & Ptt              */
/* modify: wildcat/980909         */
/* ½T»{user¬O§_³q¹Lµù¥U¡B¸ê®Æ¥¿½T */
check_register()
{
  char *ptr;
  char genbuf[200],buf[100];

  if(!HAS_PERM(PERM_POST) && (cuser.lastlogin - cuser.firstlogin >= 86400))
    cuser.userlevel |= PERM_POST;  

  stand_title("½Ð¸Ô²Ó¶ñ¼g­Ó¤H¸ê®Æ");

  while (strlen(cuser.username) < 2)
    getdata(2, 0, "ºï¸¹¼ÊºÙ¡G", cuser.username, 24, DOECHO,0);
  strcpy(currutmp->username, cuser.username);

  while (strlen(cuser.feeling) < 2)
    getdata(3, 0, "¤ß±¡ª¬ºA¡G", cuser.feeling, 5, DOECHO,0);
  cuser.feeling[4] = '\0';
  strcpy(currutmp->feeling, cuser.feeling);

  for (ptr = cuser.username; *ptr; ptr++)
  {
    if (*ptr == 9)              /* TAB convert */
      strcpy(ptr, " ");
  }
  while (strlen(cuser.realname) < 4)
    getdata(4, 0, "¯u¹ê©m¦W¡G", cuser.realname, 20, DOECHO,0);

  while (!cuser.month || !cuser.day || !cuser.year)
  {
      sprintf(genbuf, "%02i/%02i/%02i",
      cuser.year,cuser.month, cuser.day);
      getdata(6, 0, "¥X¥Í¦~¥÷ ¦è¤¸ 19", buf, 3, DOECHO,0);
      cuser.year = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(7, 0, "¥X¥Í¤ë¥÷", buf, 3, DOECHO,0);
      cuser.month = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(8, 0, "¥X¥Í¤é´Á", buf, 3, DOECHO,0);
      cuser.day = (buf[0] - '0') * 10 + (buf[1] - '0');
      if (cuser.month > 12 || cuser.month < 1 ||
        cuser.day > 31 || cuser.day < 1 || cuser.year > 90 || cuser.year < 40)
        continue;
      break;
    }

  while (strlen(cuser.address) < 8)
    getdata(9, 0, "Ápµ¸¦a§}¡G", cuser.address, 50, DOECHO,0);

  while (cuser.sex > 7)
  {
    char buf[10];
    getdata(10, 0, 
      "©Ê§O (1)¸¯®æ (2)©j±µ (3)©³­} (4)¬ü¬Ü (5)Á¦¨û (6)ªü«¼ (7)´Óª« (8)Äqª«",
      buf , 3, DOECHO, 0);
    if (buf[0] >= '1' && buf[0] <= '8')
      cuser.sex = buf[0] - '1';
  }

  if (belong_spam(BBSHOME"/etc/spam-list",cuser.email))
  {
    strcpy(cuser.email,"NULL");
    pressanykey("©êºp,¥»¯¸¤£±µ¨ü§Aªº E-Mail «H½c¦ì¸m");
  }
  
  if (!strchr(cuser.email, '@'))
  {
    bell();
    move(t_lines - 4, 0);
    prints("\
¡° ¬°¤F±zªºÅv¯q¡A½Ð¶ñ¼g¯u¹êªº E-mail address¡A ¥H¸ê½T»{»Õ¤U¨­¥÷¡A\n\
   ®æ¦¡¬° [44muser@domain_name[0m ©Î [44muser@\\[ip_number\\][0m¡C\n\n\
¡° ¦pªG±z¯uªº¨S¦³ E-mail¡A½Ðª½±µ«ö [return] §Y¥i¡C");

    do
    {
      getdata(12, 0, "¹q¤l«H½c¡G", cuser.email, 50, DOECHO,0);
      if (!cuser.email[0])
        sprintf(cuser.email, "%s%s", cuser.userid, str_mail_address);
      if(belong_spam(BBSHOME"/etc/spam-list",cuser.email))
      {
        strcpy(cuser.email, "NULL");
        pressanykey("©êºp,¥»¯¸¤£±µ¨ü§Aªº E-Mail «H½c¦ì¸m");
      }
    } while (!strchr(cuser.email, '@'));

#ifdef  REG_MAGICKEY   
    mail_justify(cuser);
#endif

  }

  cuser.userlevel |= PERM_DEFAULT;
  if (!HAS_PERM(PERM_SYSOP) && !(cuser.userlevel & PERM_LOGINOK))
  {
    /* ¦^ÂÐ¹L¨­¥÷»{ÃÒ«H¨ç¡A©Î´¿¸g E-mail post ¹L */

    setuserfile(genbuf, "email");
    if (dashf(genbuf))
      cuser.userlevel |= ( PERM_POST );

#ifdef  STRICT
    else
    {
      cuser.userlevel &= ~PERM_POST;
      more("etc/justify", YEA);
    }
#endif

  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
/* wildcat 981218 */
    clear();
    update_data(); 

#ifdef REG_FORM
    if (HAS_PERM(PERM_POST) && !HAS_PERM(PERM_LOGINOK)
      && answer("¬O§_­n¶ñ¼gµù¥U³æ (y/N)") == 'y')
    u_register();
#endif
    
    u_habit();
  }
  if (HAS_PERM(PERM_DENYPOST) && !HAS_PERM(PERM_SYSOP))
    cuser.userlevel &= ~PERM_POST;
}
