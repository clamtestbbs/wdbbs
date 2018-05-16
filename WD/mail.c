/*-------------------------------------------------------*/
/* mail.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : local/internet mail routines                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

#define MAXPATHLEN	1024
extern int cmpfmode();

char currmaildir[32];
/* static */ char msg_cc[] = "\x1b[32m[群組名單]\x1b[0m\n";
static char listfile[] = "list.0";
static int mailkeep=0, mailsum=0;
static int mailsumlimit=0,mailmaxkeep=0;
extern int TagNum;


int
setforward() /* Ptt , modify by wildcat*/
{
  char buf[80],ip[50]="",yn[4];
// wildcat : bbs 所有 domain name list
  char myhost[6][50] = {"wd.twbbs.org","wdbbs.net","wdbbs.org","wd.twbbs.org.tw","bbs.wdbbs.net","bbs.wdbbs.org"}; 
  FILE *fp;
  int i=0,allow=1;
   
  sethomepath(buf, cuser.userid);
  strcat(buf,"/.forward");
  if(fp = fopen(buf,"r"))
  {
    fscanf(fp,"%s",ip);
    fclose(fp);
  }
  getdata(b_lines-1,0,"請輸入信箱自動轉寄的email地址:",
    ip,41, DOECHO,ip);
// 判斷是否轉寄給自己,或是寄到同一台機器
  do
  {
    str_lower(myhost[i],myhost[i]);
    str_lower(ip,ip);
    if(strstr(ip,myhost[i])) allow=0;
  }while(!strlen(myhost[i++])); 
// 直接用 not_addr 來判斷是不是 email 即可
  if(allow && !not_addr(ip))
  {
    getdata(b_lines,0,"確定開啟自動轉信功\能?(Y/n)",yn,3,LCECHO,0);
    if(yn[0] != 'n' &&  (fp=fopen(buf,"w")))
    {
      fprintf(fp,"%s",ip);
      fclose(fp);
      pressanykey("設定完成!");
      refresh();
      return 0;
    }
  }
  unlink(buf);
  pressanykey("取消自動轉信!");
  refresh();
  return 0;
}

#ifdef INTERNET_PRIVATE_EMAIL
int
m_internet()
{
  char receiver[60];

  getdata(20, 0, "收信人：", receiver, 60, DOECHO,0);
  if (strchr(receiver, '@') && !not_addr(receiver) &&
    getdata(21, 0, "主  題：", save_title, TTLEN, DOECHO,0))
  {
    do_send(receiver, save_title);
  }
  else
  {
    move(22, 0);
    pressanykey("收信人或主題不正確, 請重新選取指令");
  }
  return 0;
}

void
mail_forward(fhdr, direct, mode)
  fileheader *fhdr;
  char *direct;
  int mode;
{
  char buf[STRLEN];
  char *p;

  strncpy(buf, direct, sizeof(buf));
  if (p = strrchr(buf, '/'))
    *p = '\0';
  switch (doforward(buf, fhdr, mode))
  {
  case 0:
    outz(msg_fwd_ok);
    break;
  case -1:
    outz(msg_fwd_err1);
    break;
  case -2:
    outz(msg_fwd_err2);
  }
}
#endif

int
chkmailbox(void)
{
  if (!HAVE_PERM(PERM_SYSOP) && !HAVE_PERM(PERM_MAILLIMIT))
  {
    if (HAS_PERM(PERM_BM))
       mailsumlimit = 300;
    else if (HAS_PERM(PERM_LOGINOK))
       mailsumlimit = 150;
    else
       mailsumlimit = 100;
    mailsumlimit += cuser.exmailbox;
    mailmaxkeep = MAXKEEPMAIL + cuser.exmailbox;
    sethomedir(currmaildir, cuser.userid);
    if ((mailkeep = rec_num(currmaildir, sizeof(fileheader))) > mailmaxkeep)
    {
      move(b_lines, 0);
      clrtoeol();
      bell();
      prints("您保存信件數目 %d 超出上限 %d, 請整理", mailkeep, mailmaxkeep);
      bell();
      refresh();
      igetch();
      return mailkeep;
    }
    if ((mailsum = get_sum_records(currmaildir, sizeof(fileheader))) >
                mailsumlimit)
    {
      move(b_lines, 0);
      clrtoeol();
      bell();
      prints("您保存信件容量 %d(k)超出上限 %d(k), 請整理", mailsum, mailsumlimit);
      bell();
      refresh();
      igetch();
      return mailkeep;
    }
  }
  return 0;
}


static void
do_hold_mail(fpath, receiver, holder)
  char *fpath;
  char *receiver;
  char *holder;
{
  char buf[80], title[128];

  fileheader mymail;

  sethomepath(buf, holder);
  stampfile(buf, &mymail);

  mymail.savemode = 'H';        /* hold-mail flag */
  mymail.filemode = FILE_READ;
  strcpy(mymail.owner, "[備.忘.錄]");
  if (receiver)
  {
    sprintf(title, "(%s) %s", receiver, save_title);
    strncpy(mymail.title, title, TTLEN);
  }
  else
    strcpy(mymail.title, save_title);

  sethomedir(title, holder);
  if (rec_add(title, &mymail, sizeof(mymail)) != -1)
  {
    unlink(buf);
    f_cp(fpath, buf, O_TRUNC);
  }
}


