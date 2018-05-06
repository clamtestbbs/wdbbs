/* bbs.c        ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : bulletin boards' routines                    */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

#define MAXPATHLEN  1024
extern int mail_reply ();
extern char currdirect[64];
extern int TagNum;
extern struct BCACHE *brdshm;

extern void Read();

time_t board_note_time;
time_t board_visit_time;

static char *brd_title;
char real_name[20];
int local_article;
char currmaildir[32];
//extern char *fcolor[11];
char *rcolor[11] = { "\x1b[36m", "","\x1b[32m","\x1b[1;32m",
                   "\x1b[33m","\x1b[1;33m","\x1b[1;37m" ,"\x1b[1;36m",
                   "\x1b[1;31m", "\x1b[1;35m", "\x1b[1;36m"};
#define UPDATE_USEREC   (currmode |= MODE_DIRTY)

void
log_board (board, usetime)
  char *board;
  time_t usetime;
{
  time_t now;
  boardheader bh;
  int bid = getbnum (board);

  now = time (0);
  rec_get (fn_board, &bh, sizeof (bh), bid);
  if (usetime >= 10)
  {
    ++bh.totalvisit;
    bh.totaltime += usetime;
    strcpy (bh.lastvisit, cuser.userid);
    bh.lastime = now;
  }
  substitute_record (fn_board, &bh, sizeof (bh), bid);
}

void
log_board2( mode, usetime )
char *mode;
time_t usetime;
{
    time_t      now;
    FILE        *fp;
    char        buf[512];

    now = time(0);
    sprintf( buf, "USE %-20.20s Stay: %5ld (%s) %s",
       mode, usetime ,cuser.userid, ctime(&now));
    if(usetime > 5  && (fp = fopen(BBSHOME"/usboard", "a" )) != NULL) {
        fputs( buf, fp );
        fclose( fp );
    }
}

void
log_board3( mode, str, num )
char *mode;
char *str;
int  *num;
{
    time_t      now;
    FILE        *fp;
    char        buf[512];

    now = time(0);
    sprintf( buf, "%3s %-20.20s with: %5ld (%s) %s",
      mode, str, num ,cuser.userid,ctime(&now));
    if( (fp = fopen(BBSHOME"/usboard", "a" )) != NULL ) {
        fputs( buf, fp );
        fclose( fp );
    }
}

static int
g_board_names (fhdr)
     boardheader *fhdr;
{
  AddNameList (fhdr->brdname);
  return 0;
}


void
make_blist ()
{
  CreateNameList ();
  apply_boards (g_board_names);
}


static int
g_bm_names(bh)
  boardheader *bh;
{
  char buf[IDLEN * 3 + 3];
  char* uid;

  strcpy(buf, bh->BM);
  uid = strtok(buf, "/");       /* shakalaca.990721: 找第一個板主 */
  while (uid)
  {
    if (!InNameList(uid) && searchuser(uid))
      AddNameList(uid);
    uid = strtok(0, "/");       /* shakalaca.990721: 指向下一個 */
  }
  return 0;
}

/* shakalaca.990721: 所有 BM 名單 */
void
make_bmlist()
{
  CreateNameList();
  apply_boards(g_bm_names);
}


void
set_board ()
{
  boardheader *bp;
  boardheader *getbcache ();

  bp = getbcache (currboard);
  board_note_time = bp->bupdate;
  brd_title = bp->BM;
  if (brd_title[0] <= ' ')
    brd_title = "徵求中";
  sprintf (currBM, "板主：%s", brd_title);
  brd_title = (bp->bvote == 1 ? "本看板進行投票中" : bp->title + 7);

  currmode = (currmode & MODE_DIRTY) | MODE_STARTED;
  if (HAS_PERM (PERM_ALLBOARD) ||
      (HAS_PERM (PERM_BM) && is_BM (currBM + 6)))
    {
      currmode |= (MODE_BOARD | MODE_POST);
    }
  else if (haspostperm (currboard))
    currmode |= MODE_POST;
}


static void
readtitle ()
{
  showtitle (currBM, brd_title);
  outs ("\
[←]離開 [→]閱\讀 [^P]發表文章 [b]備忘錄 [d]刪除 [z]精華區 [TAB]文摘 [^Z]求助\n\
" COLOR1 "\x1b[1m  編號   SC");
  if (currmode & MODE_TINLIKE)
    outs (" 篇 數");
  else
    outs (" 日 期");
  outs (" 作  者      文  章  標  題                                    \x1b[m");
}


void
doent (num, ent)
     int num;
     fileheader *ent;
{
  int tag;
  user_info *checkowner;
  char *mark, *title, color, type[10],buf[255];
  static char *colors[7] =
  {"\x1b[1;36m", "\x1b[1;34m", "\x1b[1;33m", "\x1b[1;32m", "\x1b[1;35m", "\x1b[1;36m", "\x1b[1;37m"};

  if(ent->score != 0)
    sprintf(buf , "%s%02d %s",ent->score > 0 ? "\x1b[1;31m" : "\x1b[1;32m", ent->score,colors[(unsigned int) (ent->date[4] + ent->date[5]) % 7]);
  else
    sprintf(buf , "   %s",colors[(unsigned int) (ent->date[4] + ent->date[5]) % 7]);
  if (currstat != RMAIL)
  {
    sprintf(type,"%c",brc_unread (ent->filename) ? '+' : ' ');

    tag = ' ';
/*  if (TagNum && !Tagger(atoi(ent->filename + 2) , num, TAG_NIN))
    tag = '*';
*/
    if ((currmode & MODE_BOARD) && (ent->filemode & FILE_DIGEST))
      sprintf(type ,"\x1b[1;35m%c",(type[0] == ' ') ? '*' : '#');
    if (ent->filemode & FILE_MARKED)
      sprintf(type ,"\x1b[1;36m%c",(type[0] == ' ') ? 'm' : 'M');
  }
  else
  {
    sprintf(type,"%c","+ Mm"[ent->filemode]);

    if (ent->filemode & FILE_REPLYOK)
      sprintf(type ,"\x1b[1;31m%c",(type[0] == ' ') ? 'r' : 'R');
  }
  /* shakalaca.990421: 改了一下, 因為 mail 中看起來不好看 */

  if (ent->filemode & FILE_TAGED && brc_unread (ent->filename))
    sprintf(type,"\x1b[1;32m%c", 'T');
  else if (ent->filemode & FILE_TAGED)
    sprintf(type,"\x1b[1;32m%c", 't');

  title = str_ttl (mark = ent->title);
  if (title == mark)
    {
      color = '6';
      mark = "□";
    }
  else
    {
      color = '3';
      mark = "R:";
    }

  if (title[47])
    strcpy (title + 44, " …");  /* 把多餘的 string 砍掉 */
  checkowner =(user_info *) searchowner(ent->owner);
  if (strncmp (currtitle, title, TTLEN))
    prints ("%6d %s%c%s%-6s\x1b[m%s%-12.12s%s%s %s\n", num, type,tag,buf,
      ent->date, checkowner ? rcolor[is_friend(checkowner)] : "", ent->owner, checkowner ? "\x1b[m" : "", mark, title);
  else
    prints ("%6d %s%c%s%-6s\x1b[m%s%-12.12s%s\x1b[1;3%cm%s %s\x1b[m\n", num, type,tag,buf,
      ent->date, checkowner ? rcolor[is_friend(checkowner)] : "", ent->owner, checkowner ? "\x1b[m" : "", color, mark, title);
}


int
cmpbnames (bname, brec)
     char *bname;
     boardheader *brec;
{
  return (!ci_strncmp (bname, brec->brdname, sizeof (brec->brdname)));
}


int
cmpfilename (fhdr)
     fileheader *fhdr;
{
  return (!strcmp (fhdr->filename, currfile));
}

#if 0
untag(fhdr, dirpath)
  fileheader *fhdr;
  char *dirpath;
{
  int now;

  fhdr->filemode ^= FILE_TAGED;
  now = getindex (dirpath, fhdr->filename, sizeof (fileheader));
  substitute_record (dirpath, fhdr, sizeof (*fhdr), now);
}
#endif
/*
   woju
 */
int
cmpfmode (fhdr)
     fileheader *fhdr;
{
  return (fhdr->filemode & currfmode);
}


int
cmpfowner (fhdr)
     fileheader *fhdr;
{
  return !strcasecmp (fhdr->owner, currowner);
}



int
do_select (ent, fhdr, direct)
     int ent;
     fileheader *fhdr;
     char *direct;
{
  char bname[20];
  char bpath[60], buf[80];
  struct stat st;

  move (0, 0);
  clrtoeol ();
  make_blist ();
  namecomplete (MSG_SELECT_BOARD, bname);

  setbpath (bpath, bname);
  if ((*bname == '\0') || (stat (bpath, &st) == -1))
    {
      move (2, 0);
      clrtoeol ();
      pressanykey (err_bid);
      return RC_FULL;
    }

/*
   board_visit_time = 0x7fffffff;
 */
  brc_initial (bname);
  setbfile (buf, bname, FN_LIST);
  if (currbrdattr & BRD_HIDE && belong_list(buf, cuser.userid) <= 0)
  {
    pressanykey (P_BOARD);
    return RC_FULL;
  }
  set_board ();
  setbdir (direct, currboard);

  move (1, 0);
  clrtoeol ();
  return RC_NEWDIR;
}

