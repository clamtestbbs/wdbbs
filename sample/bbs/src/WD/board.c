/*-------------------------------------------------------*/
/* board.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : ¬ÝªO¡B¸s²Õ¥\¯à                               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"


#define BRC_MAXSIZE     24576
#define BRC_MAXNUM      80
#define BRC_ITEMSIZE    (BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))

#define         MAX_FAVORITE    64      /* ­­©w³Ì¦h¥i¥H¦³¦h¤Ö­Ó§Úªº³Ì·R */

int page_lines = 15;

int brc_size, brc_changed = 0;
int brc_list[BRC_MAXNUM], brc_num;

char brc_buf[BRC_MAXSIZE];
char brc_name[BRC_STRLEN];

static time_t brc_expire_time;


extern int numboards;
extern boardheader *bcache;

typedef struct
{
  int pos, total;
  char *name, *title, *BM, desc[3][80];
  uschar unread, zap, bvote;
  usint brdattr;
}      boardstat;

boardstat *nbrd;

int *zapbuf;
int brdnum, yank_flag = 0;

char *boardprefix;

static char *str_local_board = "¡¸¡³¡º¡·¡µ";  /* ¥Nªí local board class */
static char *str_good_board = "¡¸¡¹";  /* ¥Nªí good board class */

/* ----------------------------------------------------- */
/* home/userid/.boardrc maintain                         */
/* ----------------------------------------------------- */

static char *fn_boardrc = ".boardrc";

static char *
brc_getrecord(ptr, name, pnum, list)
  char *ptr, *name;
  int *pnum, *list;
{
  int num;
  char *tmp;

  strncpy(name, ptr, BRC_STRLEN);
  ptr += BRC_STRLEN;
  num = (*ptr++) & 0xff;
  tmp = ptr + num * sizeof(int);
  if (num > BRC_MAXNUM)
    num = BRC_MAXNUM;
  *pnum = num;
  memcpy(list, ptr, num * sizeof(int));
  return tmp;
}


static char *
brc_putrecord(ptr, name, num, list)
  char *ptr, *name;
  int num, *list;
{
  if (num > 0 && list[0] > brc_expire_time)
  {
    if (num > BRC_MAXNUM)
      num = BRC_MAXNUM;

    while (num > 1 && list[num - 1] < brc_expire_time)
      num--;

    strncpy(ptr, name, BRC_STRLEN);
    ptr += BRC_STRLEN;
    *ptr++ = num;
    memcpy(ptr, list, num * sizeof(int));
    ptr += num * sizeof(int);
  }
  return ptr;
}


void
brc_update()
{
  if (brc_changed && cuser.userlevel)
  {
    char dirfile[STRLEN], *ptr;
    char tmp_buf[BRC_MAXSIZE - BRC_ITEMSIZE], *tmp;
    char tmp_name[BRC_STRLEN];
    int tmp_list[BRC_MAXNUM], tmp_num;
    int fd, tmp_size;

    ptr = brc_buf;
    if (brc_num > 0)
      ptr = brc_putrecord(ptr, brc_name, brc_num, brc_list);

    setuserfile(dirfile, fn_boardrc);
    if ((fd = open(dirfile, O_RDONLY)) != -1)
    {
      tmp_size = read(fd, tmp_buf, sizeof(tmp_buf));
      close(fd);
    }
    else
    {
      tmp_size = 0;
    }

    tmp = tmp_buf;
    while (tmp < &tmp_buf[tmp_size] && (*tmp >= ' ' && *tmp <= 'z'))
    {
      tmp = brc_getrecord(tmp, tmp_name, &tmp_num, tmp_list);
      if (strncmp(tmp_name, currboard, BRC_STRLEN))
        ptr = brc_putrecord(ptr, tmp_name, tmp_num, tmp_list);
    }
    brc_size = (int) (ptr - brc_buf);

    if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1)
    {
      ftruncate(fd, 0);
      write(fd, brc_buf, brc_size);
      close(fd);
    }
    brc_changed = 0;
  }
}

void
read_brc_buf()
{
  char dirfile[STRLEN];
  int fd;

  if (brc_buf[0] == '\0')
  {
    setuserfile(dirfile, fn_boardrc);
    if ((fd = open(dirfile, O_RDONLY)) != -1)
    {
      brc_size = read(fd, brc_buf, sizeof(brc_buf));
      close(fd);
    }
    else
    {
      brc_size = 0;
    }
  }
}


int
brc_initial(boardname)
  char *boardname;
{
  char *ptr;

  if (strcmp(currboard, boardname) == 0)
  {
    return brc_num;
  }
  brc_update();
  strcpy(currboard, boardname);
  currbrdattr = bcache[getbnum(currboard)-1].brdattr;
  read_brc_buf();

  ptr = brc_buf;
  while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z'))
  {
    ptr = brc_getrecord(ptr, brc_name, &brc_num, brc_list);
    if (strncmp(brc_name, currboard, BRC_STRLEN) == 0)
      return brc_num;
  }
  strncpy(brc_name, boardname, BRC_STRLEN);
  brc_num = brc_list[0] = 1;
  return 0;
}