void
hold_mail(fpath, receiver)
  char *fpath;
  char *receiver;
{
  char buf[4];

  getdata(b_lines - 1, 0, "已順利寄出，是否自存底稿(Y/N)？[N] ", buf, 4, LCECHO,0);

  if (buf[0] == 'y')
    do_hold_mail(fpath, receiver, cuser.userid);
/*
  if (is_watched(cuser.userid))
    do_hold_mail(fpath, receiver, "SYSOP");
*/
}


int
do_send(userid, title)
  char *userid, *title;
{
  fileheader mhdr;
  char fpath[STRLEN];
  char receiver[IDLEN+1];
  char genbuf[200];

#ifdef INTERNET_PRIVATE_EMAIL
  int internet_mail;

  if (strchr(userid, '@'))
  {
    internet_mail = 1;
  }
  else
  {
    internet_mail = 0;
#endif

    if (!getuser(userid))
      return -1;
    if (!(xuser.userlevel & PERM_READMAIL))
      return -3;

    if (!title)
      getdata(2, 0, "主題：", save_title, TTLEN, DOECHO,0);
    curredit |= EDIT_MAIL;
    curredit &= ~EDIT_ITEM;
#ifdef INTERNET_PRIVATE_EMAIL
  }
#endif

  setutmpmode(SMAIL);

  fpath[0] = '\0';

#ifdef INTERNET_PRIVATE_EMAIL
  if (internet_mail)
  {
    int res, ch;

    if (vedit(fpath, NA) == -1)
    {
      unlink(fpath);
      clear();
      return -2;
    }
    clear();
    prints("信件即將寄給 %s\n標題為：%s\n確定要寄出嗎? (Y/N) [Y]",
      userid, title);
    ch = igetch();
    switch (ch)
    {
    case 'N':
    case 'n':
      outs("N\n信件已取消");
      res = -2;
      break;

    default:
      outs("Y\n請稍候, 信件傳遞中...\n");
      res = bbs_sendmail(fpath, title, userid, NULL);
      hold_mail(fpath, userid);
    }
    unlink(fpath);
    return res;
  }
  else
  {
#endif
   strcpy(receiver, userid);
    if (vedit(fpath, YEA) == -1)
    {
      unlink(fpath);
      clear();
      return -2;
    }
    clear();
   strcpy(userid, receiver);
    sethomepath(genbuf, userid);
    stampfile(genbuf, &mhdr);
    f_mv(fpath, genbuf);
    strcpy(mhdr.owner, cuser.userid);
    strncpy(mhdr.title, save_title, TTLEN);
    mhdr.savemode = '\0';
    sethomedir(fpath, userid);
    if (rec_add(fpath, &mhdr, sizeof(mhdr)) == -1)
      return -1;

    hold_mail(genbuf, userid);
    return 0;

#ifdef INTERNET_PRIVATE_EMAIL
  }
#endif
}


void
my_send(uident)
  char *uident;
{
  switch (do_send(uident, NULL))
  {
  case -1:
    outs(err_uid);
    break;
  case -2:
    outs(msg_cancel);
    break;
  case -3:
    prints("使用者 [%s] 無法收信", uident);
    break;
  }
  pressanykey(NULL);
}


int
m_send()
{
  char uident[40];

  stand_title("風帶來的消息");
  usercomplete(msg_uid, uident);
  if (uident[0])
    my_send(uident);
  return 0;
}


/* ------------------------------------------------------------ */
/* 群組寄信、回信 : multi_send, multi_reply                      */
/* ------------------------------------------------------------ */

extern struct word *toplev;