/* ----------------------------------------------------- */
/* 改良 innbbsd 轉出信件、連線砍信之處理程序             */
/* ----------------------------------------------------- */

outgo_post(fh, board)
  fileheader *fh;
  char *board;
{
  char buf[512];
  if(strcmp(fh->owner,cuser.userid))
    sprintf (buf, "%s\t%s\t%s\t%s\t%s", board,
      fh->filename, fh->owner, "轉錄", fh->title);
  else
    sprintf (buf, "%s\t%s\t%s\t%s\t%s", board,
      fh->filename, fh->owner, cuser.username, fh->title);
  f_cat ("innd/out.bntp", buf);
}


static void
cancelpost (fh, by_BM)
     fileheader *fh;
     int by_BM;
{
  FILE *fin;
  char *ptr, *brd;
  fileheader postfile;
  char genbuf[512], buf[512];
  char nick[STRLEN], fn1[STRLEN], fn2[STRLEN];

  setbfile (fn1, currboard, fh->filename);
  if (fin = fopen (fn1, "r"))
  {
    brd = by_BM ? "deleted" : "junk";
    setbpath (fn2, brd);
    stampfile (fn2, &postfile);
    memcpy (postfile.owner, fh->owner, IDLEN + TTLEN + 10);
    postfile.savemode = 'D';
    log_board3("DEL", currboard, 1);
    if (fh->savemode == 'S')
    {
      nick[0] = '\0';
      while (fgets (genbuf, sizeof (genbuf), fin))
      {
        if (!strncmp (genbuf, str_author1, LEN_AUTHOR1) ||
            !strncmp (genbuf, str_author2, LEN_AUTHOR2))
        {
          if (ptr = strrchr (genbuf, ')'))
          *ptr = '\0';
          if (ptr = (char *) strchr (genbuf, '('))
          strcpy (nick, ptr + 1);
          break;
        }
      }

      sprintf (buf, "%s\t%s\t%s\t%s\t%s",
        currboard, fh->filename, fh->owner, nick, fh->title);
      f_cat ("innd/cancel.bntp", buf);
    }
    fclose (fin);
    f_mv (fn1, fn2);
    setbdir (genbuf, brd);
    rec_add (genbuf, &postfile, sizeof (postfile));
  }
}


/* ----------------------------------------------------- */
/* 發表、回應、編輯、轉錄文章                            */
/* ----------------------------------------------------- */


void
do_reply_title (row, title)
  int row;
  char *title;
{
  char genbuf[512];
  char genbuf2[4];

  if (ci_strncmp (title, str_reply, 4))
    sprintf (save_title, "Re: %s", title);
  else
    strcpy (save_title, title);
  save_title[TTLEN - 1] = '\0';
  sprintf (genbuf, "採用原標題《%.60s》嗎?[Y] ", save_title);
  getdata (row, 0, genbuf, genbuf2, 4, LCECHO, 0);
  if (*genbuf2 == 'n')
    getdata (++row, 0, "標題：", save_title, TTLEN, DOECHO, 0);
}


static void
do_reply (fhdr)
     fileheader *fhdr;
{
  char genbuf[512];

// Ptt 看板連署系統
  if(!strcmp(currboard,VOTEBOARD))
    DL_func("SO/votebrd.so:va_do_voteboardreply",fhdr);
  else
  {
    getdata (b_lines - 1, 0,
      "▲ 回應至 (F)看板 (M)作者信箱 (B)二者皆是 (Q)取消？[F] ",
      genbuf, 3, LCECHO, 0);
    switch (genbuf[0])
    {
      case 'm':
        mail_reply (0, fhdr, 0);
      case 'q':
        *quote_file = 0;
        break;

      case 'b':
        curredit = EDIT_BOTH;
      default:
        strcpy (currtitle, fhdr->title);
        strcpy (quote_user, fhdr->owner);
        quote_file[79] = fhdr->savemode;
        do_post ();
    }
  }
  *quote_file = 0;
}


/*
   woju
 */
brdperm (char *brdname, char *userid)
{
  boardheader *bp;
  boardheader *getbcache ();
  int uid = searchuser (userid);

  bp = getbcache (currboard);
  if (uid && bp)
    {
      int level = bp->level;
      char *ptr = bp->BM;
      char buf[64], manager[IDLEN + 1];
      userec xuser;

      rec_get (fn_passwd, &xuser, sizeof (xuser), uid);
      if ((level & BRD_POSTMASK) || ((level) ? xuser.userlevel & (level) : 1))
  return 1;

      if (ptr[0] <= ' ')
  return 0;

      if (userid_is_BM (userid, ptr))
  return 1;

      if ((level & 0xffff) != PERM_BBSADM)
  return 0;

      strncpy (manager, ptr, IDLEN + 1);
      if (ptr = strchr (manager, '/'))
  *ptr = 0;
      sethomefile (buf, manager, fn_overrides);
      return (belong (buf, userid));
    }
  return 0;
}

#ifdef POSTNOTIFY
do_postnotify (char *fpath)
{
  fileheader mhdr;
  char title[512], buf1[80], buf[80];
  FILE *fp;
  char genbuf[512];

  setuserfile (genbuf, "postnotify.ok");
  if (fp = fopen (genbuf, "r"))
    {
      char last_fname[80];
      boardheader *bp;
      boardheader *getbcache ();

      strcpy (last_fname, fpath);
      bp = getbcache (currboard);
      while (fgets (buf, 80, fp))
  if (brdperm (currboard, buf))
    {
      sethomefile(buf1, buf, "postlist");
      if (belong_list(buf1, buf) <= 0)
      {
        del_distinct(genbuf, cuser.userid);
        fclose(fp);
        return;
      }

      sethomepath (buf1, buf);
      stampfile (buf1, &mhdr);
      strcpy (mhdr.owner, cuser.userid);
      strcpy (mhdr.title, "[新]");
      strncat (mhdr.title, save_title, TTLEN - 4);
      mhdr.savemode = 0;
      mhdr.filemode = 0;
      sethomedir (title, buf);
      rec_add (title, &mhdr, sizeof (mhdr));
      unlink (buf1);
      f_cp (last_fname, buf1, O_TRUNC);
      strcpy (last_fname, buf1);
    }
      fclose (fp);
    }
}
#endif

do_unanonymous_post (char *fpath)
{
  fileheader mhdr;
  char title[512];
  char genbuf[512];

  setbpath (genbuf, "UnAnonymous");
  if (dashd (genbuf))
    {
      stampfile (genbuf, &mhdr);
      unlink (genbuf);
      f_ln (fpath, genbuf);
      strcpy (mhdr.owner, cuser.userid);
      strcpy (mhdr.title, save_title);
      mhdr.savemode = 0;
      mhdr.filemode = 0;
      setbdir (title, "UnAnonymous");
      rec_add (title, &mhdr, sizeof (mhdr));
    }
}

do_all_post (char *fpath)
{
  fileheader mhdr;
  char title[512];
  char genbuf[512];

  setbpath (genbuf, "All_Post");
  if (dashd (genbuf))
  {
    stampfile (genbuf, &mhdr);
    unlink (genbuf);
    f_ln (fpath, genbuf);
    strcpy (mhdr.owner, cuser.userid);
    strcpy (mhdr.title, save_title);
    mhdr.savemode = 0;
    mhdr.filemode = 0;
    setbdir (title, "All_Post");
    rec_add (title, &mhdr, sizeof (mhdr));
  }
  sethomefile(genbuf, cuser.userid ,"allpost");
  if(!dashd (genbuf))
    mkdir(genbuf,0755);
  stampfile (genbuf, &mhdr);
  unlink (genbuf);
  f_ln (fpath, genbuf);
  strcpy (mhdr.owner, cuser.userid);
  strcpy (mhdr.title, save_title);
  mhdr.savemode = 0;
  mhdr.filemode = 0;
  sprintf (title,BBSHOME"/home/%s/allpost/.DIR", cuser.userid);
  rec_add (title, &mhdr, sizeof (mhdr));
}

/* Ptt test */
getindex (fpath, fname, size)
     char *fpath;
     char *fname;
     int size;
{
  int fd, now = 0;
  fileheader fhdr;

  if ((fd = open (fpath, O_RDONLY, 0)) != -1)
    {
      while ((read (fd, &fhdr, size) == size))
  {
    now++;
    if (!strcmp (fhdr.filename, fname))
      {
        close (fd);
        return now;
      }
  }
      close (fd);
    }
  return 0;
}

extern long wordsnum;    /* 計算字數 */