void
brc_addlist(fname)
  char *fname;
{
  int ftime, n, i;

  if (!cuser.userlevel)
    return;

  ftime = atoi(&fname[2]);
  if (ftime <= brc_expire_time
     /* || fname[0] != 'M' || fname[1] != '.' */ )
  {
    return;
  }
  if (brc_num <= 0)
  {
    brc_list[brc_num++] = ftime;
    brc_changed = 1;
    return;
  }
  if ((brc_num == 1) && (ftime < brc_list[0]))
    return;
  for (n = 0; n < brc_num; n++)
  {
    if (ftime == brc_list[n])
    {
      return;
    }
    else if (ftime > brc_list[n])
    {
      if (brc_num < BRC_MAXNUM)
        brc_num++;
      for (i = brc_num - 1; --i >= n; brc_list[i + 1] = brc_list[i]);
      brc_list[n] = ftime;
      brc_changed = 1;
      return;
    }
  }
  if (brc_num < BRC_MAXNUM)
  {
    brc_list[brc_num++] = ftime;
    brc_changed = 1;
  }
}


int
brc_unread(fname)
  char *fname;
{
  int ftime, n;

  ftime = atoi(&fname[2]);
  if (ftime <= brc_expire_time
     /* || fname[0] != 'M' || fname[1] != '.' */ )
    return 0;
  if (brc_num <= 0)
    return 1;
  for (n = 0; n < brc_num; n++)
  {
    if (ftime > brc_list[n])
      return 1;
    else if (ftime == brc_list[n])
      return 0;
  }
  return 0;
}


/* ----------------------------------------------------- */
/* .bbsrc processing                                     */
/* ----------------------------------------------------- */

char *str_bbsrc = ".bbsrc";

static void
load_zapbuf()
{
  register int n, size;
  char fname[60];

  /* MAXBOARDS ==> ¦Ü¦h¬Ý±o¨£ 4 ­Ó·sªO */

  n = numboards + 4;
  size = n * sizeof(int);
  zapbuf = (int *) malloc(size);
  while (n)
    zapbuf[--n] = login_start_time;
  setuserfile(fname, str_bbsrc);
  if ((n = open(fname, O_RDONLY, 0600)) != -1)
  {
    read(n, zapbuf, size);
    close(n);
  }
  if (!nbrd)
    nbrd = (boardstat *) malloc(MAXBOARD * sizeof(boardstat));
  brc_expire_time = login_start_time - 365 * 86400;
}


static void
save_zapbuf()
{
  register int fd, size;
  char fname[60];

  setuserfile(fname, str_bbsrc);
  if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1)
  {
    size = numboards * sizeof(int);
    write(fd, zapbuf, size);
    close(fd);
  }
}

/*
woju
Ref: bbs.c: brdperm(char* brdname, char* userid)
*/

int
Ben_Perm(bptr)
  boardheader *bptr;
{
  register int level,brdattr;
  register char *ptr;
  char buf[64];

  level = bptr->level;
  brdattr = bptr->brdattr;

  if (HAS_PERM(PERM_BBSADM))
    return 1;

  ptr = bptr->BM;
  if (is_BM(ptr))
    return 1;

  /* ¯¦±K¬ÝªO */

  setbfile(buf, bptr->brdname, FN_LIST);
  if (brdattr & BRD_HIDE) /* ¨p¤H/ÁôÂÃ */
  {
    if(belong_list(buf, cuser.userid) <= 0)
    {
      if(brdattr & BRD_POSTMASK)  /* ÁôÂÃ */
        return 0;
      else
        return 2;
    }
    else return 1;
  }
                                         /* ­­¨î¾\ÅªÅv­­ */
// wildcat : ¶Ã¤C¤KÁV ...
// wildcat : ¤£­n°½Ãi°Õ ...
//  if((brdattr & BRD_PERSONAL) && !(brdattr & BRD_HIDE))
//    return 1;
  if(!(brdattr & BRD_POSTMASK) && HAS_PERM(level))
    return 1;
  else if (!(brdattr & BRD_POSTMASK) && !HAS_PERM(level))
    return 0;
  return 1;

}