static void
multi_list(reciper)
  int *reciper;
{
  char uid[16];
  char genbuf[200];

  while (1)
  {
    stand_title("群組寄信名單");
    ShowNameList(3, 0, msg_cc);
    sprintf(genbuf, 
"(I)引入好友 (O)引入上線通知 (N)引入新文章通知 (0-9)引入其他特別名單\n"
"(A)增加     (D)刪除         (M)確認寄信名單  %s(Q)取消 ？[M]",
    HAS_PERM(PERM_SYSOP) ? " (B)所有板主 " : "");
    getdata(1, 0, genbuf, genbuf, 4, LCECHO,0);
    switch (genbuf[0])
    {
    case 'a':
      while (1)
      {
        move(2, 0);
        usercomplete("請輸入要增加的代號(只按 ENTER 結束新增): ", uid);
        if (uid[0] == '\0')
          break;

        move(3, 0);

        if (!searchuser(uid))
          outs(err_uid);
        else if (!InNameList(uid))
        {
          AddNameList(uid);
          (*reciper)++;
        }
        ShowNameList(3, 0, msg_cc);
      }
      break;

    case 'd':
      while (*reciper)
      {
        move(1, 0);
        namecomplete("請輸入要刪除的代號(只按 ENTER 結束刪除): ", uid);
        if (uid[0] == '\0')
          break;
        if (RemoveNameList(uid))
        {
          (*reciper)--;
        }
        ShowNameList(3, 0, msg_cc);
      }
      break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      listfile[5] = genbuf[0];
      genbuf[0] = '1';

    case 'i':
      setuserfile(genbuf, genbuf[0] == '1' ? listfile : fn_overrides);
      ToggleNameList(reciper, genbuf, msg_cc);
      break;

    case 'o':
      setuserfile(genbuf, "alohaed");
      ToggleNameList(reciper, genbuf, msg_cc);
      break;

    case 'n':
      setuserfile(genbuf, "postlist");
      ToggleNameList(reciper, genbuf, msg_cc);
      break;

    case 'm':
      *reciper = 0;
      return;
    break;

    case 'b':
      if (HAS_PERM(PERM_SYSOP)) {
         make_bmlist();
         *reciper = CountNameList();
         return;
      }
      break;

    default:
      return;
    }
  }
}



/* static */
/* shakalaca.000117: marked for list.c */
int
multi_send(title, inmail)
  char *title;
  int inmail;
{
  FILE *fp;
  struct word *p;
  fileheader mymail;
  char fpath[TTLEN], *ptr;
  int reciper, listing;
  char genbuf[256];

  if (inmail)
  {
    CreateNameList();
    listing = reciper = 0;

    if (*quote_file)
    {
      AddNameList(quote_user);
      reciper = 1;
      fp = fopen(quote_file, "r");
      while (fgets(genbuf, 256, fp))
      {
        if (strncmp(genbuf, "※ ", 3))
        {
          if (listing)
            break;
        }  
        else
        {
          if (listing)
          {
            strtok(ptr = genbuf + 3, " \n\r");
            do
            {
              if (searchuser(ptr) && !InNameList(ptr) && strcmp(cuser.userid, ptr))
              {
                AddNameList(ptr);
                reciper++;
              }
            } while (ptr = (char *) strtok(NULL, " \n\r"));
          }
          else if (!strncmp(genbuf + 3, "[通告]", 6))
            listing = 1;
        }
      }
      ShowNameList(3, 0, msg_cc);
    }  

    multi_list(&reciper);
  }
  else
  {
    reciper = CountNameList();
  }
  
  move(1, 0);
  clrtobot();

  if (reciper)
  {
    setutmpmode(SMAIL);

    if (title)
    {
      do_reply_title(2, title);
    }
    else
    { 
      getdata(2, 0, "主題：", fpath, 64, DOECHO,0);
      sprintf(save_title, "[通告] %s", fpath);
    } 

    getdata(4, 0, "是否要在信件中顯示收件者名單 ? (Y/N) [Y] ", genbuf, 3, DOECHO, 0);
    setuserfile(fpath, fn_notes);

    if (fp = fopen(fpath, "w"))
    {
      fprintf(fp, "※ [通告] 共 %d 人收件", reciper);
      listing = 80;

      if (genbuf[0] != 'n')
      {
        for (p = toplev; p; p = p->next)
        {
          reciper = strlen(p->word) + 1;
          if (listing + reciper > 75)
          {
            listing = reciper;
            fprintf(fp, "\n※");
          }
          else
            listing += reciper;
  
          fprintf(fp, " %s", p->word);
        }
      }
      memset(genbuf, '-', 75);
      genbuf[75] = '\0';
      fprintf(fp, "\n%s\n\n", genbuf);
      fclose(fp);
    }

    curredit |= EDIT_LIST;

    if (vedit(fpath, YEA) == -1)
    {
      unlink(fpath);
      curredit = 0;
      pressanykey(msg_cancel);
      return 0;
    }

    stand_title("寄信中...");
    refresh();

    listing = 80;

    for (p = toplev; p; p = p->next)
    {
      reciper = strlen(p->word) + 1;
      if (listing + reciper > 75)
      {
        listing = reciper;
        outc('\n');
      }
      else
      {
        listing += reciper;
        outc(' ');
      }
      outs(p->word);
      if (searchuser(p->word) && strcmp(STR_GUEST, p->word) )
        sethomepath(genbuf, p->word);
      else
        continue;
      stampfile(genbuf, &mymail);
      unlink(genbuf);
      f_cp(fpath, genbuf, O_TRUNC);

      strcpy(mymail.owner, cuser.userid);
      strcpy(mymail.title, save_title);
      mymail.savemode = 'M';    /* multi-send flag */
      sethomedir(genbuf, p->word);
      if (rec_add(genbuf, &mymail, sizeof(mymail)) == -1)
        outs(err_uid);
    }
    hold_mail(fpath, NULL);
    unlink(fpath);
    curredit = 0;
  }
  else
  {
    outs(msg_cancel);
  }
  pressanykey(NULL);
}