int
do_post ()
{
  FILE *fp;
  fileheader postfile;
  char fpath[80], buf[80];
  int aborted;
  char genbuf[512], *owner;
  boardheader *bp;
  boardheader *getbcache ();
  time_t spendtime;
  int num = 0, i = 0;
  char tmp[20][80];
  char *postprefix[] = 
  {"[公告] ", "[新聞] ", "[閒聊] ", "[文件] ", "[問題] ", "[創作] ", "[隨便] ",
   "[測試] ", "[其他] ",NULL};

// Ptt 看板連署系統
  if(!strcmp(currboard,VOTEBOARD))
  {
//    do_voteboard();
    DL_func("SO/votebrd.so:do_voteboard");
    return RC_FULL;
  }
  sprintf(genbuf,BBSHOME"/boards/%s/prefix",currboard);
  if(fp = fopen(genbuf,"r"))
  {
    while(fgets(buf, 80, fp))
    {
      sprintf(tmp[i],"[%s] ",strtok(buf,"\n"));
      postprefix[i] = tmp[i];
      i++;
    }
    postprefix[i] = NULL;
    i = 0;
    fclose(fp);
  }
  bp = getbcache (currboard);
  clear ();
  setbfile (buf, currboard, FN_LIST);
  if (!(currmode & MODE_POST) || !brdperm (currboard, cuser.userid) 
    || (belong_list (buf, cuser.userid) < 0 && !HAS_PERM(PERM_SYSOP)))
  {
    pressanykey ("對不起，您目前無法在此發表文章！");
    return RC_FULL;
  }

  if (currbrdattr & BRD_INVITE && belong_list(buf, cuser.userid) <= 0 && !HAS_PERM(PERM_BBSADM))
  {
    pressanykey ("對不起,此板只准看板好友才能發表文章,請向板主申請");
    return RC_FULL;
  }

  more ("etc/post.note", NA);
  prints ("發表文章於【 %s 】看板\n", currboard);

  if (quote_file[0])
    do_reply_title (20, currtitle);
  else
  {
    memset (save_title, 0, TTLEN);
    while (postprefix[num] != NULL)
    {
      prints ("%d.%s", num + 1, postprefix[num]);
      num++;
    }
    getdata (20, 0, "請選擇文章類別或自行輸入類別(或按Enter跳過)：", genbuf, 9, DOECHO, 0);
    i = atoi (genbuf);
    if (i > 0 && i <= num)  /* 輸入數字選項 */
      strncpy (save_title, postprefix[i - 1], strlen (postprefix[i - 1]));
    else if (strlen (genbuf) >= 3)  /* 自行輸入 */
      strncpy (save_title, genbuf, strlen (genbuf));
    else      /* 空白跳過 */
      save_title[0] = '\0';

    getdata (21, 0, "標題：", save_title, TTLEN, DOECHO, save_title);
    strip_ansi (save_title, save_title, ONLY_COLOR);
  }
  if (save_title[0] == '\0')
    return RC_FULL;

  curredit &= ~EDIT_MAIL;
  curredit &= ~EDIT_ITEM;
  setutmpmode (POSTING);

  /* 未具備 Internet 權限者，只能在站內發表文章 */

  if (!HAS_PERM (PERM_INTERNET))
    local_article = 1;

  buf[0] = 0;

  spendtime = time (0);
  aborted = vedit (buf, YEA);
  spendtime = time (0) - spendtime;
  if (aborted == -1)
  {
    unlink (buf);
    pressanykey (NULL);
    return RC_FULL;
  }

  /* build filename */

  setbpath (fpath, currboard);
  stampfile (fpath, &postfile);
  f_mv (buf, fpath);
  strcpy (postfile.owner, cuser.userid);

  /* set owner to Anonymous , for Anonymous board */

#ifdef HAVE_ANONYMOUS
/* Ptt and Jaky */
  if (currbrdattr & BRD_ANONYMOUS && strcmp (real_name, "r"))
  {
    strcat (real_name, ".");
    owner = real_name;
  }
  else
  {
#endif
    owner = cuser.userid;
#ifdef HAVE_ANONYMOUS
  }
#endif

  strcpy (postfile.owner, owner);
  strcpy (postfile.title, save_title);
  if (aborted == 1)    /* local save */
  {
    postfile.savemode = 'L';
    postfile.filemode = FILE_LOCAL;
  }
  else
    postfile.savemode = 'S';

  setbdir (buf, currboard);
  if (rec_add (buf, &postfile, sizeof (postfile)) != -1)
  {
    if (currmode & MODE_SELECT)
    rec_add (currdirect, &postfile, sizeof (postfile));
    if (aborted != 1)// && !(currbrdattr & BRD_NOTRAN))
      outgo_post (&postfile, currboard);
    brc_addlist (postfile.filename);

    if (!(currbrdattr & BRD_NOCOUNT))
    {
      if (wordsnum <= 30)
        pressanykey ("抱歉，太短的文章不列入紀錄。");
      else
      {
        clear ();
        move (7, 0);
        update_data ();
        prints (""\
"              \x1b[1;36m【\x1b[37m發  表  完  畢\x1b[36m】\n\n" \
"              \x1b[37m 這是您的\x1b[33m第 %d 篇\x1b[37m文章。\n" \
"              \x1b[36m【費時】\x1b[33m %d \x1b[37m分\x1b[33m % d \x1b[37m秒。\n" \
"              \x1b[36m【字數】\x1b[33m %d \n",\
        ++cuser.numposts, spendtime / 60, spendtime % 60 , wordsnum);
        substitute_record (fn_passwd, &cuser, sizeof (userec), usernum);

        pressanykey ("文章發表完畢 :)");
      }
//      UPDATE_USEREC;
    }
    else
      pressanykey ("本看板文章不列入紀錄，敬請包涵。");

    log_board3("POS", currboard, cuser.numposts);

  /* 回應到原作者信箱 */
    if (curredit & EDIT_BOTH)
    {
      char *str, *msg = "回應至作者信箱";

      if (str = strchr (quote_user, '.'))
      {
        if (bbs_sendmail (fpath, save_title, str + 1, NULL) < 0)
          msg = "作者無法收信";
      }
      else
      {
        sethomepath (genbuf, quote_user);
        stampfile (genbuf, &postfile);
        unlink (genbuf);
        f_cp (fpath, genbuf, O_TRUNC);

        strcpy (postfile.owner, cuser.userid);
        strcpy (postfile.title, save_title);
        postfile.savemode = 'B';  /* both-reply flag */
        sethomedir (genbuf, quote_user);
        if (rec_add (genbuf, &postfile, sizeof (postfile)) == -1)
          msg = err_uid;
      }
      outs (msg);
      curredit ^= EDIT_BOTH;
    }
    do_all_post(fpath); // 紀錄所有站內的貼文
// wildcat : do_all_post 取代
//    if (currbrdattr & BRD_ANONYMOUS)  /*  反匿名  */
//      do_unanonymous_post (fpath);

#ifdef POSTNOTIFY    /* 新文章通知 */
    if (!(currbrdattr & BRD_ANONYMOUS) && !(currbrdattr & BRD_HIDE))
      do_postnotify (fpath);
#endif
  }
  return RC_FULL;
}


static int
reply_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  if (!(currmode & MODE_POST))
    return RC_NONE;

  setdirpath (quote_file, direct, fhdr->filename);
// Ptt 的看板連署系統
  if(!strcmp(currboard,VOTEBOARD))
    DL_func("SO/votebrd.so:va_do_voteboardreply",fhdr);
  else
    do_reply (fhdr);
  *quote_file = 0;
  return RC_FULL;
}


int
edit_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char fpath[80], fpath0[80];
  extern bad_user (char *name);
  char genbuf[512];
  fileheader postfile;
  int edit_mode;

  if (!strcmp(currboard,"Security")) return RC_NONE;

  if (currstat == RMAIL)
  {
    setdirpath (genbuf, direct, fhdr->filename);
    vedit (genbuf, belong ("etc/sysop", cuser.userid) ? 0 : 2);
    return RC_FULL;
  }

  if (HAS_PERM (PERM_SYSOP) ||
    (!strcmp (fhdr->owner, cuser.userid) && strcmp (cuser.userid, "guest") &&
    !bad_user (cuser.userid)))
    edit_mode = 0;
  else
    edit_mode = 2;

  setdirpath (genbuf, direct, fhdr->filename);
  local_article = fhdr->filemode & FILE_LOCAL;
  strcpy (save_title, fhdr->title);

  if (vedit (genbuf, edit_mode) != -1)
  {
    int now;
    setbpath (fpath, currboard);
    stampfile (fpath, &postfile);
    unlink (fpath);
    setbfile (fpath0, currboard, fhdr->filename);
    f_mv (fpath0, fpath);

    if (currmode&MODE_SELECT) {   // CityLion: SELECT時也要修改原.DIR
       setbdir(genbuf,currboard);
       now = getindex(genbuf,fhdr->filename,sizeof(fileheader));
    }

    strcpy (fhdr->filename, postfile.filename);
    strcpy (fhdr->title, save_title);
    brc_addlist (postfile.filename);
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);

    if (currmode&MODE_SELECT)     // CityLion: SELECT時也要修改原.DIR
       substitute_record(genbuf, fhdr, sizeof(*fhdr), now);

