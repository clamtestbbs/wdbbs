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
/* 檢查 user 註冊情況 */
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

#undef VACATION     // 是否為寒暑假保留帳號期間

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
  return 180 * 24 * 60 - value; /* 寒暑假保存帳號 180 天 */
#else
  if (!urec->numlogins)         /* 未 login 成功者，不保留 */
    return -1;
  else if (urec->numlogins <= 3)     /* #login 少於三者，保留 30 天 */
    return 30 * 24 * 60 - value;

  /* 未完成註冊者，保留 30 天 */
  /* 一般情況，保留 180 天 */
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
  /* Lazy method : 先找尋已經清除的過期帳號 */
  /* -------------------------------------- */

  if ((i = searchnewuser(0)) == 0)
  {

    /* ------------------------------- */
    /* 每 1 個小時，清理 user 帳號一次 */
    /* ------------------------------- */

    if ((stat(fn_fresh, &st) == -1) || (st.st_mtime < clock - 3600))
    {
      if ((fd = open(fn_fresh, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      write(fd, ctime(&clock), 25);
      close(fd);
      log_usies("CLEAN", "dated users");

      printf("尋找新帳號中, 請稍待片刻...\n\r");
      if ((fd = open(fn_passwd, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      i = 0;  /* Ptt解決第一個帳號老是被砍問題 */
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
// wildcat : 直接 mv , 不用跑 rm home/userid
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
      printf("抱歉，使用者帳號已經滿了，無法註冊新的帳號\n\r");
    val = (st.st_mtime - clock + 3660) / 60;
    printf("請等待 %d 分鐘後再試一次，祝你好運\n\r", val);
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

      pressanykey("您嘗試錯誤的輸入太多，請下次再來吧");
      oflush();
      exit(1);
    }
    getdata(16, 0, msg_uid, newuser.userid, IDLEN + 1, DOECHO,0);

    if (bad_user_id(newuser.userid))
      outs("無法接受這個代號，請使用英文字母，並且不要包含空格\n");
    else if (searchuser(newuser.userid))
      outs("此代號已經有人使用\n");
    else
      break;
  }

  try = 0;
  while (1)
  {
    if (++try >= 6)
    {
      pressanykey("您嘗試錯誤的輸入太多，請下次再來吧");
      oflush();
      exit(1);
    }
    if ((getdata(17, 0, "請設定密碼：", passbuf, PASSLEN, PASS,0) < 4) ||
      !strcmp(passbuf, newuser.userid))
    {
      pressanykey("密碼太簡單，易遭入侵，至少要 4 個字，請重新輸入");
      continue;
    }
    strncpy(newuser.passwd, passbuf, PASSLEN);
    getdata(18, 0, "請檢查密碼：", passbuf, PASSLEN, PASS,0);
    if (strncmp(passbuf, newuser.passwd, PASSLEN))
    {
      outs("密碼輸入錯誤, 請重新輸入密碼.\n");
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
    fprintf(stderr, "本站人口已達飽和！\n");
    exit(1);
  }
  

  if (substitute_record(fn_passwd, &newuser, sizeof(userec), allocid) == -1)
  {
    fprintf(stderr, "客滿了，再見！\n");
    exit(1);
  }

  setuserid(allocid, newuser.userid);
  if (!dosearchuser(newuser.userid))
  {
    fprintf(stderr, "無法建立帳號\n");
    exit(1);
  }
}

/* origin: SOB & Ptt              */
/* modify: wildcat/980909         */
/* 確認user是否通過註冊、資料正確 */
check_register()
{
  char *ptr;
  char genbuf[200],buf[100];

  if(!HAS_PERM(PERM_POST) && (cuser.lastlogin - cuser.firstlogin >= 86400))
    cuser.userlevel |= PERM_POST;  

  stand_title("請詳細填寫個人資料");

  while (strlen(cuser.username) < 2)
    getdata(2, 0, "綽號暱稱：", cuser.username, 24, DOECHO,0);
  strcpy(currutmp->username, cuser.username);

  while (strlen(cuser.feeling) < 2)
    getdata(3, 0, "心情狀態：", cuser.feeling, 5, DOECHO,0);
  cuser.feeling[4] = '\0';
  strcpy(currutmp->feeling, cuser.feeling);

  for (ptr = cuser.username; *ptr; ptr++)
  {
    if (*ptr == 9)              /* TAB convert */
      strcpy(ptr, " ");
  }
  while (strlen(cuser.realname) < 4)
    getdata(4, 0, "真實姓名：", cuser.realname, 20, DOECHO,0);

  while (!cuser.month || !cuser.day || !cuser.year)
  {
      sprintf(genbuf, "%02i/%02i/%02i",
      cuser.year,cuser.month, cuser.day);
      getdata(6, 0, "出生年份 西元 19", buf, 3, DOECHO,0);
      cuser.year = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(7, 0, "出生月份", buf, 3, DOECHO,0);
      cuser.month = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(8, 0, "出生日期", buf, 3, DOECHO,0);
      cuser.day = (buf[0] - '0') * 10 + (buf[1] - '0');
      if (cuser.month > 12 || cuser.month < 1 ||
        cuser.day > 31 || cuser.day < 1 || cuser.year > 90 || cuser.year < 40)
        continue;
      break;
    }

  while (strlen(cuser.address) < 8)
    getdata(9, 0, "聯絡地址：", cuser.address, 50, DOECHO,0);

  while (cuser.sex > 7)
  {
    char buf[10];
    getdata(10, 0, 
      "性別 (1)葛格 (2)姐接 (3)底迪 (4)美眉 (5)薯叔 (6)阿姨 (7)植物 (8)礦物",
      buf , 3, DOECHO, 0);
    if (buf[0] >= '1' && buf[0] <= '8')
      cuser.sex = buf[0] - '1';
  }

  if (belong_spam(BBSHOME"/etc/spam-list",cuser.email))
  {
    strcpy(cuser.email,"NULL");
    pressanykey("抱歉,本站不接受你的 E-Mail 信箱位置");
  }
  
  if (!strchr(cuser.email, '@'))
  {
    bell();
    move(t_lines - 4, 0);
    prints("\
※ 為了您的權益，請填寫真實的 E-mail address， 以資確認閣下身份，\n\
   格式為 \x1b[44muser@domain_name\x1b[0m 或 \x1b[44muser@\\[ip_number\\]\x1b[0m。\n\n\
※ 如果您真的沒有 E-mail，請直接按 [return] 即可。");

    do
    {
      getdata(12, 0, "電子信箱：", cuser.email, 50, DOECHO,0);
      if (!cuser.email[0])
        sprintf(cuser.email, "%s%s", cuser.userid, str_mail_address);
      if(belong_spam(BBSHOME"/etc/spam-list",cuser.email))
      {
        strcpy(cuser.email, "NULL");
        pressanykey("抱歉,本站不接受你的 E-Mail 信箱位置");
      }
    } while (!strchr(cuser.email, '@'));

#ifdef  REG_MAGICKEY   
    mail_justify(cuser);
#endif

  }

  cuser.userlevel |= PERM_DEFAULT;
  if (!HAS_PERM(PERM_SYSOP) && !(cuser.userlevel & PERM_LOGINOK))
  {
    /* 回覆過身份認證信函，或曾經 E-mail post 過 */

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
      && answer("是否要填寫註冊單 (y/N)") == 'y')
    u_register();
#endif
    
    u_habit();
  }
  if (HAS_PERM(PERM_DENYPOST) && !HAS_PERM(PERM_SYSOP))
    cuser.userlevel &= ~PERM_POST;
}