static int
multi_reply(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  if (fhdr->savemode != 'M')
    return mail_reply(ent, fhdr, direct);

  stand_title("群組回信");
  strcpy(quote_user, fhdr->owner);
  setuserfile(quote_file, fhdr->filename);
  multi_send(fhdr->title, 1);
  return 0;
}


int
mail_list()
{
  stand_title("群組作業");
  multi_send(NULL, 1);
  return 0;
}

extern int
bad_user_id(char userid[]);

int
mail_all()
{
   FILE *fp;
   fileheader mymail;
   char fpath[TTLEN];
   char genbuf[200];
   extern struct UCACHE *uidshm;
   int i, unum;
   char* userid;

   stand_title("給所有使用者的系統通告");
   setutmpmode(SMAIL);
/*
   if(answer("是否要給錢？ (y/N)") == 'y')
   {
     while(money <= 0)
     {
       getdata(2, 0, "要給多少金幣?",fpath, 4, DOECHO, 0);
       money = atoi(fpath);
       if(money <=0) return RC_FULL;
       mode = 1;
     }
   }
*/
   getdata(2, 0, "主題：", fpath, 64, DOECHO,0);
   sprintf(save_title, "[系統通告]\x1b[1;32m %s\x1b[m", fpath);

   setuserfile(fpath, fn_notes);

   if (fp = fopen(fpath, "w")) {
      fprintf(fp, "※ [\x1b[1m系統通告\x1b[m] 這是封給所有使用者的信\n");
      fprintf(fp, "---------------------------------------------------------------------------\n");
      fclose(fp);
    }

   *quote_file = 0;

   curredit |= EDIT_MAIL;
   curredit &= ~EDIT_ITEM;
   if (vedit(fpath, YEA) == -1) {
      curredit = 0;
      unlink(fpath);
      pressanykey(msg_cancel);
      return 0;
   }
   curredit = 0;

   setutmpmode(MAILALL);
   stand_title("寄信中...");

   sethomepath(genbuf, cuser.userid);
   stampfile(genbuf, &mymail);
   unlink(genbuf);
   f_cp(fpath, genbuf, O_TRUNC);
   unlink(fpath);
   strcpy(fpath, genbuf);

   strcpy(mymail.owner, cuser.userid);  /*站長 ID*/
   strcpy(mymail.title, save_title);
   mymail.savemode = 0;

   sethomedir(genbuf, cuser.userid);
   if (rec_add(genbuf, &mymail, sizeof(mymail)) == -1)
      outs(err_uid);

   for (unum = uidshm->number, i = 0; i < unum; i++) {

      if(bad_user_id(uidshm->userid[i])) continue; /* Ptt */

      userid = uidshm->userid[i];
      if (strcmp(userid, "guest") && strcmp(userid, "new") && strcmp(userid, cuser.userid)) {
         sethomepath(genbuf, userid);
         stampfile(genbuf, &mymail);
         unlink(genbuf);
         f_cp(fpath, genbuf, O_TRUNC);

         strcpy(mymail.owner, cuser.userid);
         strcpy(mymail.title, save_title);
         mymail.savemode = 0;
         sethomedir(genbuf, userid);
         if (rec_add(genbuf, &mymail, sizeof(mymail)) == -1)
            outs(err_uid);
         sprintf(genbuf, "%*s %5d / %5d", IDLEN + 1, userid, i + 1, unum);
/*
         if(mode)
           inugold(userid,money);
*/
         outmsg(genbuf);
         refresh();
      }
   }
   return 0;
}


mail_mbox()
{
   char cmd[100];
   fileheader fhdr;

   sprintf(cmd, "/tmp/%s.tgz", cuser.userid);
   sprintf(fhdr.title, "%s 私人資料", cuser.userid);
   doforward(cmd, &fhdr, 'Z');
}

static int
m_forward(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char uid[STRLEN];

  stand_title("轉達信件");
  usercomplete(msg_uid, uid);
  if (uid[0] == '\0')
  {
    return RC_FULL;
  }

  strcpy(quote_user, fhdr->owner);
  setuserfile(quote_file, fhdr->filename);
  sprintf(save_title, "%.64s (fwd)", fhdr->title);
  move(1, 0);
  clrtobot();
  prints("轉信給: %s\n標  題: %s\n", uid, save_title);

  switch (do_send(uid, save_title))
  {
  case -1:
    outs(err_uid);
    break;
  case -2:
    outs(msg_cancel);
    break;
  case -3:
    prints("使用者 [%s] 無法收信", uid);
    break;
  }
  pressanykey(NULL);
  return RC_FULL;
}


/* JhLin: At most 128 mail */

int delmsgs[128];
int delcnt;
int mrd;