#ifdef POSTNOTIFY
    if (currbrdattr & BRD_ANONYMOUS)
      do_postnotify (fpath);
#endif

  }
  return RC_FULL;
}


static int
cross_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char xboard[20], fname[80], xfpath[80], xtitle[80], inputbuf[10];
  fileheader xfile;
  FILE * xptr;
  int author = 0;
  char genbuf[512];
  char genbuf2[4];

  make_blist ();
  move (2, 0);
  clrtoeol ();
  move (3, 0);
  clrtoeol ();
  move (1, 0);
  namecomplete ("轉錄本文章於看板：", xboard);
  if (*xboard == '\0' || !haspostperm (xboard))
    return RC_FULL;

  ent = 1;
  if (HAS_PERM (PERM_SYSOP) || !strcmp (fhdr->owner, cuser.userid))
  {
    getdata (2, 0, "(1)原文轉載 (2)舊轉錄格式？[1] ",
    genbuf, 3, DOECHO, "1");
    if (genbuf[0] != '2')
    {
      ent = 0;
      getdata (2, 0, "保留原作者名稱嗎?[Y] ", inputbuf, 3, DOECHO, 0);
      if (inputbuf[0] != 'n' && inputbuf[0] != 'N') author = 1;
    }
  }

  if (ent)
    sprintf (xtitle, "[轉錄]%.66s", fhdr->title);
  else
    strcpy (xtitle, fhdr->title);

  sprintf (genbuf, "採用原標題《%.60s》嗎?[Y] ", xtitle);
  getdata (2, 0, genbuf, genbuf2, 4, LCECHO, 0);
  if (*genbuf2 == 'n')
  {
    if (getdata (2, 0, "標題：", genbuf, TTLEN, DOECHO, xtitle))
      strcpy (xtitle, genbuf);
  }

  getdata (2, 0, "(S)存檔 (L)站內 (Q)取消？[S] ", genbuf, 3, LCECHO, "S");
  if (genbuf[0] == 'l' || genbuf[0] == 's')
  {
    int currmode0 = currmode;

    currmode = 0;
    setbpath (xfpath, xboard);
    stampfile (xfpath, &xfile);
    if (author)
      strcpy (xfile.owner, fhdr->owner);
    else
      strcpy (xfile.owner, cuser.userid);
    strcpy (xfile.title, xtitle);
    if (genbuf[0] == 'l')
    {
      xfile.savemode = 'L';
      xfile.filemode = FILE_LOCAL;
    }
    else
      xfile.savemode = 'S';

    setbfile (fname, currboard, fhdr->filename);
    if (ent)
    {
      xptr = fopen (xfpath, "w");
      strcpy (save_title, xfile.title);
      strcpy (xfpath, currboard);
      strcpy (currboard, xboard);
      write_header (xptr);
      strcpy (currboard, xfpath);

      fprintf (xptr, "※ [本文轉錄自 %s 看板]\n\n", currboard);

      b_suckinfile (xptr, fname);
      addsignature (xptr);
      fclose (xptr);
    }
    else
    {
      unlink (xfpath);
      f_cp (fname, xfpath, O_TRUNC);
    }

    setbdir (fname, xboard);
    rec_add (fname, (char *) &xfile, sizeof (xfile));
    if (!xfile.filemode)
      outgo_post (&xfile, xboard);
    cuser.numposts++;
    UPDATE_USEREC;
    pressanykey ("文章轉錄完成");
    currmode = currmode0;
  }
  return RC_FULL;
}

static int
read_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[512];
  int more_result;

  if (fhdr->owner[0] == '-')
    return RC_NONE;

  setdirpath (genbuf, direct, fhdr->filename);

//  if(dashd(genbuf))
//    read_dir(genbuf,fhdr->title);

  if ((more_result = more (genbuf, YEA)) == -1)
    return RC_NONE;

  brc_addlist (fhdr->filename);
  strncpy (currtitle, str_ttl(fhdr->title), TTLEN);
  strncpy (currowner, str_ttl(fhdr->owner), STRLEN);
/*  woju  */
  switch (more_result)
  {
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
    case 8:
      if (currmode & MODE_POST)
      {
        strcpy (quote_file, genbuf);
        do_reply (fhdr);
        *quote_file = 0;
      }
      return RC_FULL;
    case 9:
      return 'A';
    case 10:
      return 'a';
    case 11:
      return '/';
    case 12:
      return '?';
  }
  return RC_FULL;
}



/* ----------------------------------------------------- */
/* 採集精華區                                            */
/* ----------------------------------------------------- */

man()
{
  char buf[64], buf1[64], xboard[20], fpath[512];
  boardheader * bp;
  boardheader * getbcache ();

  if (currstat == RMAIL)
  {
    move (2, 0); clrtoeol ();
    move (3, 0); clrtoeol ();
    move (1, 0); make_blist ();
    namecomplete ("輸入看版名稱 (直接Enter進入私人信件夾)：", buf);
    if (*buf)
      strcpy (xboard, buf);
    else
      strcpy (xboard, "0");
    if (xboard && (bp = getbcache (xboard)))
    {
      setapath (fpath, xboard);
      setutmpmode (ANNOUNCE);
      a_menu (xboard, fpath, HAS_PERM (PERM_ALLBOARD) ? 2 : is_BM (bp->BM) ? 1 : 0);
    }
    else if(HAS_PERM(PERM_MAILLIMIT) || HAS_PERM(PERM_BM)) // wildcat : 之前忘記加 PERM 限制啦 ^^;
    {
      int mode0 = currutmp->mode;
      int stat0 = currstat;
      sethomeman (buf, cuser.userid);
      sprintf (buf1, "%s 的信件夾", cuser.userid);
      setutmpmode (ANNOUNCE);
      a_menu (buf1, buf, belong ("etc/sysop", cuser.userid) ? 2 : 1);
      currutmp->mode = mode0;
      currstat = stat0;
      return RC_FULL;
    }
  }
  else
  {
    setapath (buf, currboard);
    setutmpmode (ANNOUNCE);
    a_menu (currboard, buf, HAS_PERM (PERM_ALLBOARD) ? 2 :
      currmode & MODE_BOARD ? 1 : 0);
  }
  return RC_FULL;
}

int
cite (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char fpath[512];
  char title[TTLEN + 1];

  if (currstat == RMAIL)
  {
    setuserfile (fpath, fhdr->filename);
    add_tag ();
  }
  else
    setbfile (fpath, currboard, fhdr->filename);

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("不可複製隱藏目錄");
    return RC_NONE;
  }
  strcpy (title, "◇ ");
  strncpy (title + 3, fhdr->title, TTLEN - 3);
  title[TTLEN] = '\0';
//  a_copyitem (fpath, title, fhdr->owner);
  a_copyitem (fpath, title, cuser.userid);
  /* shakalaca.990517: 應使用者要求, 編者為板主 */
  man ();
  return RC_FULL;
}

Cite_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char fpath[512];
  char title[TTLEN + 1];

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("不可複製隱藏目錄");
    return RC_NONE;
  }
  setbfile (fpath, currboard, fhdr->filename);
  sprintf (title, "%s%.72s",(currutmp->pager > 1) ? "" : "◇ ", fhdr->title);
  title[TTLEN] = '\0';
  a_copyitem (fpath, title, cuser.userid);
  load_paste ();
  if (*paste_path)
    a_menu (paste_title, paste_path, paste_level, ANNOUNCE);
  return RC_FULL;
}

Cite_posts (int ent, fileheader * fhdr, char *direct)
{
  char fpath[512];

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("不可複製隱藏目錄");
    return RC_NONE;
  }
  setbfile (fpath, currboard, fhdr->filename);
/* shakalaca.000502: 為什麼要 ??
  add_tag (); */
  load_paste ();
  if (*paste_path && paste_level && dashf (fpath))
  {
    fileheader fh;
    char newpath[MAXPATHLEN];

    strcpy (newpath, paste_path);
    stampfile (newpath, &fh);
/* shakalaca.990714: 將檔案獨立
    unlink (newpath); */
    f_cp (fpath, newpath, O_TRUNC);
    strcpy (fh.owner, cuser.userid);
/* wildcat 000505 : 關 pager 何事? */
    sprintf (fh.title, "%s%.72s","◇ " , fhdr->title);
/*    (currutmp->pager > 1) ? "◇ " : "◇ ", fhdr->title); */
    strcpy (strrchr (newpath, '/') + 1, ".DIR");
    rec_add (newpath, &fh, sizeof (fh));
    return POS_NEXT;
  }
  bell ();
  return RC_NONE;
}