static void
load_boards(char *bname , usint mode)
{
  boardheader *bptr;
  boardstat *ptr;
  char brdclass[5];
  int n;
  register char state;

  resolve_boards();
  if (!zapbuf)
    load_zapbuf();

  brdnum = 0;
  for (n = 0; n < numboards; n++)
  {
    bptr = &bcache[n];
    if (bptr->brdname[0] == '\0')
      continue;
    if (bname)
    {
      if(strcmp(bname,bptr->brdname)) continue;
      ptr = &nbrd[brdnum++];
      ptr->name = bptr->brdname;
      ptr->title = bptr->title;
      ptr->BM = bptr->BM;
      strcpy(ptr->desc[0], bptr->desc[0]);
      strcpy(ptr->desc[1], bptr->desc[1]);
      strcpy(ptr->desc[2], bptr->desc[2]);
      ptr->pos = n;
      ptr->total = -1;
      ptr->bvote = bptr->bvote;
      ptr->zap = (zapbuf[n] == 0);
      return;
    }

    if(mode)
    {
      if(!(bptr->brdattr & mode))
        continue;
    }
    else if (boardprefix)
    {
      if (boardprefix == str_local_board || boardprefix == str_good_board)
      {
        strncpy(brdclass, bptr->title + 5, 2);
        brdclass[2] = '\0';
      }
      else
      {
        strncpy(brdclass, bptr->title, 4);
        brdclass[4] = '\0';
      }
      if (strstr(boardprefix, brdclass) == NULL)
        continue;
    }
    else if (currmode & MODE_FAVORITE)
    {
      char fpath[80];
      if (currmode & MODE_FAVORITE)
        setuserfile(fpath, fn_myfavorite);
      if (!belong(fpath, bptr->brdname))
        continue;
    }
    else if(bptr->brdattr & BRD_GROUPBOARD || bptr->brdattr & BRD_CLASS)
      continue;

    state = Ben_Perm(bptr);
    if (state && (yank_flag == 1 ||
         yank_flag == 2 && (bptr->brdattr & (BRD_GROUPBOARD | BRD_CLASS) || have_author(bptr->brdname, n)) ||
         yank_flag != 2 && zapbuf[n]))
    {
      ptr = &nbrd[brdnum++];
      ptr->total = -1;
/*      ptr->lastposttime = &(brdshm->lastposttime[n]); */
      ptr->name = bptr->brdname;
      ptr->title = bptr->title;
      ptr->BM = bptr->BM;
      strcpy(ptr->desc[0], bptr->desc[0]);
      strcpy(ptr->desc[1], bptr->desc[1]);
      strcpy(ptr->desc[2], bptr->desc[2]);
      ptr->pos = n;
      ptr->bvote = bptr->bvote;
      ptr->zap = (zapbuf[n] == 0);
      ptr->brdattr =  bptr->brdattr;

      if((bptr->brdattr & BRD_HIDE) && state == 1)
              ptr->brdattr |= BRD_POSTMASK;
/*      check_newpost(ptr);*/
    }
  }

  /* ¦pªG user ±N©Ò¦³ boards ³£ zap ±¼¤F */

  if (!brdnum && !boardprefix)
  {
     if (yank_flag == 0)
        yank_flag = 1;
     else if (yank_flag == 2)
        yank_flag = 0;
     brdauthor = 0;
  }
}


static int
search_board(num)
{
  char genbuf[IDLEN + 2];
/* Ptt get out
  if (num >= 0) {
     getdata(b_lines - 1, 0, "½Ð¿é¤J¬ÝªO¦WºÙ¡G", genbuf, IDLEN + 1, DOECHO,0);
     move(b_lines - 1, 0);
     clrtoeol();
     move(b_lines, 0);
     if (genbuf[0])
     {
       int n = num + 1;
       str_lower(genbuf, genbuf);
       while (n != num)
       {
         if (++n >= brdnum)        n = 0;
         if (strstr_lower(nbrd[n].name, genbuf))
           return n;
         if (++n >= brdnum)
           n = 0;
       }
     }
  }
  else {
*/
     move(0,0);
     clrtoeol();
     CreateNameList();
     for (num = 0; num < brdnum; num++)
        AddNameList(nbrd[num].name);
     namecomplete(MSG_SELECT_BOARD, genbuf);
     for (num = 0; num < brdnum; num++)
        if (!strcmp(nbrd[num].name, genbuf))
           return num;
/*
  }
*/
  return -1;
}


int
check_newpost(ptr)
  boardstat *ptr;
{
  fileheader fh;
  struct stat st;
  char fname[FNLEN];
  char genbuf[200];
  int fd, total;

  ptr->total = ptr->unread = 0;
  setbdir(genbuf, ptr->name);
  if ((fd = open(genbuf, O_RDWR)) < 0)
    return 0;
  fstat(fd, &st);
  total = st.st_size / sizeof(fh);
  if (total <= 0)
  {
    close(fd);
    return 0;
  }
  ptr->total = total;
  if (!brc_initial(ptr->name))
  {
    ptr->unread = 1;
  }
  else
  {
    lseek(fd, (off_t)((total - 1) * sizeof(fh)), SEEK_SET);
    if (read(fd, fname, FNLEN) > 0 && brc_unread(fname))
    {
      ptr->unread = 1;
    }
  }
  close(fd);
  return 1;
}


static int
unread_position(dirfile, ptr)
  char *dirfile;
  boardstat *ptr;
{
  fileheader fh;
  char fname[FNLEN];
  register int num, fd, step, total;

  total = ptr->total;
  num = total + 1;
  if (ptr->unread && (fd = open(dirfile, O_RDWR)) > 0)
  {
    if (!brc_initial(ptr->name))
    {
      num = 1;
    }
    else
    {
      num = total - 1;
      step = 4;
      while (num > 0)
      {
        lseek(fd, (off_t)(num * sizeof(fh)), SEEK_SET);
        if (read(fd, fname, FNLEN) <= 0 || !brc_unread(fname))
          break;
        num -= step;
        if (step < 32)
          step += step >> 1;
      }
      if (num < 0)
        num = 0;
      while (num < total)
      {
        lseek(fd, (off_t)(num * sizeof(fh)), SEEK_SET);
        if (read(fd, fname, FNLEN) <= 0 || brc_unread(fname))
          break;
        num++;
      }
    }
    close(fd);
  }
  if (num < 0)
    num = 0;
  return num;
}