static int
read_new_mail(fptr)
  fileheader *fptr;
{
  static int idc;
  char done = NA, delete_it;
  char fname[256];
  char genbuf[4];

  if (fptr == NULL)
  {
    delcnt = 0;
    idc = 0;
    return 0;
  }
  idc++;
  if (fptr->filemode)
    return 0;
  clear();
  move(10, 0);
  prints("您要讀來自[%s]的訊息(%s)嗎？", fptr->owner, fptr->title);
  getdata(11, 0, "請您確定(Y/N/Q)?[Y] ", genbuf, 3, DOECHO,0);
  if (genbuf[0] == 'q')
    return QUIT;
  if (genbuf[0] == 'n')
    return 0;

  setuserfile(fname, fptr->filename);
  fptr->filemode |= FILE_READ;
  if (substitute_record(currmaildir, fptr, sizeof(*fptr), idc))
    return -1;

  mrd = 1;
  delete_it = NA;
  while (!done)
  {
    int more_result = more(fname, YEA);
    switch (more_result) {
    case 1:
       return RS_PREV;
    case 2:
       return RELATE_PREV;
    case 3:
       return RS_NEXT;
    case 4:
       return RELATE_NEXT;
    case 5:
       return RELATE_FIRST;
    case 6:
       return 0;
    case 7:
      mail_reply(idc, fptr, currmaildir);
      return RC_FULL;
    case 8:
      multi_reply(idc, fptr, currmaildir);
      return RC_FULL;
    }
    move(b_lines, 0);
    clrtoeol();
    outs(msg_mailer);
    refresh();

    switch (igetkey())
    {
    case 'r':
    case 'R':
      mail_reply(idc, fptr, currmaildir);
      break;
    case 'x':
      m_forward(idc, fptr, currmaildir);
      break;
    case 'y':
      multi_reply(idc, fptr, currmaildir);
      break;
    case 'd':
    case 'D':
      delete_it = YEA;
    default:
      done = YEA;
    }
  }
  if (delete_it)
  {
    clear();
    prints("刪除信件《%s》", fptr->title);
    getdata(1, 0, msg_sure_ny, genbuf, 2, LCECHO,0);
    if (genbuf[0] == 'y')
    {
      unlink(fname);
      delmsgs[delcnt++] = idc;
    }
  }
  clear();
  return 0;
}


int
m_new()
{
  clear();
  mrd = 0;
  setutmpmode(RMAIL);
  read_new_mail(NULL);
  clear();
  curredit |= EDIT_MAIL;
  curredit &= ~EDIT_ITEM;
  if (rec_apply(currmaildir, read_new_mail, sizeof(fileheader)) == -1)
  {
    pressanykey("沒有新信件了");
    return -1;
  }
  curredit = 0;
  if (delcnt)
  {
    while (delcnt--)
      rec_del(currmaildir, sizeof(fileheader), delmsgs[delcnt]);
  }
  outs(mrd ? "信已閱\畢" : "沒有新信件了");
  pressanykey(NULL);
  return -1;
}


static void
mailtitle()
{
  char buf[100]="";

  sprintf(tmpbuf,"%s [線上 %d 人]",BOARDNAME,count_ulist());
  showtitle("\0郵件選單", tmpbuf);
  outs("\
[←]離開 [↑,↓]選擇 [→,r]閱\讀信件 [R]回信 [x]轉達 [y]群組回信 [^Z]求助\n\x1b[1m\
"COLOR1" 編號   日 期  作 者          信  件  標  題 ");
  if(mailsumlimit)
  {
    sprintf(buf,"\x1b[32m(容量:%d/%dk %d/%d篇)",mailsum, mailsumlimit
                    ,mailkeep,mailmaxkeep);
  }
  sprintf(buf,"%s%*s\x1b[m",buf,34-strlen(buf),"");
  outs(buf);
}

static int
mail_del(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char genbuf[200];

  if (fhdr->filemode & FILE_MARKED)
    return RC_NONE;

  getdata(1, 0, msg_del_ny, genbuf, 3, LCECHO,0);
  if (genbuf[0] == 'y')
  {
    extern int cmpfilename();

    strcpy(currfile, fhdr->filename);
    if (!delete_file(direct, sizeof(*fhdr), ent, cmpfilename))
    {
      setdirpath(genbuf, direct, fhdr->filename);
      unlink(genbuf);
      if( currmode & MODE_SELECT ){
         int now;
         sethomedir(genbuf,cuser.userid);
         now=getindex(genbuf,fhdr->filename,sizeof(fileheader));
         delete_file (genbuf, sizeof(fileheader),now,cmpfilename);
      }
      return RC_CHDIR;
    }
  }
  return RC_FULL;
}