int
edit_title (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[512];

//  if (!strcmp(currboard,"Security")) return RC_NONE;
  if (HAS_PERM (PERM_SYSOP) || (currmode & MODE_BOARD))
  {
    fileheader tmpfhdr = *fhdr;
    int dirty = 0;

    if (getdata (b_lines - 1, 0, "標題：", genbuf, TTLEN, DOECHO, tmpfhdr.title))
    {
      strcpy (tmpfhdr.title, genbuf);
      dirty++;
    }

    if(HAS_PERM (PERM_SYSOP))
    {
      if (getdata (b_lines - 1, 0, "作者：", genbuf, IDLEN + 2, DOECHO, tmpfhdr.owner))
      {
        strcpy (tmpfhdr.owner, genbuf);
        dirty++;
      }

      if (getdata (b_lines - 1, 0, "日期：", genbuf, 6, DOECHO, tmpfhdr.date))
      {
        sprintf (tmpfhdr.date, "%+5s", genbuf);
        dirty++;
      }
    }

    if (getdata (b_lines - 1, 0, "確定(Y/N)?[n] ", genbuf, 3, DOECHO, 0) &&
      (*genbuf == 'y' || *genbuf == 'Y' )&& dirty)
    {
      *fhdr = tmpfhdr;
      substitute_record (direct, fhdr, sizeof (*fhdr), ent);
      if (currmode & MODE_SELECT)
      {
        int now;
        setbdir (genbuf, currboard);
        now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
        substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
      }
    }
    return RC_FULL;
  }
  return RC_NONE;
}

int
add_tag (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  int now;
  char genbuf[100];

  if (!strcmp(currboard,"Security") && !HAS_PERM(PERM_BBSADM)) return RC_NONE;

  if (currstat == RMAIL)
  {
    fhdr->filemode ^= FILE_TAGED;
    sethomedir (genbuf, cuser.userid);
    if (currmode & SELECT)
    {
      now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
      substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
      sprintf (genbuf, "home/%s/SR.%s", cuser.userid, cuser.userid);
      substitute_record (genbuf, fhdr, sizeof (*fhdr), ent);
    }
    else
      substitute_record (genbuf, fhdr, sizeof (*fhdr), ent);
    return POS_NEXT;
  }
//  if(currstat == READING) return RC_NONE;
  if (currmode & MODE_BOARD)
  {
    fhdr->filemode ^= FILE_TAGED;
    if (currmode & MODE_SELECT)
    {
      setbdir (genbuf, currboard);
      now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
      substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
      sprintf (genbuf, "boards/%s/SR.%s", currboard, cuser.userid);
      substitute_record (genbuf, fhdr, sizeof (*fhdr), ent);
      return POS_NEXT;
    }
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);
    return POS_NEXT;
  }
  return RC_NONE;
}


int
del_tag (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[3];
  int number;

  if (currstat == RMAIL)
  {
    getdata (1, 0, "確定刪除標記信件(Y/N)? [Y]", genbuf, 3, LCECHO, 0);
    if (genbuf[0] != 'n')
    {
      currfmode = FILE_TAGED;
      if (delete_files (direct, cmpfmode))
        return RC_CHDIR;
    }
    return RC_FULL;
  }
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    if (!strcmp(currboard,"Security") && !HAS_PERM(PERM_BBSADM)) return RC_NONE;
    getdata (1, 0, "確定刪除標記文章(Y/N)? [N]", genbuf, 3, LCECHO, 0);
    if (genbuf[0] == 'y')
    {
      currfmode = FILE_TAGED;
      if (currmode & MODE_SELECT)
      {
        unlink (direct);
        currmode ^= MODE_SELECT;
        setbdir (direct, currboard);
        delete_files (direct, cmpfmode);
      }
       if (number=delete_files(direct, cmpfmode)){
        log_board3("TAG", currboard, number);
                 return RC_CHDIR;
        }
    }
    return RC_FULL;
  }
  return RC_NONE;
}

int
gem_tag (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[3];

  load_paste(); //讀入 paste_file 來定位
  if(!*paste_path)
  {
    pressanykey("尚未定位,請進入精華區中你想收錄的目錄按 [P]");
    return RC_FOOT;
  }

  if (currstat == RMAIL)
  {
    getdata (1, 0, "確定收錄標記信件(Y/N)? [Y]", genbuf, 3, LCECHO, 0);
    if (genbuf[0] != 'n')
    {
      currfmode = FILE_TAGED;
      if (gem_files (direct, cmpfmode))
        return RC_CHDIR;
    }
    return RC_FULL;
  }
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    getdata (1, 0, "確定收錄標記文章(Y/N)? [N]", genbuf, 3, LCECHO, 0);
    if (genbuf[0] == 'y')
    {
      currfmode = FILE_TAGED;
      if (currmode & MODE_SELECT)
      {
        unlink (direct);
        currmode ^= MODE_SELECT;
        setbdir (direct, currboard);
        gem_files (direct, cmpfmode);
      }
      else
        gem_files(direct, cmpfmode);
      return RC_CHDIR;
    }
    return RC_FULL;
  }
  return RC_NONE;
}


int
mark (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
/*
  if (currstat == RMAIL)
  {
    fhdr->filemode ^= FILE_MARKED;
    fhdr->filemode &= ~FILE_TAGED;
    sethomedir (currmaildir, cuser.userid);
    substitute_record (currmaildir, fhdr, sizeof (*fhdr), ent);
    return (RC_DRAW);
  }
*/
  if (currstat == READING && !(currmode & MODE_BOARD))
    return RC_NONE;

  fhdr->filemode ^= FILE_MARKED;

  if (currmode & MODE_SELECT)
  {
    int now;
    char genbuf[100];

    if (currstat != READING)
      sethomedir(genbuf, cuser.userid);
    else
      setbdir (genbuf, currboard);
    now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
    substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
    substitute_record(direct, fhdr, sizeof(*fhdr), ent);
  }
  else
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);

  return RC_DRAW;
}

int
score (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char buf[128];
  time_t now = time(0);

  if (currstat == RMAIL || ((cuser.scoretimes <= 0 || currbrdattr & BRD_PERSONAL) && !HAS_PERM(PERM_SYSOP)) || !strcmp("guest",cuser.userid))
    return RC_NONE;

  getdata (b_lines, 0, "請問要 [1]加分 [2]扣分 ? ", buf, 2, LCECHO, 0);
  if(!buf[0] || buf[0] < '1' || buf[0] > '2')
    return RC_DRAW;
  else if(buf[0] == '1')
  {
    if(fhdr->score >= 99)
    {
      pressanykey("已經是最高分了!!");
      return RC_FULL;
    }
    fhdr->score++;
  }
  else if(buf[0] == '2')
  {
    if(fhdr->score <= -9)
    {
      pressanykey("已經是最低分了!!");
      return RC_FULL;
    }
    fhdr->score--;
  }
  if(!HAS_PERM(PERM_SYSOP))
    cuser.scoretimes--;
  if (currmode & MODE_SELECT)
  {
    int now;
    char genbuf[100];

    if (currstat != READING)
      sethomedir(genbuf, cuser.userid);
    else
      setbdir (genbuf, currboard);
    now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
    substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
  }
  else
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);

  substitute_record (fn_passwd, &cuser, sizeof (userec), usernum);
  sprintf(buf , "%-12.12s 替 %-12.12s 板 [%-40.40s] 評分 [%s] %s", 
    cuser.userid,currboard,fhdr->title,buf[0] == '1' ? "+1" : "-1",Etime(&now));
  f_cat(BBSHOME"/log/article_score.log",buf);
  if(!HAS_PERM(PERM_SYSOP))
  {
    sprintf(buf , "你的評分次數還有 %d 次 ...", cuser.scoretimes);
    pressanykey(buf);
  }
  return RC_DRAW;
}

int
del_range (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char num1[8], num2[8];
  int inum1, inum2;

  if (!strcmp(currboard,"Security")) return RC_NONE;
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    getdata (1, 0, "[設定刪除範圍] 起點：", num1, 5, DOECHO, 0);
    inum1 = atoi (num1);
    if (inum1 <= 0)
    {
      outz ("起點有誤");
      return RC_FOOT;
    }
    getdata (1, 28, "終點：", num2, 5, DOECHO, 0);
    inum2 = atoi (num2);
    if (inum2 < inum1)
    {
      outz ("終點有誤");
      return RC_FULL;
    }
    getdata (1, 48, msg_sure_ny, num1, 3, LCECHO, 0);
    if (*num1 == 'y')
    {
      outmsg ("處理中,請稍後...");
      refresh ();
      if (currmode & MODE_SELECT)
      {
        int fd, size = sizeof (fileheader);
        char genbuf[100];
        fileheader rsfh;
        int i = inum1, now;
        if (currstat == RMAIL)
          sethomedir (genbuf, cuser.userid);
        else
          setbdir (genbuf, currboard);
        if ((fd = (open (direct, O_RDONLY, 0))) != -1)
        {
          if (lseek (fd, (off_t) (size * (inum1 - 1)), SEEK_SET) != -1)
          {
            while (read (fd, &rsfh, size) == size)
            {
              if (i > inum2)
                break;
              now = getindex (genbuf, rsfh.filename, size);
              strcpy (currfile, rsfh.filename);
              if (!(rsfh.filemode & FILE_MARKED))
                delete_file (genbuf, sizeof (fileheader), now, cmpfilename);
              i++;
            }
          }
          close (fd);
        }
      }
      delete_range (direct, inum1, inum2);
//      fixkeep (direct, inum1);
      return RC_NEWDIR;
    }
    return RC_FULL;
  }
  return RC_NONE;
}