static void
brdlist_foot()
{
  prints(COLOR2"  ¿ï¾Ü¬ÝªO  "COLOR1"[1m  [33m(c)[37m·s¤å³¹¼Ò¦¡  [33m(v/V)\
[37m¼Ð°O¤wÅª/¥¼Åª  [33m(y)[33m¦C¥X%s  [33m(z)[37m¨ú®ø/­q¾\\     [m",
    yank_flag ? "­q¾\\" : "¥þ³¡");
}
/*
woju
*/
have_author(char* brdname, int n)
{
   char dirname[96];
   extern cmpfowner();

   sprintf(dirname, "¥¿¦b·j´M§@ªÌ[33m%s[m ¬ÝªO:[1;33m%12s[0m.....[%d/%d]",
           currauthor, brdname, n, numboards);
   move(b_lines, 0);
   clrtoeol();
   outs(dirname);
   refresh();

   setbdir(dirname, brdname);
   str_lower(currowner, currauthor);

   return search_rec(dirname, cmpfowner);
}

static void
show_brdlist(head, clsflag, newflag)
{
  if (clsflag)
  {
    sprintf(tmpbuf,"%s [½u¤W %d ¤H]",BOARDNAME,count_ulist());
    showtitle("¬ÝªO¦Cªí", tmpbuf);
#ifdef HYPER_BBS
    prints(HB_BACK"\033[200m\033[444m\033[507m[¡÷]¾\\Åª\033[201m[\033[200m\033[444m\033[504m¡ô\033[201m\033[200m\033[444m\033[505m¡õ\033[201m]¿ï¾Ü[y]¸ü¤J[S]±Æ§Ç[/]·j´M [^Z]¨D§U [\033[200m\033[444m\033[500mPgUp\033[201m/\033[200m\033[444m\033[501mPgDn\033[201m]¤W¤U­¶\n"
      COLOR1"[1m%-20s Ãþ§OÂà«H%-33s§ë²¼ ªO    ¥D    [m",
      newflag ? "Á`¼Æ ¥¼Åª ¬Ý  ªO" : "  ½s¸¹  ¬Ý  ªO", 
      clsflag == 1 ? " ¤¤   ¤å   ±Ô   ­z" : " ¢è²ÎÂà¨pÁô°ÎÀu­Ó ");
#else
    prints("[¡ö]¤W¤@­¶[¡÷]¾\\Åª¡ô¡õ]¿ï¾Ü[y]¸ü¤J[S]±Æ§Ç[/]·j´M [^Z]¨D§U [PgUp/PgDn]¤W¤U­¶\n"
      COLOR1"[1m%-20s Ãþ§OÂà«H%-33s§ë²¼ ªO    ¥D    [m",
      newflag ? "Á`¼Æ ¥¼Åª ¬Ý  ªO" : "  ½s¸¹  ¬Ý  ªO", 
      clsflag == 1 ? " ¤¤   ¤å   ±Ô   ­z" : " ¢è²ÎÂà¨pÁô°ÎÀu­Ó ");
#endif
    move(b_lines, 0);
    brdlist_foot();
  }

  if (brdnum > 0)
  {
    boardstat *ptr;
    int myrow;  /* Ptt add color */
    static char *color[7]={"[1;36m","[1;34m","[1;33m","[1;32m","[1;35m","[1;36m","[1;37m"};

    static char *unread[2]={"  ","[36m¡´"};

    myrow = 2;
    while (++myrow < b_lines-5)
    {
#ifdef HYPER_BBS
      char hbuf[256];
#endif
      move(myrow, 0);
      clrtoeol();
      if (head < brdnum)
      {
        ptr = &nbrd[head++];

#ifdef HYPER_BBS
	sprintf(hbuf,"\033[200m\033[400m\033[444m\033[300m\033[%dm\033[%dm\033[%dm\033[%dm\033[%dm\033[613m\033[613m",
	  (head/10000)+648,
	  ((head%10000)/1000)+648,
	  ((head%1000)/100)+648,
	  ((head%100)/10)+648,
	  (head%10)+648);
#endif
        if (ptr->total == -1)
            check_newpost(ptr);

        if (yank_flag == 2)
        { 
	  prints("%5d%c%c",
          head,ptr->brdattr & BRD_HIDE ? ')' : 
          head,ptr->brdattr & BRD_INVITE ? '@' : ' ',
          (ptr->brdattr & BRD_GROUPBOARD || ptr->brdattr & BRD_CLASS) ? ' ':'A');
#ifdef HYPER_BBS
          prints("%s ",hbuf);
#else
          prints(" ");
#endif
        }
        else if (!newflag)
#ifdef HYPER_BBS
          prints("%5d%c%s%s",
#else
          prints("%5d%c%s",
#endif
                 head,!(ptr->brdattr & BRD_HIDE) ? ' ':
                 (ptr->brdattr & BRD_POSTMASK) ? ')' : '-',
                 ptr->zap ? "--" :
                 (ptr->brdattr & BRD_GROUPBOARD) ? "[1;34m£U" :
                 (ptr->brdattr & BRD_CLASS) ? "[1;36m¡¼" :
                 unread[ptr->unread]);

        else if (ptr->zap)
        {
          outs("  ------");
        }
        else
        {
          if (ptr->total == -1)
            check_newpost(ptr);
          if (newflag)
            {
              prints((ptr->brdattr & BRD_GROUPBOARD
              || ptr->brdattr & BRD_CLASS) ? "        "
              :"%6d%s", (ptr->total),unread[ptr->unread]);
            }
        }

        if(clsflag != 2)
        {
          prints("%-13s[m%s%5.5s[m%-2.2s %-34.34s%s  %-13.13s[201m",
            ptr->name,
            color[(unsigned int)(ptr->title[1]+ptr->title[2]+
                   ptr->title[3]+ptr->title[0])%7],
            ptr->title,ptr->title+5,ptr->title+7,
            (ptr->bvote == 1 ? "[1;33m¦³[m" : 
             ptr->bvote == 2 ? "[1;37m¶}[m" : "  "),
            ptr->BM);
        }

        else
        {
          char attrbuf[35];
          
          sprintf(attrbuf," %s%s%s%s%s%s%s%s ",
            ptr->brdattr & BRD_NOZAP ? "¢æ" : "£¾",
	    ptr->brdattr & BRD_NOCOUNT ? "¢æ" : "£¾",
	    ptr->brdattr & BRD_NOTRAN ? "¢æ" : "£¾",
	    ptr->brdattr & BRD_HIDE ? "£¾" : "¢æ",
	    (ptr->brdattr & BRD_HIDE) && (ptr->brdattr & BRD_POSTMASK) ? "£¾" : "¢æ",
	    ptr->brdattr & BRD_ANONYMOUS ? "£¾" : "¢æ",
	    ptr->brdattr & BRD_GOOD ? "£¾" : "¢æ",
	    ptr->brdattr & BRD_PERSONAL ? "£¾" : "¢æ");

          prints("%-13s[m%s%5.5s[m%-2.2s %-34.34s%s  %-13.13s[201m",
            ptr->name,
            color[(unsigned int)(ptr->title[1]+ptr->title[2]+
                   ptr->title[3]+ptr->title[0])%7],
            ptr->title,ptr->title+5,attrbuf,
            (ptr->bvote == 1 ? "[1;33m¦³[m" : 
             ptr->bvote == 2 ? "[1;37m¶}[m" : "  "),
             ptr->BM);
        }

      }
//      clrtoeol();
//      refresh();
    }
  }
}

/* ®Ú¾Ú title ©Î name °µ sort */

static
int
cmpboard(brd, tmp)
  boardstat *brd, *tmp;
{
  register int type = 0;

  if (!type)
   {
    type = strncmp(brd->title, tmp->title, 4);
    type *= 256;
    type += strcasecmp(brd->name, tmp->name);
   }
  if (!(cuser.uflag & BRDSORT_FLAG))
    type = strcasecmp(brd->name, tmp->name);
  return type;
}

static void
set_menu_BM(char *BM)
{
  if (HAS_PERM(PERM_ALLBOARD) || is_BM(BM))
  {
    currmode |= MODE_MENU;
  }
}

static void
choose_board(int newflag,usint mode)
{
  static int num = 0;
  int attmode = 1;
  boardstat *ptr;
  int head, ch, tmp,tmp1;
  char genbuf[200],*prefixtmp;
  extern time_t board_visit_time;
  extern int m_newbrd(),m_mod_board(char *bname);

  setutmpmode(newflag ? READNEW : READBRD);
  brdnum = 0;
  if (!cuser.userlevel)         /* guest yank all boards */
    yank_flag = 1;

  do
  {
    if (brdnum <= 0)
    {
      load_boards(NULL,mode);
      if (brdnum <= 0)
        break;
      qsort(nbrd, brdnum, sizeof(boardstat), cmpboard);
      head = -1;
    }

    if (num < 0)
      num = 0;
    else if (num >= brdnum)
      num = brdnum - 1;

    if (head < 0)
    {
      if (newflag)
      {
        tmp = num;
        while (num < brdnum)
        {
          ptr = &nbrd[num];
          if (ptr->total == -1)
            check_newpost(ptr);
          if (ptr->unread)
            break;
          num++;
        }
        if (num >= brdnum)
          num = tmp;
      }
      head = (num / page_lines) * page_lines;
      show_brdlist(head, 1, newflag);
    }
    else if (num < head || num >= head + page_lines)
    {
      head = (num / page_lines) * page_lines;
      show_brdlist(head, 0, newflag);
    }
    clrchyiuan(b_lines-5,b_lines-1);
    move(b_lines-5,0);
    prints("[1;36m%s[m\n", msg_seperator);
    prints(" [1;33m%s [mªOªº¬ÝªO»¡©ú :\n%-80.80s\n%-80.80s\n%-80.80s",
    nbrd[num].name,nbrd[num].desc[0],nbrd[num].desc[1],nbrd[num].desc[2]);
    {
      int c;

      c = page_lines - (num%page_lines);
      if(num+c < brdnum && nbrd[num+c].name)
      {
        move(b_lines-5,69);
        prints("[ÁÙ¦³¤U­¶­ò¡I]");
      }
    }
    ch = cursor_key(3 + num - head, 0);

    switch (ch)
    {
      case 'e':
      case KEY_LEFT:
      case EOF:
        ch = 'q';
      case 'q':
        break;

     case 'c':
      if (yank_flag == 2) {
         newflag = yank_flag = 0;
         brdnum = -1;
         brdauthor = 0;
      }
      show_brdlist(head, 1, newflag ^= 1);
      break;


// wildcat : show board attr in list
     case 'A':
      if(!HAS_PERM(PERM_SYSOP))
        break;
      if(attmode >= 2) attmode = 1;
      else attmode = 2;
      show_brdlist(head, attmode, newflag);
      break;

     case 'a': 
     {
       if (yank_flag != 2 ) 
       {
         sprintf(genbuf, "%s", currauthor);
         if (getdata(1, 0,"§@ªÌ:", genbuf, IDLEN + 2, DOECHO, currauthor))
            strncpy(currauthor, genbuf, IDLEN + 2);
         if (*currauthor)
         {
           yank_flag = 2;
           brdauthor = 1;
         }
         else
           yank_flag= 0;
       }
       else
       {
         yank_flag = 0;
         brdauthor = 0;
       }

       brdnum = -1;
       show_brdlist(head, 1, newflag);
       break;
      }

      case 'f':
      case 'F':
      {
        char fpath[256];
        ptr = &nbrd[num];
/*
        if ((ptr->brdattr & (BRD_GROUPBOARD | BRD_CLASS)))
          break;
*/
        brc_initial(ptr->name);
        setuserfile(fpath, fn_myfavorite);
        if (ch == 'f' )
        {
          if(file_list_count(fpath) > MAX_FAVORITE)
            pressanykey("­Ó¤H³ß·R¬Ýª©¤w¹F³Ì¤j¶q..^_^");
          else
          {
            idlist_add(fpath, NULL,ptr->name);
            pressanykey("[%s]ª©¤w¸g¥[¤J..^O^", ptr->name);
          }
          brdnum = -1;
        }
        else if (ch =='F')
        {
          idlist_delete(fpath,ptr->name);
          pressanykey("[%s]ª©¤w¸g²¾°£¤F..:(", ptr->name);
          brdnum = -1;
        }
        break;
      }
      case KEY_PGUP:
      case 'P':
      case 'b':
      case Ctrl('B'):
        if (num)
        {
          num -= page_lines;
          break;
        }

      case KEY_END:
      case '$':
        num = brdnum - 1;
        break;

      case ' ':
      case KEY_PGDN:
      case 'N':
      case Ctrl('F'):
        if (num == brdnum - 1)
          num = 0;
        else
          num += page_lines;
        break;

      case KEY_ESC: if (KEY_ESC_arg == 'n') 
      {
         edit_note();
         show_brdlist(head, 1, newflag);
      }
      break;

      case KEY_UP:
      case 'p':
      case 'k':
        if (num-- <= 0)
          num = brdnum - 1;
        break;

      case KEY_DOWN:
      case 'n':
      case 'j':
        if (++num < brdnum)
          break;

      case '0':
      case KEY_HOME:
        num = 0;
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
        if ((tmp = search_num(ch, brdnum)) >= 0)
          num = tmp;
        brdlist_foot();
        break;

      case Ctrl('A'):
        Announce();
        show_brdlist(head, 1, newflag);
        break;

      case '/':
        if ((tmp = search_board(num)) >= 0)
          num = tmp;
        show_brdlist(head, 1, newflag);
        break;

      case 'S':
        cuser.uflag ^= BRDSORT_FLAG;
        substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* °O¿ý */
        move(3,0);
        clrtobot();
        qsort(nbrd, brdnum, sizeof(boardstat), cmpboard);
        head = 999;
        move(b_lines,0);
        brdlist_foot();
        break;

      case 'y':
        if (yank_flag == 2)
        {
          yank_flag = 0;
          brdauthor = 0;
        }
        else
          yank_flag ^= 1;

        brdnum = -1;
        break;

      case 'z':                   /* opus: no perm check ? */
        if (HAS_PERM(PERM_BASIC))
        {
          ptr = &nbrd[num];
          ptr->zap = !ptr->zap;
          if(ptr->brdattr & BRD_NOZAP) ptr->zap = 0;
          if(!ptr->zap) check_newpost(ptr);
          zapbuf[ptr->pos] = (ptr->zap ? 0 : login_start_time);
          head = 999;
        }
        break;

      case 'Z':                   /* opus: no perm check ? */
        if (HAS_PERM(PERM_BASIC))
        {
          int i;
          for(i=0;i < MAXBOARD;i++)
          {
            ptr = &nbrd[i];
            ptr->zap = 0;
            check_newpost(ptr);
            zapbuf[ptr->pos] = (ptr->zap ? 0 : login_start_time);
            head = 999;
          }
        }
        break;

      case 'v':
      case 'V':
        ptr = &nbrd[num];
        brc_initial(ptr->name);
        if (ch == 'v')
        {
          ptr->unread = 0;
          zapbuf[ptr->pos] = time((time_t *) &brc_list[0]);
        }
        else
          zapbuf[ptr->pos] = brc_list[0] = ptr->unread = 1;
        brc_num = brc_changed = 1;
        brc_update();
        show_brdlist(head, 0, newflag);
        break;

      case 's':
        if ((tmp = search_board(-1)) < 0)
          show_brdlist(head, 1, newflag);
        num = tmp;
        break;

      case 'Q':
        if (HAS_PERM(PERM_BASIC) || (currmode & MODE_MENU))
        {
          ptr = &nbrd[num];
          move(1,1);
          clrtobot();
          m_mod_board(ptr->name);
          brdnum = -1;
        }
        break;
      case 'B':
        if (HAS_PERM(PERM_SYSOP) || (currmode & MODE_MENU)) 
        {
          m_newbrd();
          brdnum = -1;
        }
        break;
      case KEY_RIGHT:
      case '\n':
      case '\r':
      case 'r':
      {
        char buf[STRLEN];

        ptr = &nbrd[num];

        if (!(ptr->brdattr & BRD_GROUPBOARD || ptr->brdattr & BRD_CLASS)) /* «Dsub class */
        {

/*
   wildcat 000121 : ¥u­n§PÂ_¨p¤HªO´N¦n , ÁôÂÃªO¬Ýªº¨ìªº´N¬Ýªº¨ì , ¬Ý¤£¨ìªº
                    ÁÙ¬O¬Ý¤£¨ì :Q , ¥»¨Ó¦³¬q©_©Çªº FN_APPLICATION ´N®³±¼§a
                    ¥i¬O«ç»òÄ±±o¬O¦b­«½Æ Ben_Perm °µªº§PÂ_?
*/
          if((ptr->brdattr & BRD_HIDE && !(ptr->brdattr & BRD_POSTMASK))
             && (!HAS_PERM(PERM_SYSOP) && !is_BM(ptr->BM)))
          {
            setbfile(buf, ptr->name, FN_LIST);
            if(belong_list(buf,cuser.userid) <= 0)
            {
              pressanykey(P_BOARD);
              break;
            }
          }

          brc_initial(ptr->name);

          if (yank_flag == 2)
          {
            setbdir(buf, currboard);
            tmp = have_author(currboard, num) - 1;
            head = tmp - t_lines / 2;
            getkeep(buf, head > 1 ? head : 1, -(tmp + 1));
          }
          else if (newflag)
          {
            setbdir(buf, currboard);
            tmp = unread_position(buf, ptr);
            head = tmp - t_lines / 2;
            getkeep(buf, head > 1 ? head : 1, tmp + 1);
          }
          board_visit_time = zapbuf[ptr->pos];
          if (!ptr->zap)
            time((time_t *) &zapbuf[ptr->pos]);
          Read();
          check_newpost(ptr);
          head = -1;
          setutmpmode(newflag ? READNEW : READBRD);
        }
        else                                   /* sub class */
        {
            prefixtmp = boardprefix;
            tmp1=num; num=0;
            boardprefix = ptr->title+7;
            set_menu_BM(ptr->BM);
            log_board2(ptr->name,0);
            if(!strcmp(ptr->name,"Personal_All"))
              choose_board(cuser.habit & HABIT_BOARDLIST,BRD_PERSONAL);
            else if(!strcmp(ptr->name,"Hide_All"))
              choose_board(cuser.habit & HABIT_BOARDLIST,BRD_HIDE);
            else
              choose_board(cuser.habit & HABIT_BOARDLIST,0);

            currmode &= ~MODE_MENU;
            num=tmp1;
            boardprefix = prefixtmp;
            brdnum = -1;

         }
      }
    }
  } while (ch != 'q');
  save_zapbuf();
}


int
board()
{
  choose_board(cuser.habit & HABIT_BOARDLIST,0);
  return 0;
}


int
local_board()
{
  boardprefix = str_local_board;
  choose_board(cuser.habit & HABIT_BOARDLIST,0);
  return 0;
}

int
good_board()
{
  boardprefix = str_good_board;
  choose_board(cuser.habit & HABIT_BOARDLIST,0);
  return 0;
}

int
Boards()
{
  boardprefix = NULL;
  choose_board(cuser.habit & HABIT_BOARDLIST,0);
  return 0;
}


int
New()
{
  int mode0 = currutmp->mode;
  int stat0 = currstat;

  boardprefix = NULL;
  choose_board(cuser.habit & HABIT_BOARDLIST,0);
  currutmp->mode = mode0;
  currstat = stat0;
  return 0;
}

int
Favor()
{
  int mode0 = currutmp->mode;
  int stat0 = currstat;
  char fpath[80];
  FILE *fp;
  boardprefix = NULL;
  currmode |= MODE_FAVORITE;

  setuserfile(fpath, fn_myfavorite);
  if ((fp = fopen(fpath, "r")) == NULL) return;
  else fclose(fp);

  choose_board(cuser.habit & HABIT_BOARDLIST,0);

  currmode &= ~MODE_FAVORITE;
  currutmp->mode = mode0;
  currstat = stat0;
  return 0;
}



int
favor_edit() {
  char fpath[80];
  int count, column, dirty;
  FILE *fp;
  char genbuf[200];
  char bn[60];

  if (brdnum <= 0)
  {
    load_boards(NULL,0);
    qsort(nbrd, brdnum, sizeof(boardstat), cmpboard);
  }

  setuserfile(fpath, fn_myfavorite);
  move(0, 0);

  dirty = 0;
  while (1)
  {
    stand_title("§Úªº³Ì·R");

    count = 0;
    CreateNameList();

    if (fp = fopen(fpath, "r"))
    {
      move(3, 0);
      column = 0;
      while (fgets(genbuf, STRLEN, fp))
      {
        if (genbuf[0] <= ' ')
          continue;
        strtok(genbuf, str_space);
        if (count < MAX_FAVORITE) {
          AddNameList(genbuf);
          prints("%-13s", genbuf);
          count++;
          if (++column > 5) {
            column = 0;
            outc('\n');
          }
        }
      }
      fclose(fp);
    }
    getdata(1, 0, (count ? "(A)¼W¥[ (D)§R°£ (Q)µ²§ô¡H[Q] " :
        "(A)¼W¥[ (Q)µ²§ô¡H[Q] "), bn, 4, LCECHO, 0);
    if (*bn == 'a') {
      move(1, 0);
      brdcomplete(MSG_MY_FAVORITE, bn);
      if (count < MAX_FAVORITE) {
        if (bn[0] && !InNameList(bn)) {
          idlist_add(fpath, NULL, bn);
          dirty = 1;
        }
      }
    }
    else if ((*bn == 'd') && count)
    {
      move(1, 0);
      bn[0] = 0;
      namecomplete(MSG_MY_FAVORITE, bn);
      if (bn[0] && InNameList(bn))
      {
        idlist_delete(fpath, bn);
        dirty = 1;
      }
    }
    else
      break;
  }

  return 0;
}

extern struct BCACHE *brdshm;

char *
b_namearray(buf, pnum, tag)
  char buf[][IDLEN + 1], *tag;
  int *pnum;
{
  register struct BCACHE *reg_bshm = brdshm;
  register char *ptr, tmp;
  register int n, total;
  char tagbuf[STRLEN];
  int ch, ch2, num;

  resolve_boards();
  total = reg_bshm->number;
  if (*tag == '\0')
  {
    for (n = num = 0; n < total; n++)
    {
      if (!Ben_Perm(&reg_bshm->bcache[n])) continue;
      ptr = reg_bshm->bcache[n].brdname;
      tmp = *ptr;
      strcpy(buf[num++], ptr);
    }
    *pnum = num;
    *pnum = num;
    return buf[0];
  }
  for (n = 0; tag[n]; n++)
  {
    tagbuf[n] = chartoupper(tag[n]);
  }
  tagbuf[n] = '\0';
  ch = tagbuf[0];
  ch2 = ch - 'A' + 'a';
  for (n = num = 0; n < total; n++)
  {
    if (!Ben_Perm(&reg_bshm->bcache[n])) continue;
    ptr = reg_bshm->bcache[n].brdname;
    tmp = *ptr;
    if (tmp == ch || tmp == ch2)
    {
      if (chkstr(tag, tagbuf, ptr))
        strcpy(buf[num++], ptr);
    }
  }
  *pnum = num;
  return buf[0];
}

void force_board(char *bname)
{
  boardstat *ptr;
  char buf[80];

  setbpath(buf ,bname);
  if(!dashd(buf)) return;
  brdnum = 0;
  load_boards(bname,0);
  ptr = &nbrd[0];
  check_newpost(ptr);
  while(ptr->unread && cuser.userlevel) /* guest skip force read */
  {
    pressanykey(" %s ª©¦³·s¤å³¹! ½Ð¾\\Åª§¹·s¤å³¹«á¦AÂ÷¶}.. ^^",bname);
    brc_initial(ptr->name);
    Read();
    check_newpost(ptr);
  }
}

void voteboard()
{
  boardstat *ptr;
  char buf[80];

  setbpath(buf ,VOTEBOARD);
  if(!dashd(buf)) return;
  brdnum = 0;
  load_boards(VOTEBOARD,0);
  ptr = &nbrd[0];
  brc_initial(ptr->name);
  Read();
}