/*
woju
static int
*/
mail_read(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char buf[64];
  char done, delete_it, replied;

  clear();
  setdirpath(buf, direct, fhdr->filename);
  strncpy(currtitle, str_ttl(fhdr->title), 40);
  done = delete_it = replied = NA;
  while (!done)
  {
/*
woju
*/
    int more_result =  more(buf, YEA);
    if (more_result != -1) {
       fhdr->filemode |= FILE_READ;
       if (!strncmp("[新]", fhdr->title, 4) && !(fhdr->filemode & FILE_MARKED))
          fhdr->filemode |= FILE_TAGED;
     if ( currmode & MODE_SELECT )
     {
        int now;
        now = getindex(currmaildir, fhdr->filename, sizeof(*fhdr));
        substitute_record(currmaildir, fhdr, sizeof(*fhdr), now);
     }
     else
       substitute_record(currmaildir, fhdr, sizeof(*fhdr), ent);
    }
    switch (more_result) {
    case 1:
       return RS_PREV;
    case 2:
       return RELATE_PREV;
    case 3:
       return RS_NEXT;
    case 4:
       return RELATE_NEXT;
    case 5:
       return RELATE_FIRST;
    case 6:
       return RC_FULL;
    case 7:
      mail_reply(ent, fhdr, direct);
      return RC_FULL;
    case 8:
      multi_reply(ent, fhdr, direct);
      return RC_FULL;
    }
    move(b_lines, 0);
    clrtoeol();
    refresh();
    outs(msg_mailer);
  }
}

/* ---------------------------------------------- */
/* in boards/mail 回信給原作者，轉信站亦可        */
/* ---------------------------------------------- */

int
mail_reply(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char uid[STRLEN];
  char *t;
  FILE *fp;
  char genbuf[512];
  int enttmp=ent;

  stand_title("回  信");

  /* 判斷是 boards 或 mail */

  if (curredit & EDIT_MAIL)
    setuserfile(quote_file, fhdr->filename);
  else
    setbfile(quote_file, currboard, fhdr->filename);

  /* find the author */

  strcpy(quote_user, fhdr->owner);

  if (strchr(quote_user, '.'))
  {
    genbuf[0] = '\0';
    if (fp = fopen(quote_file, "r"))
    {
      fgets(genbuf, 512, fp);
      fclose(fp);
    }

    t = strtok(genbuf, str_space);
    if (!strcmp(t, str_author1) || !strcmp(t, str_author2))
    {
      strcpy(uid, strtok(NULL, str_space));
    }
    else
    {
      pressanykey("錯誤: 找不到作者。");
      return RC_FULL;
    }
  }
  else
    strcpy(uid, quote_user);

  /* make the title */

  do_reply_title(3, fhdr->title);
  prints("\n收信人: %s\n標  題: %s\n", uid, save_title);

  /* edit, then send the mail */

  ent = curredit;
  switch (do_send(uid, save_title))
  {
  case -1:
    outs(err_uid);
    break;
  case -2:
    outs(msg_cancel);
    break;
  case -3:
    prints("使用者 [%s] 無法收信", uid);
    break;
  default:
    fhdr->filemode |= FILE_REPLYOK;
    substitute_record(currmaildir, fhdr, sizeof(*fhdr), enttmp);
    break;
  }
  curredit = ent;
  pressanykey(NULL);
  return RC_FULL;
}


mail_save(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char fpath[256], title[TTLEN + 1];
  fileheader tmp;

  memcpy(&tmp, fhdr, sizeof(fileheader));

  if (HAS_PERM(PERM_MAILLIMIT))
  {
    setuserfile(fpath, fhdr->filename);
    sprintf(title, "◇ %s", tmp.title);
    strncpy(tmp.title, title, TTLEN);
    tmp.title[TTLEN] = '\0';
    gem_copyitem(fpath, &tmp);
    sethomeman(fpath, cuser.userid);
    gem_menu(fpath, 1, RMAIL);
    return RC_FULL;
  }
  return RC_NONE;
}

extern int cross_post();
extern int man();
extern int cite_article();
extern int edit_post();
extern int mark();
extern int del_range();
extern int edit_title();
/*
extern int del_tag();
*/

static struct one_key mail_comms[] = {
  'z', man,
  'c', cite_article,
  'D', del_range,
  'E', edit_post,
  'm', mark,
  'T', edit_title,
  'x', cross_post,
/*
  Ctrl('D'), del_tag,
 */
  'r', mail_read,
  'R', mail_reply,
  's', mail_save,
  'd', mail_del,
  'X', m_forward,
  'y', multi_reply,
  '\0', NULL
};


int
m_read()
{
  if (rec_num(currmaildir, sizeof(fileheader)))
  {
    curredit = EDIT_MAIL;
    TagNum = 0;
    i_read(RMAIL, currmaildir, mailtitle, doent, mail_comms,NULL);
    currfmode = FILE_TAGED;
    if (search_rec(currmaildir, cmpfmode))
      del_tag(0, 0, currmaildir);
    curredit = 0;
  }
  else
    pressanykey("您沒有來信");

  return 0;
}


#ifdef INTERNET_EMAIL
#include <pwd.h>