#if 0
static void
lazy_delete (fhdr)
  fileheader * fhdr;
{
  char buf[20];

  sprintf (buf, "-%s", fhdr->owner);
  strncpy (fhdr->owner, buf, IDLEN + 1);
  strcpy (fhdr->title, "<< article deleted >>");
  fhdr->savemode = 'L';
}

int
del_one (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    strcpy (currfile, fhdr->filename);

    if (!update_file (direct, sizeof (fileheader), ent, cmpfilename, lazy_delete))
    {
      cancelpost (fhdr, YEA);
      lazy_delete (fhdr);
      return RC_DRAW;
    }
  }
  return RC_NONE;
}
#endif

static int
del_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  int not_owned;
  char genbuf[100];

  if (!strcmp(currboard,"Security")) return RC_NONE;
  if ((fhdr->filemode & FILE_MARKED)
    || (fhdr->filemode & FILE_DIGEST) || (fhdr->owner[0] == '-'))
    return RC_NONE;

  not_owned = strcmp (fhdr->owner, cuser.userid);
// wildcat : 站長可以連線砍信
  if(HAS_PERM(PERM_SYSOP) && answer("是否要連線砍信(y/N)") == 'y')
    not_owned = 0;

  if (!(currmode & MODE_BOARD) && not_owned || !strcmp (cuser.userid, "guest"))
    return RC_NONE;

  getdata (1, 0, msg_del_ny, genbuf, 3, LCECHO, 0);
  if (genbuf[0] == 'y')
  {
    strcpy (currfile, fhdr->filename);
    setbfile (genbuf, currboard, fhdr->filename);
    if (!delete_file (direct, sizeof (fileheader), ent, cmpfilename))
    {
      if (currmode & MODE_SELECT)
      {
        int now;

        setbdir (genbuf, currboard);
        now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
        delete_file (genbuf, sizeof (fileheader), now, cmpfilename);
      }
      cancelpost (fhdr, not_owned);
      if (!not_owned && !(currbrdattr & BRD_NOCOUNT) && !HAS_PERM(PERM_SYSOP))
      {
        UPDATE_USEREC;
        move (b_lines - 1, 0);
        clrtoeol ();
        pressanykey ("%s，您的文章減為 %d 篇", msg_del_ok,
          cuser.numposts > 0 ? --cuser.numposts : cuser.numposts);
        substitute_record (fn_passwd, &cuser, sizeof (userec), usernum);
      }
      return RC_CHDIR;
    }
  }
  return RC_FULL;
}


save_mail (int ent, fileheader * fh, char *direct)
{
  fileheader mhdr;
  char fpath[MAXPATHLEN];
  char genbuf[MAXPATHLEN];
  char *p;

  if (ent < 0)
    strcpy (fpath, direct);
  else
  {
    strcpy (genbuf, direct);
    if (p = strrchr (genbuf, '/'))
      * p = '\0';
    sprintf (fpath, "%s/%s", genbuf, fh->filename);
  }
  if (!dashf (fpath) || !HAS_PERM (PERM_BASIC))
  {
    bell ();
    return RC_NONE;
  }
  sethomepath (genbuf, cuser.userid);
  stampfile (genbuf, &mhdr);
  unlink (genbuf);
  f_cp (fpath, genbuf, O_TRUNC);
  if (HAS_PERM (PERM_SYSOP))
    strcpy (mhdr.owner, fh->owner);
  else
    strcpy (mhdr.owner, cuser.userid);
  strncpy (mhdr.title, fh->title + ((currstat == ANNOUNCE) ? 3 : 0), TTLEN);
  mhdr.savemode = '\0';
  mhdr.filemode = FILE_READ;
  sethomedir (fpath, cuser.userid);
  if (rec_add (fpath, &mhdr, sizeof (mhdr)) == -1)
  {
    bell ();
    return RC_NONE;
  }
  return POS_NEXT;
}

/* ----------------------------------------------------- */
/* 看板備忘錄、文摘、精華區                              */
/* ----------------------------------------------------- */

static int
board_edit ()
{
  boardheader bp;
  boardheader * getbcache ();
  int bid, mode = 0;
  time_t now = time(0);

  if (currmode & MODE_BOARD)
  {
    char genbuf[BTLEN], buf[512], ans;
    bid = getbnum (currboard);

    if (rec_get (fn_board, &bp, sizeof (boardheader), bid) == -1)
    {
      pressanykey (err_bid);
      return -1;
    }

    if (bp.brdattr & BRD_PERSONAL || HAS_PERM(PERM_SYSOP))
      mode = 1;

    sprintf(buf, "看板 1)改中文名稱 2)看板說明 3)進版畫面 4)看板名單 5)設密碼 %s:",
      "6)進階");
//      mode ? "7)進階" : "");

    getdata (1, 0, buf, genbuf, 2, DOECHO, 0);
    switch(genbuf[0])
    {
      case '1':
        move (1, 0);
        clrtoeol ();
        getdata (1, 0, "請輸入看板新中文敘述:"
          ,genbuf, BTLEN - 16, DOECHO, bp.title + 7);
        if (!genbuf[0]) return 0;
        strip_ansi (genbuf, genbuf, 0);
        if(strcmp(bp.title+7,genbuf))
        {
          sprintf(buf,"%-13.13s 更換看板 %s 敘述 [%s] -> [%s] , %s",
            cuser.userid,bp.brdname,bp.title+7,genbuf,ctime(&now));
          f_cat(BBSHOME"/log/board_edit",buf);
          log_usies ("NameBoard", currboard);
          strcpy (bp.title + 7, genbuf);
        }
        break;

      case '2':
        clrchyiuan(1, 5);
        move(1, 0);
        outs("對本看板的描述 (共三行) :");
        getdata (2, 0, ":", bp.desc[0], 79, DOECHO, bp.desc[0]);
        getdata (3, 0, ":", bp.desc[1], 79, DOECHO, bp.desc[1]);
        getdata (4, 0, ":", bp.desc[2], 79, DOECHO, bp.desc[2]);
        sprintf(buf,"%-13.13s 更換看板 %s 說明 , %s",
          cuser.userid,bp.brdname,ctime(&now));
        f_cat(BBSHOME"/log/board_edit",buf);
        log_usies ("SetBoardDesc", currboard);
        break;

      case '3':
        setbfile (buf, currboard, fn_notes);
        if (vedit (buf, NA) == -1)
          pressanykey (msg_cancel);
        else
        {
          int aborted;

          getdata (3, 0, "請設定有效期限(0 - 9999)天？", buf, 5, DOECHO, "9999");
          aborted = atol (buf);
          bp.bupdate = aborted ? time (0) + aborted * 86400 : 0;
        }
        break;

      case '4':
        setbfile(buf, currboard, FN_LIST);
        ListEdit(buf);
        return RC_FULL;

      case '5':
      {
        char genbuf[PASSLEN+1],buf[PASSLEN+1];

        move (1, 0);
        clrtoeol ();
        if(!HAS_PERM(PERM_ALLBOARD))
        {
          if(!getdata (1, 0, "請輸入原本的密碼" ,genbuf, PASSLEN, PASS, 0) ||
             !chkpasswd(bp.passwd, genbuf))
             {
               pressanykey("密碼錯誤");
               return -1;
             }
        }
        if (!getdata(1, 0, "請設定新密碼：", genbuf, PASSLEN, PASS,0))
        {
          pressanykey("密碼設定取消, 繼續使用舊密碼");
          return -1;
        }
        strncpy(buf, genbuf, PASSLEN);

        getdata(1, 0, "請檢查新密碼：", genbuf, PASSLEN, PASS,0);
        if (strncmp(genbuf, buf, PASSLEN))
        {
          pressanykey("新密碼確認失敗, 無法設定新密碼");
          return -1;
        }
        buf[8] = '\0';
        strncpy(bp.passwd, genpasswd(buf), PASSLEN);
        log_usies ("SetBrdPass", currboard);
      }
      break;

      case '6':
        getdata(1, 0, "您要 1)改屬性 2)改類別 3)看今日記錄 4)文章類別 5)備份精華區:", genbuf, 2, DOECHO, 0);
        switch(genbuf[0])
        {
          case '1':
          {
            int oldattr=bp.brdattr;

            if (!mode)
              break;
            ans = answer("看板狀態更改為 (o)開放 (p)私人 (h)隱藏 (i)邀請?");
            if(ans == 'p')
            {
              bp.brdattr &= ~BRD_POSTMASK;
              bp.brdattr |= BRD_HIDE;
            }
            else if(ans == 'h')
            {
              bp.brdattr |= BRD_POSTMASK;
              bp.brdattr |= BRD_HIDE;
            }
            else
            {
              if(answer("是否要讓 guest 看到你的板? (Y/n)") == 'n')
                bp.brdattr &= ~BRD_POSTMASK;
              else
                bp.brdattr |= BRD_POSTMASK;
              bp.brdattr &= ~BRD_HIDE;
              if(ans == 'i')
                bp.brdattr |= BRD_INVITE;
              else
                bp.brdattr &= ~BRD_INVITE;
            }
            if(bp.brdattr != oldattr)
            {
              sprintf(buf,"%-13.13s 更改看板 [%s] 之屬性為 %s , %s",
                cuser.userid,bp.brdname,
                ans == 'p' ? "私人" : ans == 'h' ? "隱藏" : ans == 'o' ? "邀請" : "開放",ctime(&now));
              f_cat(BBSHOME"/log/board_edit",buf);
              log_usies("ATTR_Board",currboard);
            }
          }
          break;

          case '2':
            if (!mode)
              break;
            move (1, 0);
            clrtoeol ();
            getdata (1, 0, "請輸入看板新類別:",genbuf, 5, DOECHO, bp.title );
            if (!genbuf[0]) return 0;
            strip_ansi (genbuf, genbuf, 0);
            if(strncmp(bp.title,genbuf,4))
            {
              sprintf(buf,"%-13.13s 更換看板 %s 類別 [%-4.4s] -> [%s] , %s",
                cuser.userid,bp.brdname,bp.title,genbuf,ctime(&now));
              f_cat(BBSHOME"/log/board_edit",buf);
              log_usies("PREFIX",currboard);
              strncpy (bp.title , genbuf, 4);
            }
            break;
          case '3':
            if (!mode)
              break;
            sprintf(buf,"/usr/bin/grep \"USE %s\" %s/usboard > %s/tmp/usboard.%s",
              currboard, BBSHOME, BBSHOME, currboard);
            system(buf);
            sprintf(buf,BBSHOME"/tmp/usboard.%s",currboard);
            more(buf, YEA);
            log_usies("BOARDLOG", currboard);
            break;
          case '4':
            sprintf(buf,BBSHOME"/boards/%s/prefix",currboard);
            vedit(buf,0);
            break;
          case '5':
          {
            char cmd[100];
            fileheader fhdr;
            sprintf(cmd, "/tmp/%s.tgz", currboard);
            sprintf(fhdr.title, "看板 [%s] 精華區", currboard);
            doforward(cmd, &fhdr, 'M');
          }

          default:
            break;
        }
        goto end;

      default:
        pressanykey("放棄修改");
        break;
    }
end:
    substitute_record (fn_board, &bp, sizeof (boardheader), bid);
    touch_boards ();
    return RC_FULL;
  }
  return 0;
}


