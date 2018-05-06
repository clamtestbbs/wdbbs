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
  outs("\x1b[1;33m●────────────\x1b[42m   使用者資料   \x1b[40m────────────●\n");
  prints(""
"  \x1b[32m英文代號：\x1b[37m%-16.16s\x1b[32m暱稱：\x1b[37m%-20.20s\x1b[32m性別：\x1b[37m%-8.8s\n"
"  \x1b[32m真實姓名：\x1b[37m%-16.16s\x1b[32m住址：\x1b[37m%-40s\n"
"  \x1b[32m出生日期：\x1b[37m19%02i年%02i月%02i日  \x1b[32mＥMail：\x1b[37m%-40s\n",
    u->userid,u->username,sex[u->sex],
    u->realname, u->address,
    u->year, u->month, u->day, u->email); 
  prints("  \x1b[32m上站次數：\x1b[37m%-16d\x1b[32m註冊日期：\x1b[37m%s"
    ,u->numlogins,ctime(&u->firstlogin)); 
  prints("  \x1b[32m文章數目：\x1b[37m%-16d\x1b[32m前次上站：\x1b[37m%s"
    ,u->numposts,ctime(&u->lastlogin)); 
  prints("  \x1b[32m私人信箱：\x1b[37m%-4d 封         \x1b[32m利息發放：\x1b[37m%s"
    ,rec_num(genbuf, sizeof(fileheader)),ctime(&u->dtime));
  prints(
"  \x1b[32m信箱上限：\x1b[37m%d 封\n"
"  \x1b[32m人氣指數：\x1b[37m%-16ld\x1b[32m好奇指數：\x1b[37m%-16ld\x1b[32m心情：\x1b[37m%-4.4s\n"
"  \x1b[32m發訊息數：\x1b[37m%-16d\x1b[32m收訊息數：\x1b[37m%d\n"
"  \x1b[32m上站地點：\x1b[37m%s \n"
"  \x1b[32m傳呼號碼：\x1b[37m0%d-%s \n"
,(u->exmailbox+MAXKEEPMAIL)
,u->bequery,u->toquery,u->feeling,u->sendmsg,u->receivemsg
,u->lasthost,u->pagermode,u->pagernum);

  if (real)
  {
    strcpy(genbuf, "bTCPRp#@XWBA#VSA?crFG??????????");
    for (diff = 0; diff < 31; diff++)
      if (!(u->userlevel & (1 << diff)))
        genbuf[diff] = '-';
    prints("  \x1b[32m認證資料：\x1b[37m%-50.50s\n",u->justify);
    prints("  \x1b[32m使用權限：\x1b[37m%-32s\n",genbuf);
  }
  diff = u->totaltime / 60;
  day = diff / 1440;
  hour = (diff/60)%24;
  min = diff%60;
  prints("  \x1b[32m上站時間：\x1b[37m%d天 %d小時 %d分\n",day,hour,min);

  if (u->userlevel >= PERM_BM)
  {
    int i;
    boardheader *bhdr;
    resolve_boards();

    outs("  \x1b[32m擔任板主：\x1b[37m");

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
  prints("\x1b[33m●────────────────────────────────●\x1b[m\n");

  if (!real)
    outs((u->userlevel & PERM_LOGINOK) ?
      "您的註冊程序已經完成，歡迎加入本站" :
      "如果要提昇權限，請參考本站公佈欄辦理註冊");

#ifdef  NEWUSER_LIMIT
  if (!real)
    if ((u->lastlogin - u->firstlogin < 3 * 86400) && !HAS_PERM(PERM_POST))
      outs("新手上路，三天後開放權限");
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
    "(1)修改資料 (2)設定密碼 (3)設定權限 (4)清除帳號 (5)改ID (6)送封號 [0]結束 " :
    "請選擇 (1)修改資料 (2)設定密碼 ==> [0]結束 ");

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
    outs("請逐項修改。");
    getdata(i++, 0," 暱 稱  ：",x.username, 24, DOECHO,x.username);
    strip_ansi(x.username,x.username,STRIP_ALL);
    getdata(i++, 0,"居住地址：",x.address, 50, DOECHO,x.address);
    strip_ansi(x.address,x.address,STRIP_ALL);
    getdata(i++, 0,"現在心情：",x.feeling, 5, DOECHO,x.feeling);
    x.feeling[4] = '\0';
    getdata(i++, 0,"電子信箱：",buf, 50, DOECHO,x.email);
    if(belong_spam(BBSHOME"/etc/spam-list",x.email))
    {
      pressanykey("抱歉,本站不接受你的 E-Mail 信箱位置");
      strcpy(buf,x.email);
    }
    if (strcmp(buf,x.email) && !not_addr(buf))
    {
      strcpy(x.email,buf);
      mail_changed = 1 - real;
    }
    strip_ansi(x.email,x.email,STRIP_ALL);

    sprintf(buf, "%04d", u->pagermode);
    getdata(i, 0,"呼叫器門號：",buf, 5, DOECHO, buf );
    if(!strncmp(buf,"094",3) && buf[3] != '2' && buf[3] != '4')
      x.pagermode = atol(buf);
    else
      x.pagermode = u->pagermode;      

    getdata(i++, 30,"呼叫器號碼：",x.pagernum, 7, DOECHO,x.pagernum);

    sprintf(genbuf,
"性別 (1)葛格 (2)姐接 (3)底迪 (4)美眉 (5)薯叔 (6)阿姨 (7)植物 (8)礦物 [%i]:",
      u->sex + 1);
    getdata(i++, 0, genbuf,
      buf, 3, DOECHO, 0);
    if (buf[0] >= '1' && buf[0] <= '8')
      x.sex = buf[0] - '1';

    while (1)
    {
      sprintf(genbuf,"%02i/%02i/%02i",u->year,u->month,u->day);
      getdata(i, 0, "出生年份 19", buf, 3, DOECHO, genbuf);
      x.year  = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(i, 0, "出生月份   ", buf, 3, DOECHO, genbuf+3);
      x.month = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(i, 0, "出生日期   ", buf, 3, DOECHO, genbuf+6);
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

// wildcat:不要讓 user 註冊完亂改名字? 
      getdata(i++, 0,"真實姓名：",x.realname, 20, DOECHO,x.realname);
      strip_ansi(x.realname,x.realname,STRIP_ALL);
      sprintf(genbuf, "%d", x.numlogins);
      if (getdata(i, 0,"上線次數：", buf, 10, DOECHO,genbuf))
        if ((l = atoi(buf)) >= 0)
          x.numlogins = (int) l;
      sprintf(genbuf,"%d", u->numposts);
      if (getdata(i++, 25, "文章數目：", buf, 10, DOECHO,genbuf))
        if ((l = atoi(buf)) >= 0)
          x.numposts = l;
      sprintf(genbuf, "%ld", x.sendmsg);
      if (getdata(i, 0,"發水球數：", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.sendmsg = l;
      sprintf(genbuf, "%ld", x.receivemsg);
      if (getdata(i++, 25,"收水球數：", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.receivemsg = l;
      sprintf(genbuf, "%ld", x.bequery);
      if (getdata(i, 0,"人氣度：", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.bequery = l;
      sprintf(genbuf, "%ld", x.toquery);
      if (getdata(i++, 25,"好奇度：", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.toquery = l;
      sprintf(genbuf, "%ld", x.totaltime);
      if (getdata(i++, 0,"上站時數：", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.totaltime = l;
      sprintf(genbuf, "%d", x.exmailbox);
      if (getdata(i++, 0,"購買信箱數：", buf, 4, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.exmailbox = (int) l;

      getdata(i++, 0,"認證資料：", x.justify, 39, DOECHO, x.justify);
      getdata(i++, 0,"最近光臨機器：", x.lasthost, 24, DOECHO,x.lasthost);


      fail = 0;
    }
    break;

  case '2':
    i = 19;
    if (!real)
    {
      if (!getdata(i++, 0, "請輸入原密碼：", buf, PASSLEN, PASS,0) ||
        !chkpasswd(u->passwd, buf))
      {
        outs("\n\n您輸入的密碼不正確\n");
        fail++;
        break;
      }
    }

    if (!getdata(i++, 0, "請設定新密碼：", buf, PASSLEN, PASS,0))
    {
      outs("\n\n密碼設定取消, 繼續使用舊密碼\n");
      fail++;
      break;
    }
    strncpy(genbuf, buf, PASSLEN);

    getdata(i++, 0, "請檢查新密碼：", buf, PASSLEN, PASS,0);
    if (strncmp(buf, genbuf, PASSLEN))
    {
      outs("\n\n新密碼確認失敗, 無法設定新密碼\n");
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
    if (getdata(b_lines - 3, 0, "新的使用者代號：", genbuf, IDLEN + 1,
        DOECHO,x.userid))
    {
      if (searchuser(genbuf))
      {
        outs("錯誤! 已經有同樣 ID 的使用者");
        fail++;
      }
      else
      {
        strcpy(x.userid, genbuf);
      }
    }
    break;
  case '6':
    if (getdata(b_lines - 3, 0, "你要給他什麼封號：", genbuf, 18,
        DOECHO,x.title))
    {
      time_t now=time(0);
      strcpy(x.title, genbuf);
      sprintf(genbuf,"%s 修改 %s 的封號為 [%s] %s",cuser.userid,x.userid,
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
// wildcat : 簡單多了 , 不要用 rm home/userid
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
  stand_title("使用者喜好設定");
  move(2, 0);
  outs("請依照使用習慣自行調整(●為開,   為關)：\n");
  move(4, 0);
  for (i = 0; i < NUMHABITS; i++)
  {
    prints("%c. %-28s %-7s\n"
       , 'A' + i, habitstrings[i],((fbits >> i) & 1 ? "●" : "  "));
  }
  clrtobot();
  while (getdata(b_lines - 1, 0, "請按 [A-N] 切換設定，按 [Return] 結束：",
      choice, 3, LCECHO, 0))
  {
    i = choice[0] - 'a';
    if (i < 0 || i >= NUMHABITS)
      bell();
    else
    {
      fbits ^= (1 << i);
      move( i%14 + 4, i <= 14 ? 32 : 72);
      prints((fbits >> i) & 1 ? "●" : "  ");
    }
    /* for HABIT_COLOR, 專為了彩色模式設的, i==1是表示第二個HABIT,
       當HABIT_COLOR不在第二個HABIT時, 這邊也要相應調整.
                                                - add by wisely - */
    if( i == 1) showansi ^= 1;
  }
  update_data(); /* SiE: sync data prev bug user */
  cuser.habit = fbits;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* 記錄 */
  return RC_FULL;
}


void
showplans(char* uid)
{
  char genbuf[200];

  if (!strcmp(uid, STR_GUEST))	/* alan.000330: guest不須要有個人精華區 */
  {
    pressanykey("guest 無此功\能！");
    return;
  }

  sethomefile(genbuf, uid, fn_plans);
  if (dashf(genbuf))
    more(genbuf,YEA);
  else
    pressanykey("%s 目前沒有名片", uid);

  my_query(uid);

  switch (getans("[1]基本資料 [2]簽名檔 [3]精華文章 [4]個人作品 [q]離開? "))
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
  ans = getans(" 要看哪個檔案 ? ");

  switch(ans)
  {
    case '0':
      clear();
      setuserfile(buf, fn_plans);
      more(buf, YEA);
      mode = EDITPLAN;
      strcpy(msg2, "名片檔");
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
      prints("      \x1b[1;33m●────────────\x1b[42m   簽名檔 %c   \x1b[40m────────────●\033[m", ans);
//    clear();
      show_file(buf, 4, MAXSIGLINES, ONLY_COLOR);
      move(4 + MAXSIGLINES, 0);
      prints("      \x1b[1;33m●───────────────────────────────●\033[m", ans);
      mode = EDITSIG;
      strcpy(msg2, "簽名檔");
      break;

    case 'c':
      setuserfile(buf,"cursor");
      clear();
      show_file(buf, 2, 2, STRIP_ALL);
      strcpy(msg2, "游標");
      break;

    case 's':
      setuserfile(buf,"spam-list");
      clear();
      show_file(buf, 2, 2, STRIP_ALL);
      strcpy(msg2, "拒收郵件名單");
      break;

    default:
      return 0;
  }
    
  sprintf(msg1, "%s 編輯(E) 刪除(D) 沒事[Q] ", msg2);
  ans = getans(msg1);
  if (ans == 'e')
  {
    setutmpmode(mode);
    if (vedit(buf, NA) == -1)
      pressanykey("%s 沒有更動", msg2);
    else
      pressanykey("%s 更新完畢", msg2);
  }
  else if (ans == 'd')
    unlink(buf);

  return 0;
}


#ifdef REG_FORM
/* --------------------------------------------- */
/* 使用者填寫註冊表格                            */
/* --------------------------------------------- */

static void
getfield(line, info, desc, buf, len)
  int line, len;
  char *info, *desc, *buf;
{
  char prompt[STRLEN];
  char genbuf[200];

  sprintf(genbuf, "原先設定：%-30.30s (%s)", buf, info);
  move(line, 2);
  outs(genbuf);
  sprintf(prompt, "%s：", desc);
  if (getdata(line + 1, 2, prompt, genbuf, len, DOECHO,0))
    strcpy(buf, genbuf);
  move(line, 2);
  prints("%s：%s", desc, buf);
  clrtoeol();
}


int
u_register()
{
  char rname[20], addr[50] ,howto[50]="請確實填寫";
  char phone[20], career[40], email[50],birthday[9],sex_is[2],year,mon,day;
  char ans[3], *ptr;
  FILE *fn;
  time_t now;
  char genbuf[200];
  
  if (cuser.userlevel & PERM_LOGINOK)
  {
    pressanykey("您的身份確認已經完成，不需填寫申請表");
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
        pressanykey("您的註冊申請單尚在處理中，請耐心等候");
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
    prints("%s\x1b[1;32m【\x1b[m%s\x1b[1;32m】\x1b[m 您好，請據實填寫以下的資料:(無變更請按enter跳過)",
      cuser.userid, cuser.username);
    getfield(6, "請確實填寫中文姓名", "真實姓名", rname, 20);
    getfield(8, "學校系級或單位職稱", "服務單位", career, 40);
    getfield(10, "包括寢室或門牌號碼", "目前住址", addr, 50);
    getfield(12, "包括長途撥號區域碼", "連絡電話", phone, 20);
    while (1)
    {
    int len;
    getfield(14, " 19xx/月/日 如: 77/12/01","生日",birthday,9);
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
    getfield(16,"1.葛格 2.姐接 3.底迪 4.美眉","性別",sex_is,2);
    getfield(18, "身分認證用", "E-Mail Address", email, 50);
    getfield(20, "從哪邊知道這個站的", "從何得知", howto, 50);

    getdata(b_lines - 1, 0, "以上資料是否正確(Y/N)？(Q)取消註冊 [N] ", ans, 3, LCECHO,0);
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
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* 記錄 */
  return 0;
}
#endif

/* --------------------------------------------- */
/* 列出所有註冊使用者                            */
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
    prints("\x1b[7m  使用者代號   %-25s   上站  文章  %s  最近光臨日期     \x1b[0m\n",
      showrealname ? "真實姓名" : "綽號暱稱"
      ,HAS_PERM(PERM_SEEULEVELS) ? "等級" : "");
    i = 3;
    return 0;
  }

  if (bad_user_id(uentp->userid))  /* Ptt */
    return 0;

  if (uentp->userlevel < u_list_special)
    return 0;

  if (i == b_lines)
  {
    prints(COLOR1"  已顯示 %d/%d 人(%d%%)  "COLOR2"  (Space)\x1b[30m 看下一頁  \x1b[32m(Q)\x1b[30m 離開  \x1b[0m",
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
    if (getans("觀看 [1]特殊等級 (2)全部？") != '2')
      u_list_special = 32;
  }
  if (HAS_PERM(PERM_CHATROOM) || HAS_PERM(PERM_SYSOP))
  {
    if (getans("顯示 [1]真實姓名 (2)暱稱？") != '2')
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
  prints(COLOR1"  已顯示 %d/%d 的使用者(系統容量無上限)  "COLOR2"  (請按任意鍵繼續)  \x1b[m",
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

  stand_title ("審核新文章通知");
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
  getdata (b_lines - 1, 0, "開始審核嗎(Y/N)？[Y] ", ans, 3, LCECHO, "Y");
  if (ans[0] == 'y')
  {
    sethomefile(fname,cuser.userid,"postnotify.ok"); 
    for(i = n-1; i>= 0; i--)
    {
      move(1,0);
      clrtobot();  
      if(!getuser(id[i]))
      {
        pressanykey("查無此人 %s",id[i]);
        sethomefile(genbuf, cuser.userid, "postnotify");
        del_distinct(genbuf, id[i]);
        continue;
      }  
      move(2,0); 
      prints("\x1b[1;32m英文代號 : \x1b[37m%-13.13s   \x1b[32m暱稱: \x1b[37m%s\n",xuser.userid,xuser.username);
      prints("\x1b[1;32m上站次數 : \x1b[37m%-13d   \x1b[32m文章: \x1b[37m%d\n",xuser.numlogins,xuser.numposts);
      prints("\x1b[1;32m電子信箱 : \x1b[37m%s\x1b[m\n",xuser.email);
      getdata(10,0,"是否要讓他加入? (y/n/Skip)？[S]",ans,3,LCECHO,0);
      if(ans[0] == 'y' || ans[0] == 'Y')
      {
        add_distinct(fname, xuser.userid);
        sethomefile(genbuf, cuser.userid, "postnotify"); 
        del_distinct(genbuf, xuser.userid); 
        mail2user(xuser,"[新文章通知] 同意加入",BBSHOME"/etc/pn_agree"); 
      } 
      if(ans[0] == 'n' || ans[0] == 'N')
      {
        sethomefile(genbuf, cuser.userid, "postnotify"); 
        del_distinct(genbuf, xuser.userid); 
        mail2user(xuser,"[新文章通知] 拒絕加入",BBSHOME"/etc/pn_dissent"); 
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
    pressanykey("目前沒有任何人設定你的新文章通知"); 
    return 0;
  } 

  if (dashf(buf2))
    m_postnotify();

  if (answer("是否要重新審核? (y/N)") == 'y')
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
    pressanykey("您已經通過認證, 不須要輸入 MagicKey");
    return XEASY;
  }

  sethomefile(keyfile, cuser.userid, "MagicKey");
  if (!dashf(keyfile))
  {
    pressanykey("您尚未發出認證信.. -_-");
    return XEASY;
  }

  if (!(fp = fopen(keyfile, "r")))
  {
    pressanykey("開啟檔案有問題, 請通知站長");
    fclose(fp);
    return XEASY;
  }

  while (fgets(buf, 80, fp))
  {
    key = strtok(buf, "\n");
  }
  fclose(fp);

  getdata(b_lines, 0, "請輸入 MagicKey：", inbuf, 14, DOECHO, 0);
  if (*inbuf)
  {
    if (strcmp(key, inbuf))
    {
      pressanykey("錯誤, 請重新輸入.");
    }
    else
    {
      int unum = getuser(cuser.userid);
      
      unlink(keyfile);
      pressanykey("恭喜您通過認證, 歡迎加入 :)");
      cuser.userlevel |= (PERM_PAGE | PERM_POST | PERM_CHAT | PERM_LOGINOK);
      mail2user(cuser, "[註冊成功\囉]", BBSHOME"/etc/registered");
      substitute_record (fn_passwd, &cuser, sizeof (cuser), unum);
    }
  }

  return RC_FULL;
}
#endif