int
bbs_sendmail(fpath, title, receiver, key)
  char *fpath, *title, *receiver, *key;
{
  static int configured = 0;
  static char myhostname[STRLEN];
  static char myusername[20];
  struct hostent *hbuf;
  struct passwd *pbuf;
  char *ptr, genbuf[200];

  FILE *fin, *fout;

  if (ptr = strchr(receiver, ';'))
    *ptr = '\0';

  if ((ptr = strstr(receiver, str_mail_address)) || !strchr(receiver, '@'))
  {
    fileheader mymail;
    char hacker[20];
    int len;

    if (strchr(receiver, '@'))
    {
      len = ptr - receiver;
      memcpy(hacker, receiver, len);
      hacker[len] = '\0';
    } 
    else
      strcpy(hacker, receiver);

    if (!searchuser(hacker))
      return -2;

    sethomepath(genbuf, hacker);
    stampfile(genbuf, &mymail);
    if (!strcmp(hacker, cuser.userid))
    {
      strcpy(mymail.owner, BOARDNAME);
      mymail.filemode = FILE_READ;
    } else
      strcpy(mymail.owner, cuser.userid);
    strncpy(mymail.title, title, TTLEN);
    f_rm(genbuf);
    f_cp(fpath, genbuf, O_TRUNC);
    sethomefile(genbuf, hacker, ".DIR");
    return rec_add(genbuf, &mymail, sizeof(mymail));
  }
  /* setup the hostname and username */
  if (!configured)
  {
    /* get host name */
    gethostname(myhostname, STRLEN);
    hbuf = gethostbyname(myhostname);
    if (hbuf)
      strncpy(myhostname, hbuf->h_name, STRLEN);

    /* get bbs uident */
    pbuf = getpwuid(getuid());
    if (pbuf)
      strncpy(myusername, pbuf->pw_name, 20);
    if (hbuf && pbuf)
      configured = 1;
    else
      return -1;
  }
  /* Running the sendmail */

#ifdef  INTERNET_PRIVATE_EMAIL
  if (fpath == NULL)
  {
    sprintf(genbuf, "/usr/sbin/sendmail %s > /dev/null", receiver);
    fin = fopen("etc/confirm", "r");
  } else
  {
    sprintf(genbuf, "/usr/sbin/sendmail -f %s%s %s > /dev/null"
	    ,cuser.userid, str_mail_address, receiver);
    fin = fopen(fpath, "r");
  }
  fout = popen(genbuf, "w");
  if (fin == NULL || fout == NULL)
    return -1;

  if (fpath)
    fprintf(fout, "Reply-To: %s%s\nFrom: %s%s\n",
	    cuser.userid, str_mail_address, cuser.userid, str_mail_address);
#else
  sprintf(genbuf, "/usr/sbin/sendmail %s > /dev/null", receiver);
  fout = popen(genbuf, "w");
  fin = fopen(fpath ? fpath : "etc/confirm", "r");
  if (fin == NULL || fout == NULL)
    return -1;

  if (fpath)
    fprintf(fout, "From: %s@%s (%s)\n", myusername, myhostname, BBSNAME);
#endif

  fprintf(fout, "To: %s\nSubject: %s\n", receiver, title);
  fprintf(fout, "X-Disclaimer: " BOARDNAME "對本信內容恕不負責。\n\n");

  while (fgets(genbuf, sizeof(genbuf), fin))
  {
#ifdef REG_MAGICKEY
    char *po;
#endif    

    if (genbuf[0] == '.' && genbuf[1] == '\n')
      fputs(". \n", fout);
    else
    {
#ifdef REG_MAGICKEY
      while (po = strstr(genbuf, "<Magic>"))
      {
	char buf[128];

	po[0] = 0;
	sprintf(buf, "%s%s%s", genbuf, key, po + 7);
	strcpy(genbuf, buf);
      }
#endif
      fputs(genbuf, fout);
    }
  }
  fclose(fin);
  fprintf(fout, ".\n");
  pclose(fout);
  return 0;
}