/* wildcat modify 981221 */
int
b_notes ()
{
  char buf[64];

  setbfile (buf, currboard, fn_notes);
  if (more (buf, YEA) == -1) // 可以看多頁
  {
    clear ();
    pressanykey ("本看板尚無「備忘錄」。");
  }
  return RC_FULL;
}


int
board_select ()
{
  struct stat st;
  char fpath[80];
  char genbuf[100];
  currmode &= ~(MODE_SELECT | MODE_TINLIKE);

  sprintf (genbuf, "SR.%s", cuser.userid);
  setbfile (fpath, currboard, genbuf);
  unlink (fpath);

  /* shakalaca.000112: 超過 30min 才將 index 刪除, 作 cache 用 */
  setbfile (fpath, currboard, "SR.thread");
  if (stat(fpath, &st) == 0 && st.st_mtime < time(0) - 60 * 30)
    unlink (fpath);

  if (currstat == RMAIL)
    sethomedir (currdirect, cuser.userid);
  else
    setbdir (currdirect, currboard);

  return RC_NEWDIR;
}


int
board_digest ()
{
  if (currmode & MODE_SELECT)
    board_select ();

  currmode ^= MODE_DIGEST;
  if (currmode & MODE_DIGEST)
    currmode &= ~MODE_POST;
  else if (haspostperm (currboard))
    currmode |= MODE_POST;

  setbdir (currdirect, currboard);
  return RC_NEWDIR;
}


static int
good_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[512];
  char genbuf2[512];
  fileheader digest;

  memcpy (&digest, fhdr, sizeof (digest));
  digest.filename[0] = 'G';

  if ((currmode & MODE_DIGEST) || !(currmode & MODE_BOARD))
    return RC_NONE;

  if (fhdr->filemode & FILE_DIGEST)
  {
    int now;
    setbgdir (genbuf2, currboard);
    now = getindex (genbuf2, digest.filename);
    strcpy (currfile, digest.filename);
    delete_file (genbuf2, sizeof (fileheader), now, cmpfilename);
    sprintf (genbuf2, BBSHOME "/boards/%s/%s", currboard, currfile);
    unlink (genbuf2);
    fhdr->filemode = (fhdr->filemode & ~FILE_DIGEST);
  }
  else
  {
    char *ptr, buf[64];
    strcpy (buf, direct);
    ptr = strrchr (buf, '/') + 1;
    ptr[0] = '\0';
    sprintf (genbuf, "%s%s", buf, digest.filename);
    if (!dashf (genbuf))
    {
      digest.savemode = digest.filemode = 0;
      sprintf (genbuf2, "%s%s", buf, fhdr->filename);
      f_cp (genbuf2, genbuf, O_TRUNC);
      strcpy (ptr, fn_mandex);
      rec_add (buf, &digest, sizeof (digest));
    }
    fhdr->filemode = (fhdr->filemode & ~FILE_MARKED) | FILE_DIGEST;
  }
  substitute_record (direct, fhdr, sizeof (*fhdr), ent);
  if (currmode & MODE_SELECT)
  {
    int now;
    char genbuf[100];
    setbdir (genbuf, currboard);
    now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
    substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
  }
  return RC_DRAW;
}

int
go_chat()
{
  DL_func("SO/chat.so:t_chat");
  return RC_FULL;
}

/* ----------------------------------------------------- */
/* 看板功能表                                            */
/* ----------------------------------------------------- */

struct one_key read_comms[] =
{
  KEY_TAB, board_digest,    /* 進入/退出 文摘 */
  'b', b_notes,         /* 看進版畫面 */
  'c', cite,            /* 收錄精華 */
  'r', read_post,       /* 閱讀文章 */
  'z', man,         /* 進入精華區 */
  'D', del_range,       /* 大 D 砍信 */
  Ctrl ('S'), save_mail,    /* 存入信箱 */
  'E', edit_post,       /* 修改文章 */
  'T', edit_title,      /* 修改標題 */
  's', do_select,       /* 選擇看板 */
  'B', board_edit,      /* 看板編輯 */

#if 0
  'i', bh_passwd_edit,      /* 設定看板密碼 */
  'W', b_notes_edit,        /* 標輯進版畫面 */
  'o', b_list_edit,     /* 編輯看板可見名單 */
#endif

  't', add_tag,         /* 標記文章 */
  Ctrl ('D'), del_tag,      /* 刪除標記文章 */
  'x', cross_post,      /* 轉貼 */
  'g', good_post,       /* 收到文摘中 */
  'y', reply_post,      /* 回覆文章 */
  'd', del_post,        /* 刪除文章 */
  'm', mark,            /* Mark 文章 */
  Ctrl ('P'), do_post,      /* 發表文章 */
  'C', gem_tag,         /* 收錄標記文章 */
  Ctrl ('C'), Cite_posts,   /* 直接收錄文章至精華區 */
  '%', score,		/* 文章評分 */
  '#', go_chat,
  '\0', NULL
};

void
Read ()
{
  int mode0 = currutmp->mode;
  int currmode0 = currmode;
  int stat0 = currstat;
  int bid;
  char buf[40];
  time_t startime = time (0);
  extern struct BCACHE * brdshm;

  resolve_boards ();
  setutmpmode (READING);
  set_board ();
  if (board_visit_time < board_note_time)
  {
    setbfile (buf, currboard, fn_notes);
    more (buf, YEA);
  }
  bid = getbnum (currboard);
  currutmp->brc_id = bid;
  if(Ben_Perm(&brdshm->bcache[bid]-1) != 1) 
  {
    pressanykey(P_BOARD);
    return;
  }
  setbdir (buf, currboard);
  curredit &= ~EDIT_MAIL;
  i_read (READING, buf, readtitle, doent, read_comms, &(brdshm->total[bid - 1]));

  log_board (currboard, time (0) - startime);
  log_board2 (currboard, time (0) - startime);
  brc_update ();

  currutmp->brc_id = 0;
  currutmp->mode = mode0;
  currstat = stat0;
  currmode = currmode0;
//  return;
}


void
ReadSelect ()
{
/*
  int mode0 = currutmp->mode;
  int stat0 = currstat;
  char genbuf[512];

  currstat = XMODE;
*/
  if (do_select (0, 0, NULL) == RC_NEWDIR)
    Read ();
/*
  currutmp->mode = mode0;
  currstat = stat0;
*/
}


int
Select ()
{
  char genbuf[512];

  setutmpmode (SELECT);
  do_select (0, NULL, genbuf);
  return 0;
}


int
Post ()
{
  do_post ();
  return 0;
}

void
cancel_post(fhdr, fpath)
  fileheader *fhdr;
  char *fpath;
{
  int fd;

  if ((fhdr->savemode == 'S') &&/* 外轉信件 */
    ((fd = open(fpath, O_RDONLY)) >= 0))
  {
#define NICK_LEN    80

    char *ptr, *left, *right, nick[NICK_LEN];
    FILE *fout;
    int ch;

    ptr = nick;
    ch = read(fd, ptr, NICK_LEN);
    close(fd);
    ptr[ch] = '\0';
    if (!strncmp(ptr, str_author1, LEN_AUTHOR1) ||
      !strncmp(ptr, str_author2, LEN_AUTHOR2))
    {
      if (left = (char *) strchr(ptr, '('))
      {
    right = NULL;
    for (ptr = ++left; ch = *ptr; ptr++)
    {
      if (ch == ')')
        right = ptr;
      else if (ch == '\n')
        break;
    }

    if (right != NULL)
    {
      *right = '\0';
          log_board3("DEL", currboard, 1);

      if (fout = fopen(BBSHOME"/innd/cancel.bntp", "a"))
      {
        fprintf(fout, "%s\t%s\t%s\t%s\t%s\n",
          currboard, fhdr->filename, fhdr->owner    /* cuser.userid */
          ,left, fhdr->title);
        fclose(fout);
      }
    }
      }
    }
#undef  NICK_LEN
  }
}

/* ----------------------------------------------------- */
/* 離開 BBS 站                                           */
/* ----------------------------------------------------- */


void
note()
{
  static char *fn_note_tmp = "note.tmp";
  static char *fn_note_dat = "note.dat";
  int total, i, collect, len;
  struct stat st;
  char buf[512], buf2[512];
  int fd, fx;
  FILE *fp, *foo;
  struct notedata
  {
    time_t date;
    char userid[IDLEN + 1];
    char username[19];
    char buf[3][80];
  };
  struct notedata myitem;
  setutmpmode(EDNOTE);
  myitem.buf[0][0] = myitem.buf[1][0] = myitem.buf[2][0] = '\0';
  do
  {
    move(12, 0);
    clrtobot();
    outs("\n請留言 (至多三行)，按[Enter]結束");
    for (i = 0; (i < 3) &&
      getdata(16 + i, 0, "：", myitem.buf[i], 70, DOECHO,0); i++);
    getdata(b_lines - 1, 0, "(S)儲存 (E)重新來過 (Q)取消？[S] ", buf, 3, LCECHO,"S");
/*
woju
*/
    if (buf[0] == 'q' || i == 0 && *buf != 'e')
      return;
  } while (buf[0] == 'e');
  strcpy(myitem.userid, cuser.userid);
  strncpy(myitem.username, cuser.username, 18);
  myitem.username[18] = '\0';
  time(&(myitem.date));

  /* begin load file */

  if ((foo = fopen(BBSHOME"/.note", "a")) == NULL)
    return;

  if ((fp = fopen(fn_note_ans, "w")) == NULL)
    return;

  if ((fx = open(fn_note_tmp, O_WRONLY | O_CREAT, 0644)) <= 0)
    return;

  if ((fd = open(fn_note_dat, O_RDONLY)) == -1)
  {
    total = 1;
  }
  else if (fstat(fd, &st) != -1)
  {
    total = st.st_size / sizeof(struct notedata) + 1;
    if (total > MAX_NOTE)
      total = MAX_NOTE;
  }

  fputs("\x1b[1m                             "COLOR1" \x1b[33m◆ \x1b[37m心 情 留 言 板 \x1b[33m◆ \x1b[m \n\n",fp);
  collect = 1;

  while (total)
  {
    sprintf(buf, "\x1b[46m\x1b[1;34m┌┴ \x1b[33m%s\x1b[37m(%s)",
      myitem.userid, myitem.username);
    len = strlen(buf);
    strcat(buf," \x1b[34m" + (len&1));

    for (i = len >> 1; i < 38; i++)
      strcat(buf, "─");
    sprintf(buf2, "─\x1b[33m %.14s  \x1b[34m┴┐\x1b[m\n",
      Etime(&(myitem.date)));
    strcat(buf, buf2);
    fputs(buf, fp);

    if (collect)
      fputs(buf, foo);

    sprintf(buf, "\x1b[1;46m\x1b[34m└┐\x1b[37m%-70s\x1b[34m┌┘\x1b[m\n",myitem.buf[0]);
    if(*myitem.buf[1])
    {
      sprintf(buf2, "  \x1b[1;46m\x1b[34m│\x1b[37m%-70s\x1b[34m│\x1b[m\n",myitem.buf[1]);
      strcat(buf, buf2);
    }
    if(*myitem.buf[2])
    {
      sprintf(buf2, "  \x1b[1;46m\x1b[34m│\x1b[37m%-70s\x1b[34m│\x1b[m\n",myitem.buf[2]);
      strcat(buf, buf2);
    }
    fputs(buf,fp);
    if (collect)
    {
      fputs(buf, foo);
      fclose(foo);
      collect = 0;
    }
    write(fx, &myitem, sizeof(myitem));

    if (--total)
      read(fd, (char *) &myitem, sizeof(myitem));
  }
  fputs("  \033[1;34;46m└───────────────────────────────────┘\033[m", fp);
  fclose(fp);
  close(fd);
  close(fx);
  f_mv(fn_note_tmp, fn_note_dat);
  more(fn_note_ans, YEA);
}


int
m_sysop()
{
  FILE *fp;
  char genbuf[512];

  setutmpmode(MSYSOP);
  if (fp = fopen(BBSHOME"/etc/sysop", "r"))
  {
    int i, j;
    char *ptr;

    struct SYSOPLIST
    {
      char userid[IDLEN + 1];
      char duty[40];
    }         sysoplist[9];

    j = 0;
    while (fgets(genbuf, 128, fp))
    {
      if (ptr = strchr(genbuf, '\n'))
      {
        *ptr = '\0';
        ptr = genbuf;
        while (isalnum(*ptr))
           ptr++;
        if (*ptr)
        {
          *ptr = '\0';
          do
          {
            i = *++ptr;
          } while (i == ' ' || i == '\t');
          if (i)
          {
            strcpy(sysoplist[j].userid, genbuf);
            strcpy(sysoplist[j++].duty, ptr);
          }
        }
      }
    }


    move(12, 0);
    clrtobot();
    prints("%16s   %-18s權責劃分\n\n", "編號", "站長 ID"/*, msg_seperator*/);

    for (i = 0; i < j; i++)
    {
      prints("%15d.   \x1b[1;%dm%-16s%s\x1b[0m\n",
        i + 1, 31 + i % 7, sysoplist[i].userid, sysoplist[i].duty);
    }
    prints("%-14s0.   \x1b[1;%dm離開\x1b[0m", "", 31 + j % 7);
    getdata(b_lines - 1, 0, "                   請輸入代碼[0]：", genbuf, 4, DOECHO,"1");
    i = genbuf[0] - '0' - 1;
    if (i >= 0 && i < j)
    {
      clear();
      do_send(sysoplist[i].userid, NULL);
    }
  }
  return 0;
}


int
Goodbye()
{
  if (getans("您確定要離開【 " BOARDNAME " 】嗎(Y/N)？[N] ") != 'y')
    return 0;

  movie(999);
  if (cuser.userlevel)
  {
    char ans;

    ans = getans("(G)隨風而逝 (M)托夢站長 (N)我要吶喊 [G] ");
    if (ans == 'm')
      m_sysop();
    else if (ans == 'n')
      note();
  }

  t_display();
  clear();
  prints("\x1b[1;31m親愛的 \x1b[31m%s(\x1b[37m%s)\x1b[31m，別忘了再度光臨"COLOR1
    " %s \x1b[40;33m！\n\n以下是您在站內的註冊資料:\x1b[m\n",
    cuser.userid, cuser.username, BoardName);
  user_display(&cuser, 0);
  pressanykey(NULL);

  more(BBSHOME"/etc/Logout",NA);
  pressanykey(NULL);
  if (currmode)
    u_exit("EXIT ");
  reset_tty();
  exit(0);
}