int
doforward(direct, fh, mode)
  char *direct;
  fileheader *fh;
  int mode;			/* 是否 uuencode */
{
  static char address[60];
  fileheader fhdr;
  char fname[MAXPATHLEN], fpath[MAXPATHLEN], genbuf[200];
  int return_no, taged, locus;

  if (mode != 'Z')		/* shakalaca.000715: 打包不用 tag */
    taged = AskTag("轉寄");
  else
    taged = 0;

  if (taged < 0)
    return taged;

  if (!address[0])
    strcpy(address, cuser.email);

  if (address[0])
  {
    sprintf(genbuf, "確定轉寄給 [%s] 嗎 (Y/N/Q)？[Y] ", address);
    switch (getans(genbuf))
      {
	case 'q':
	  outz("取消轉寄");
	  return 1;
	case 'n':
	  address[0] = '\0';
	default:
	  break;
      }
  }
  if (!address[0])
  {
    getdata(b_lines, 0, "請輸入轉寄地址：", fname, 60, DOECHO, 0);
    if (fname[0])
    {
      if (strchr(fname, '.'))
	strcpy(address, fname);
      else
	sprintf(address, "%s.bbs@%s", fname, MYHOSTNAME);
    } else
    {
      outz("取消轉寄");
      return 1;
    }
  }
  if (not_addr(address))
    return -2;

  if (taged)
  {
    fh = &fhdr;
    sprintf(fpath, "%s/.DIR", direct);
  }
  locus = 0;
  do
  {
    if (taged)
    {
      EnumTagFhdr(fh, fpath, locus);
      locus++;
    }
    if (mode == 'Z')
    {
      FILE *fp;
      int address_ok = valid_ident(address);

      if (fp = fopen("mbox_sent", "a"))
      {
	time_t now = time(0);

	fprintf(fp, "%c%-12s %s => %s\n",
	    address_ok ? ' ' : '-', cuser.userid, Cdatelite(&now), address);
	fclose(fp);
      }
      if (!address_ok)
      {
	sprintf(fname, "無效的工作站位址 %s", address);
	pressanykey(fname);
	return -2;
      }
      sprintf(fname, "cd home; tar cfz - %s | uuencode %s.tgz > /tmp/%s.tgz",
	      cuser.userid, cuser.userid, cuser.userid);
      system(fname);
      strcpy(fname, direct);
    } else if (mode == 'U')
    {
      char tmp_buf[128];

      sprintf(fname, "/tmp/bbs.uu%05d", currpid);
      sprintf(tmp_buf, "/usr/bin/uuencode %s/%s uu.%05d > %s",
	      direct, fh->filename, currpid, fname);
      system(tmp_buf);
    } else
      sprintf(fname, "%s/%s", direct, fh->filename);

    return_no = bbs_sendmail(fname, fh->title, address, NULL);
  } while ((locus < taged) && (!return_no));

  if (mode != 'F')
    return (return_no);
}

#endif


int
chkmail(rechk)
  int rechk;
{
  static time_t lasttime = 0;
  static int ismail = 0;
  struct stat st;
  int fd;
  register numfiles;
  unsigned char ch;

  if (!HAS_PERM(PERM_BASIC))
    return 0;

  if (stat(currmaildir, &st) < 0)
    return (ismail = 0);

  if ((lasttime >= st.st_mtime) && !rechk)
    return ismail;

  lasttime = st.st_mtime;
  numfiles = st.st_size / sizeof(fileheader);
  if (numfiles <= 0)
    return (ismail = 0);

  /* ------------------------------------------------ */
  /* 看看有沒有信件還沒讀過？從檔尾回頭檢查，效率較高 */
  /* ------------------------------------------------ */

  if ((fd = open(currmaildir, O_RDONLY)) > 0)
  {
    lseek(fd, (off_t)(st.st_size - 1), SEEK_SET);
    while (numfiles--)
    {
      read(fd, &ch, 1);
      if (!(ch & FILE_READ))
      {
        close(fd);
        return (ismail = 1);
      }
      lseek(fd, -(off_t)(sizeof(fileheader) + 1), SEEK_CUR);
    }
    close(fd);
  }
  return (ismail = 0);
}

#ifdef  REG_MAGICKEY
void
mail_justify(userec muser)
{
  fileheader mhdr;
  char title[128], buf1[80];

  sethomepath(buf1, muser.userid);
  stampfile(buf1, &mhdr);
  unlink(buf1);
  strcpy(mhdr.owner, "站長");
  strncpy(mhdr.title, "[EMail認證]", TTLEN);
  mhdr.savemode = 0;
  mhdr.filemode = 0;

  if (valid_ident(muser.email) && !not_addr(muser.email))
  {
    char title[80], *ptr, ch, MagicKey[9];
    ushort checksum;            /* 16-bit is enough */
    time_t now;
                
    checksum = getuser(muser.userid);
    ptr = muser.email;
    while (ch = *ptr++)
    {
      if (ch <= ' ')
        break;
      if (ch >= 'A' && ch <= 'Z')
        ch |= 0x20;
      checksum = (checksum << 1) ^ ch;
    }
    /* shakalaca.990725: 身份認證用的 MagicKey */
    sprintf(title, "%d", checksum + time(&now));
    strncpy(MagicKey, title, 8);
    MagicKey[8] = '\0';

/*
    sprintf(title, "[WD BBS]To %s(%d:%d) [User Justify]",
      muser.userid, getuser(muser.userid) + MAGIC_KEY, checksum);
*/ /* shakalaca.990718: 將證認標記獨立出來方便修改 */
/*
    sprintf(title, "%s%s(%d:%d) [User Justify]", TAG_VALID, 
      muser.userid, getuser(muser.userid) + MAGIC_KEY, checksum);
*/
    sprintf(title, "%s To %s: [重要 ! 請閱\讀]", TAG_VALID, muser.userid);

    if (bbs_sendmail(NULL, title, muser.email, MagicKey) < 0)
      f_cp("etc/bademail", buf1, O_TRUNC);
    else
      f_cp("etc/justify", buf1, O_TRUNC);
  }
  else
    f_cp("etc/bademail", buf1, O_TRUNC);

  sethomedir(title, muser.userid);
  rec_add(title, &mhdr, sizeof(mhdr));
}
#endif                          /* REG_MAGICKEY */
