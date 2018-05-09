/*-------------------------------------------------------*/
/* announce.c   ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : 精華區閱讀、編選                             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

/*==========

    This file is re-edited by Tsai Menguang (mgtsai), Sep 10th, '96

    Change the announce directory structure as .DIR format

==========*/


#include "bbs.h"

#define FHSZ            sizeof(fileheader)

#define ADDITEM         0
#define ADDGROUP        1
#define ADDGOPHER       2
#define ADDLINK         3
#define ADDFILE         4
#define MAXPATHLEN	1024
char trans_buffer[256];

enum
{
  NOBODY, MANAGER, SYSOP
};


static char copyfile[MAXPATHLEN];
static char copytitle[TTLEN + 1];
static char copyowner[IDLEN + 2];
static char *mytitle = BOARDNAME "佈告欄";
static char *a_title;
static char paste_fname[200];

static int
a_perm(fname,fhdr,me)
  char *fname;
  fileheader *fhdr;
  AMENU *me;
{
  char buf[MAXPATHLEN];
  if (fhdr->filemode & FILE_REFUSE)
  {
    if(dashf(fname))
      sprintf(buf,"%s.vis",fname);
    else
      sprintf(buf,"%s/.vis",fname);
    if(me->level >= MANAGER)
      return 2;
//    if(belong_list(buf,cuser.userid) <= 0)
    if(belong_list(buf,cuser.userid) != 0)
      return 1;
    else
      return 0;
  }
  else
    return 1;
}
static void
g_showmenu (pm)
     GMENU *pm;
{
  static char *mytype = "編    選     絲路之旅";
  char *title, ch;
  int n, max;
  ITEM *item;

  showtitle ("精華文章", pm->mtitle);
  prints ("  \x1b[1;32m編號    標      題%56s\x1b[m", mytype);

  if (pm->num)
    {
      n = pm->page;
      max = n + p_lines;
      if (max > pm->num)
	max = pm->num;
      while (n < max)
	{
#ifdef	HYPER_BBS
          char hbuf[256];	// HyperBBS
          sprintf(hbuf, "\x1b[200m\x1b[400m\x1b[444m\x1b[300m\x1b[%dm\x1b[%dm\x1b[%dm\x1b[%dm\x1b[%dm\x1b[613m\x1b[613m",
                  (n/10000)+648,
                  ((n%10000)/1000)+648,
                  ((n%1000)/100)+648,
                  ((n%100)/10)+648,
                  (n%10)+648);
#endif
	  item = pm->item[n++];
	  title = item->title;
	  ch = title[1];
#ifdef	HYPER_BBS
	  prints ("\n%5d.%s %-72.71s\x1b[201m", n, hbuf, title);
#else
	  prints ("\n%5d. %-72.71s", n, title);
#endif
	}
    }
  else
      outs ("\n  \x1b[1;32m《\x1b[36m精華區\x1b[32m》\x1b[m吸取天地日月精華當中.... :)");

  move (b_lines, 0);
  outs (pm->level ?
	COLOR2 " 【板  主】 " COLOR1 "\x1b[1m \x1b[33m(Ctrl+Z)\x1b[37m說明  \x1b[33m(q/←)\x1b[37m離開  \
\x1b[33m(n)\x1b[37m新增文章  \x1b[33m(g)\x1b[37m新增目錄  \x1b[33m(e)\x1b[37m編輯檔案  \x1b[m" :
	COLOR2 " 【功\能鍵】 " COLOR1 "\x1b[1m \x1b[33m(Ctrl+Z)\x1b[37m說明  \x1b[33m(q/←)\x1b[37m離開  \
\x1b[33m(k↑j↓)\x1b[37m移動游標  \x1b[33m(enter/→)\x1b[37m讀取資料  \x1b[m");
}

/*-------------------------------------------------------*/

FILE *
go_cmd (node, sock)
     ITEM *node;
     int *sock;
{
  struct sockaddr_in sin;
  struct hostent *host;
  char *site;
  FILE *fp;

  *sock = socket (AF_INET, SOCK_STREAM, 0);

  if (*sock < 0)
    {
      perror ("ERROR(get socket)");
      return NULL;
    }
  (void) memset ((char *) &sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons (node->X.G.port);

  host = gethostbyname (site = node->X.G.server);
  if (host == NULL)
    sin.sin_addr.s_addr = inet_addr (site);
  else
    (void) memcpy (&sin.sin_addr.s_addr, host->h_addr, host->h_length);

  if (connect (*sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
  {
    perror ("ERROR(connect)");
    return NULL;
  }
  fp = fdopen (*sock, "r+");
  if (fp != NULL)
  {
    setbuf (fp, (char *) 0);
    fprintf (fp, "%s\r\n", node->X.G.path);
    fflush (fp);
  }
  else
    close (*sock);
  return fp;
}


char *
nextfield (data, field)
     register char *data, *field;
{
  register int ch;

  while (ch = *data)
    {
      data++;
      if ((ch == '\t') || (ch == '\r' && *data == '\n'))
	break;
      *field++ = ch;
    }
  *field = '\0';
  return data;
}

FILE *
my_open (char *path)
{
  FILE *ans = 0;
  struct stat st;
  time_t now = time (0);

  if (stat (path, &st) == 0 && st.st_mtime < now - 3600 * 24 * 7)
    {
/*
   woju
   getdata(b_lines - 1, 0, "已超過一星期沒更新，是否更新資料(Y/N)？[Y] ",
   buf, 4, LCECHO, 0);
   if (*buf != 'n')
 */
      return fopen (path, "w");
    }

  if (ans = fopen (path, "r+"))
    {
      fclose (ans);
      return 0;
/*
   return directly due to currutmp->pager > 1 mode (real copy)
      fgets (buf, 80, ans);
      if (!strncmp (buf, str_author1, strlen (str_author1))
	  || *buf == '0' || *buf == '1')
	{
	  fclose (ans);
	  return 0;
	}

      rewind (ans);
*/
    }
  else
    ans = fopen (path, "w");
  return ans;
}

static jmp_buf jbuf;

static void
isig (sig)
     int sig;
{
  longjmp (jbuf, 1);
}

go_proxy (char *fpath, ITEM * node, int update)
{
  char *ptr, *str, *server;
  int ch;
  FILE *fo;

#define PROXY_HOME      "proxy/"

  strcpy (fpath, PROXY_HOME);
  ptr = fpath + sizeof (PROXY_HOME) - 1;

  str = server = node->X.G.server;
  while (ch = *str)
    {
      str++;
      if (ch == '.')
	{
	  if (!strcmp (str, "edu.tw"))
	    break;
	}
      else if (ch >= 'A' && ch <= 'Z')
	{
	  ch |= 0x20;
	}
      *ptr++ = ch;
    }
  *ptr = '\0';
  mkdir (fpath, 0755);

  *ptr++ = '/';
  str = node->X.G.path;
  while (ch = *str)
    {
      str++;
      if (ch == '/')
	{
	  ch = '.';
	}
      *ptr++ = ch;
    }
  *ptr = '\0';

  /* expire proxy data */

  if (fo = update ? fopen (fpath, "w") : my_open (fpath))
    {
      FILE *fp;
      char buf[512];
      int sock;

      if (fo == NULL)
	return 0;

      outmsg ("★ 建立 proxy 資料連線中 ... ");
      refresh ();

      sock = -1;
      if (setjmp (jbuf))
	{
/*      reset_tty(); */
	  if (sock != -1)
	    close (sock);
	  fseek (fo, 0, SEEK_SET);
	  fwrite ("", 0, 0, fo);
	  fclose (fo);
	  alarm (0);
	  return 0;
	}

      signal (SIGALRM, isig);
      alarm (5);
      fp = go_cmd (node, &sock);
      alarm (0);

      str = node->title;
      ch = str[1];
      if (ch == (char) 0xbc && !(HAS_PERM (PERM_SYSOP) && currutmp->pager > 1))
	{
	  time_t now;

	  time (&now);
	  fprintf (fo, "作者: %s (連線精華區)\n標題: %s\n時間: %s\n",
		   server, str + 3, ctime (&now)
	    );
	}

      while (fgets (buf, 511, fp))
	{
	  if (!strcmp (buf, ".\r\n"))
	    break;
	  if (ptr = strstr (buf, "\r\n"))
	    strcpy (ptr, "\n");
	  fputs (buf, fo);
	}
      fclose (fo);
      fclose (fp);
    }
}

g_additem (GMENU * pm, ITEM * myitem)
{
  if (pm->num < MAXITEMS)
    {
      ITEM *newitem = (ITEM *) malloc (sizeof (ITEM));

      memcpy (newitem, myitem, sizeof (ITEM));
      pm->item[(pm->num)++] = newitem;
    }
}
go_menu (GMENU * pm, ITEM * node, int update)
{
  FILE *fp;
  char buf[512], *ptr, *title;
  ITEM item;
  int ch;
  go_proxy (buf, node, update);

  pm->num = 0;
  if (fp = fopen (buf, "r"))
    {
      title = item.title;
      while (fgets (buf, 511, fp))
	{
	  ptr = buf;
	  ch = *ptr++;
	  if (ch != '0' && ch != '1')
	    continue;

	  strcpy (title, "□ ");
	  if (ch == '1')
	    title[1] = (char) 0xbd;

	  ptr = nextfield (ptr, title + 3);
	  if (!*ptr)
	    continue;
	  title[sizeof (item.title) - 1] = '\0';

	  ptr = nextfield (ptr, item.X.G.path);
	  if (!*ptr)
	    continue;

	  ptr = nextfield (ptr, item.X.G.server);
	  if (!*ptr)
	    continue;

	  nextfield (ptr, buf);
	  item.X.G.port = atoi (buf);

	  g_additem (pm, &item);
	}
      fclose (fp);
    }
}

g_searchtitle (GMENU * pm, int rev)
{
  static char search_str[30] = "";
  int pos;
  if (getdata (b_lines - 1, 1, "[搜尋]關鍵字:", search_str, 40,
	       DOECHO, 0))
    if (!*search_str)
      return pm->now;

  str_lower (search_str, search_str);

  rev = rev ? -1 : 1;
  pos = pm->now;
  do
    {
      pos += rev;
      if (pos == pm->num)
	pos = 0;
      else if (pos < 0)
	pos = pm->num - 1;
      if (strstr_lower (pm->item[pos]->title, search_str))
	return pos;
    }
  while (pos != pm->now);
  return pm->now;
}

gem(char *maintitle, ITEM * path, int update)
{
  GMENU me;
  int ch;
  char fname[MAXPATHLEN];

  strncpy (me.mtitle, maintitle, 40);
  me.mtitle[40] = 0;
  go_menu (&me, path, update);

  /* 精華區-tree 中部份結構屬於 cuser ==> BM */

  me.level = 0;
  me.page = 9999;
  me.now = 0;
  for (;;)
    {
      if (me.now >= me.num && me.num > 0)
	me.now = me.num - 1;
      else if (me.now < 0)
	me.now = 0;

      if (me.now < me.page || me.now >= me.page + p_lines)
	{
	  me.page = me.now - (me.now % p_lines);
	  g_showmenu (&me);
	}
      ch = cursor_key (2 + me.now - me.page, 0);
      if (ch == 'q' || ch == 'Q' || ch == KEY_LEFT)
	break;

      if (ch >= '0' && ch <= '9')
	{
	  if ((ch = search_num (ch, me.num)) != -1)
	    me.now = ch;
	  me.page = 9999;
	  continue;
	}

      switch (ch)
	{
	case KEY_UP:
	case 'k':
	  if (--me.now < 0)
	    me.now = me.num - 1;
	  break;
	case KEY_DOWN:
	case 'j':
	  if (++me.now >= me.num)
	    me.now = 0;
	  break;

	case KEY_PGUP:
	case 'b':
	  if (me.now >= p_lines)
	    me.now -= p_lines;
	  else if (me.now > 0)
	    me.now = 0;
	  else
	    me.now = me.num - 1;
	  break;

	case ' ':
	case KEY_PGDN:
	case Ctrl ('F'):
	  if (me.now < me.num - p_lines)
	    me.now += p_lines;
	  else if (me.now < me.num - 1)
	    me.now = me.num - 1;
	  else
	    me.now = 0;
	  break;

	case '?':
	case '/':
	  me.now = g_searchtitle (&me, ch == '?');
	  me.page = 9999;
	  break;
	case 'N':
	  if (HAS_PERM (PERM_SYSOP))
	    {
	      go_proxy (fname, me.item[me.now], 0);
	      move (b_lines - 1, 0);
	      pressanykey (fname);
	      me.page = 9999;
	    }
	  break;
	case 'c':
	case 'C':
	case Ctrl ('C'):
	  if (me.now < me.num)
	    {
	      ITEM *node = me.item[me.now];
	      char *title = node->title;
	      int mode = title[1];

	      load_paste ();
	      if (mode == (char) 0xbc || ch == Ctrl ('C'))
	      {
		if (mode == (char) 0xbc)
		  go_proxy (fname, node, 0);
		if (ch == Ctrl ('C') && *paste_path && paste_level)
		{
		  char newpath[MAXPATHLEN];
		  fileheader item;

		  strcpy (newpath, paste_path);
		  if (mode == (char) 0xbc)
		  {
		    stampfile (newpath, &item);
		    unlink (newpath);
		    f_cp (fname, newpath,O_TRUNC);
		  }
		  else
		    stampdir(newpath, &item);
		  strcpy(item.owner, cuser.userid);
		  sprintf(item.title, "%s%.72s","◇ " , title + 3);

		  strcpy (strrchr (newpath, '/') + 1, ".DIR");
		  rec_add (newpath, &item, FHSZ);
		  if(++me.now >= me.num)
		    me.now = 0;
		  break;
		}
		if (mode == (char) 0xbc)
		{
		  a_copyitem (fname, title , 0);
		  if (ch == 'C' && *paste_path)
		  {
		    a_menu (paste_title, paste_path, paste_level, ANNOUNCE);
		  }
		  me.page = 9999;
		}
		else
		  bell ();
	      }
	    }
	  break;

	case Ctrl ('S'):
	  if (me.now < me.num)
	    {
	      ITEM *node = me.item[me.now];
	      char *title = node->title;
	      int mode = title[1];

	      if (mode == (char) 0xbc)
		{
		  fileheader fhdr;

		  go_proxy (fname, node, 0);
		  strcpy (fhdr.owner, "連線精華區");
		  strcpy (fhdr.title, title);
		  if (save_mail (-1, &fhdr, fname) == POS_NEXT)
		    if (++me.now >= me.num)
		      me.now = 0;
		}
	      else
		bell ();
	    }
	  break;

	case KEY_ESC:
	  if (KEY_ESC_arg == 'n')
	    {
	      edit_note ();
	      me.page = 9999;
	    }
	  break;

	case Ctrl ('B'):
	  m_read ();
	  me.page = 9999;
	  break;

	case 's':
	  AnnounceSelect ();
	  me.page = 9999;
	  break;

	case '\n':
	case '\r':
	case KEY_RIGHT:
	case 'r':
	case 'R':
	  if (me.now < me.num)
	    {
	      ITEM *node = me.item[me.now];
	      char *title = node->title;
	      int mode = title[1];
	      int update = (ch == 'R') ? 1 : 0;

	      title += 3;

	      if (mode == (char) 0xbc)
		{
		  int more_result;

		  go_proxy (fname, node, update);
		  strcpy (vetitle, title);
		  while (more_result = more (fname, YEA))
		    {
		      if (more_result == 1)
			{
			  if (--me.now < 0)
			    {
			      me.now = 0;
			      break;
			    }
			}
		      else if (more_result == 3)
			{
			  if (++me.now >= me.num)
			    {
			      me.now = me.num - 1;
			      break;
			    }
			}
		      else
			break;
		      node = me.item[me.now];
		      if (node->title[1] != (char) 0xbc)
			break;
		      // CityLion patch
                      if (me.now < me.page || me.now >= me.page + p_lines)
                        break;
		      go_proxy (fname, node, update);
		      strcpy (vetitle, title);
		    }
		}
	      else if (mode == (char) 0xbd)
		{
		  gem (title, node, update);
		}
	      me.page = 9999;
	    }
	  break;
	}
    }
  for (ch = 0; ch < me.num; ch++)
    free (me.item[ch]);
}


valid_dirname(char* fname)
{
   int l = strlen(fname);

   if (!strchr("MDSGH", *fname))
      return 0;
   if (fname[1] != '.')
      return 0;
   if (fname[l - 2] != '.')
      return 0;
   if (fname[l - 1] != 'A')
      return 0;
   return 1;
}


copy_stamp(char* fpath, char* fname)
{
  time_t dtime;
  char *ip = fpath + strlen(fpath) + 3;
  struct stat st;

  if (!valid_dirname(fname + 1)) 
  {
    fileheader fhdr;

    stampfile(fpath, &fhdr);
    return 0;
  }
  dtime = atoi(fname + 3);
  strcat(fpath, fname);
  do 
  {
    sprintf(ip, "%d.A", dtime++);
  } while (!stat(fpath, &st));
}


static void
a_timestamp (buf, time)
     char *buf;
     time_t *time;
{
  struct tm *pt = localtime (time);
  sprintf (buf, "%02d/%02d/%02d", pt->tm_mon + 1, pt->tm_mday, pt->tm_year % 100);
}


static void
a_additem (pm, myheader)
     AMENU *pm;
     fileheader *myheader;
{
  char buf[MAXPATHLEN];

  setadir (buf, pm->path);
  if (rec_add (buf, myheader, FHSZ) == -1)
    return;
  pm->now = pm->num++;
}


static void
a_loadname (pm)
     AMENU *pm;
{
  char buf[MAXPATHLEN];
  int len;

  setadir (buf, pm->path);
  len = get_records (buf, pm->header, FHSZ, pm->page + 1, p_lines);
  if (len < p_lines)
    bzero (&pm->header[len], FHSZ * (p_lines - len));
}
void
atitle ()
{
  showtitle ("精華文章", a_title);
  outs ("\
[←]離開 [→]閱\讀 [^P]發表文章 [b]備忘錄 [d]刪除 [q]精華區 [TAB]文摘 [^Z]求助\n\
" COLOR1 "\x1b[1m  編號   日 期  作  者       文  章  標  題\
                                  \x1b[m");
}

static int
a_showmenu (pm)
     AMENU *pm;
{
  char *title, *editor;
  int n;
  fileheader *item;
  char buf[MAXPATHLEN];
  time_t dtime;

  showtitle ("精華文章", pm->mtitle);
  prints ("   \x1b[1;32m編號    標      題%56s\x1b[0m", "編    選      日    期");

  if (pm->num)
    {
      setadir (buf, pm->path);
      a_loadname (pm);
      for (n = 0; n < p_lines && pm->page + n < pm->num; n++)
	{
#ifdef	HYPER_BBS
          char hbuf[256];	// HyperBBS
          sprintf(hbuf, "\x1b[200m\x1b[400m\x1b[444m\x1b[300m\x1b[%dm\x1b[%dm\x1b[%dm\x1b[%dm\x1b[%dm\x1b[613m\x1b[613m",
                  ((pm->page + n + 1)/10000)+648,
                  (((pm->page + n + 1)%10000)/1000)+648,
                  (((pm->page + n + 1)%1000)/100)+648,
                  (((pm->page + n + 1)%100)/10)+648,
                  ((pm->page + n + 1)%10)+648);
#endif
	  item = &pm->header[n];
	  title = item->title;
	  editor = item->owner;
	  sprintf (buf, "%s/%s", pm->path, item->filename);
	  dtime = dasht (buf);
	  a_timestamp (buf, &dtime);
#ifdef	HYPER_BBS
          prints ("\n%6d%s%s %-47.46s%-13s[%s]\x1b[201m",
            pm->page + n + 1, (item->filemode & FILE_REFUSE) ? ")" : ".",
            hbuf, title, editor, buf);
#else
          prints ("\n%6d%s %-47.46s%-13s[%s]",
            pm->page + n + 1, (item->filemode & FILE_REFUSE) ? ")" : ".",
            title, editor, buf);
#endif
	}
    }
  else
    {
      outs ("\n  \x1b[1;32m《\x1b[33m精華區\x1b[32m》\x1b[m板主打混中，或是徵求板主中");
    }

  move (b_lines, 0);
  outs (pm->level ?
	COLOR2 " 【板  主】 " COLOR1 "\x1b[1m \x1b[33m(Ctrl+Z)\x1b[37m說明  \x1b[33m(q/←)\x1b[37m離開  \
\x1b[33m(n)\x1b[37m新增文章  \x1b[33m(g)\x1b[37m新增目錄  \x1b[33m(e)\x1b[37m編輯檔案  \x1b[m" :
	COLOR2 " 【功\能鍵】 " COLOR1 "\x1b[1m \x1b[33m(Ctrl+Z)\x1b[37m說明  \x1b[34m(q/←)\x1b[37m離開  \
\x1b[33m(k↑j↓)\x1b[37m移動游標  \x1b[33m(enter/→)\x1b[37m讀取資料  \x1b[m");
}

/* ===== Added by mgtsai, Sep 10th, '96 ===== */
static void
a_refusemark (pm)
     AMENU *pm;
{
  char buf[256];
  fileheader item;

  memcpy (&item, &pm->header[pm->now - pm->page], FHSZ);
  item.filemode ^= FILE_REFUSE;
  setadir (buf, pm->path);
  substitute_record (buf, &item, FHSZ, pm->now + 1);
}


static void
a_moveitem (pm)
     AMENU *pm;
{
  fileheader *tmp;
  char newnum[4];
  int num, max, min;
  char buf[MAXPATHLEN];
  int fail;

  sprintf (buf, "請輸入第 %d 選項的新次序：", pm->now + 1);
  if (!getdata (b_lines - 1, 1, buf, newnum, 6, DOECHO, 0))
    return;
  num = (newnum[0] == '$') ? 9999 : atoi (newnum) - 1;
  if (num >= pm->num)
    num = pm->num - 1;
  else if (num < 0)
    num = 0;
  setadir (buf, pm->path);
  min = num < pm->now ? num : pm->now;
  max = num > pm->now ? num : pm->now;
  tmp = (fileheader *) calloc (max + 1, FHSZ);

  fail = 0;
  if (get_records (buf, tmp, FHSZ, 1, min) != min)
    fail = 1;
  if (num > pm->now)
    {
      if (get_records (buf, &tmp[min], FHSZ, pm->now + 2, max - min) != max - min)
	fail = 1;
      if (get_records (buf, &tmp[max], FHSZ, pm->now + 1, 1) != 1)
	fail = 1;
    }
  else
    {
      if (get_records (buf, &tmp[min], FHSZ, pm->now + 1, 1) != 1)
	fail = 1;
      if (get_records (buf, &tmp[min + 1], FHSZ, num + 1, max - min) != max - min)
	fail = 1;
    }
  if (!fail)
    substitute_record (buf, tmp, FHSZ * (max + 1), 1);
  pm->now = num;
  free (tmp);
}
static void
a_delrange (pm)
     AMENU *pm;
{
  char fname[256];
  sprintf (fname, "%s/.DIR", pm->path);
  del_range (0, NULL, fname);
  pm->num = rec_num (fname, FHSZ);
}

static void
a_delete (pm)
     AMENU *pm;
{
  char fpath[MAXPATHLEN], buf[MAXPATHLEN], cmd[MAXPATHLEN];
  char ans[4];
  fileheader backup;

  sprintf (fpath, "%s/%s", pm->path, pm->header[pm->now - pm->page].filename);
  setadir (buf, pm->path);

  if(pm->header[pm->now - pm->page].filemode & FILE_MARKED)
  {
    if(answer("此檔案/目錄不可刪除 , 是否要刪除索引 (y/N)")=='y')
    {
      if (rec_del (buf, FHSZ, pm->now + 1) == -1)
        return;
      pm->num--;
    }
    return;
  }
  if (pm->header[pm->now - pm->page].filename[0] == 'H' &&
      pm->header[pm->now - pm->page].filename[1] == '.')
    {
      getdata (b_lines - 1, 1, "您確定要刪除此精華區連線嗎(Y/N)？[N] "
	       ,ans, 3, LCECHO, 0);
      if (ans[0] != 'y')
	return;
      if (rec_del (buf, FHSZ, pm->now + 1) == -1)
	return;
    }
  else if (dashl (fpath))
    {
      getdata (b_lines - 1, 1, "您確定要刪除此 symbolic link 嗎(Y/N)？[N] ", ans, 3, LCECHO, 0);
      if (ans[0] != 'y')
	return;
      if (rec_del (buf, FHSZ, pm->now + 1) == -1)
	return;
      unlink (fpath);
    }
  else if (dashf (fpath))
    {
      getdata (b_lines - 1, 1, "您確定要刪除此檔案嗎(Y/N)？[N] ", ans, 3, LCECHO, 0);
      if (ans[0] != 'y')
	return;
      if (rec_del (buf, FHSZ, pm->now + 1) == -1)
	return;

      setbpath (buf, "deleted");
      stampfile (buf, &backup);
      strcpy (backup.owner, cuser.userid);
      strcpy (backup.title, pm->header[pm->now - pm->page].title + 2);
      backup.savemode = 'D';

      sprintf (cmd, "mv -f %s %s", fpath, buf);
      system (cmd);
      setbdir (buf, "deleted");
      rec_add (buf, &backup, sizeof (backup));
    }
  else if (dashd (fpath))
    {
      getdata (b_lines - 1, 1, "您確定要刪除整個目錄嗎(Y/N)？[N] ", ans, 3,
	       LCECHO, 0);
      if (ans[0] != 'y')
	return;
      if (rec_del (buf, FHSZ, pm->now + 1) == -1)
	return;
      sprintf (buf, "rm -rf %s", fpath);
      system (buf);
    }
  else
    /* Ptt 損毀的項目 */
    {
      getdata (b_lines - 1, 1, "您確定要刪除此損毀的項目嗎(Y/N)？[N] "
	       ,ans, 3, LCECHO, 0);
      if (ans[0] != 'y')
	return;
      if (rec_del (buf, FHSZ, pm->now + 1) == -1)
	return;
    }
  pm->num--;
}

load_paste ()
{
  struct stat st;
  FILE *fp;

  if (!*paste_fname)
    setuserfile (paste_fname, "paste_path");
  if (stat (paste_fname, &st) == 0 && st.st_mtime > paste_time
      && (fp = fopen (paste_fname, "r")))
    {
      int i;
      fgets (paste_path, MAXPATHLEN, fp);
      i = strlen (paste_path) - 1;
      if (paste_path[i] == '\n')
	paste_path[i] = 0;
      fgets (paste_title, STRLEN, fp);
      i = strlen (paste_title) - 1;
      if (paste_title[i] == '\n')
	paste_title[i] = 0;
      fscanf (fp, "%d", &paste_level);
      paste_time = st.st_mtime;
      fclose (fp);
    }
}

static void
a_pasteitem (pm)
     AMENU *pm;
{
  char newpath[MAXPATHLEN];
  char buf[MAXPATHLEN * 2];
  char ans[2];
  int i;
  fileheader item;

  if (copyfile[0])
  {
    if (dashd (copyfile))
    {
      for (i = 0; copyfile[i] && copyfile[i] == pm->path[i]; i++);
      
      if (!copyfile[i])
      {
	pressanykey ("將目錄拷進自己的子目錄中，會造成無窮迴圈！");
	return;
      }
    }

    sprintf (buf, "確定要拷貝[%s]嗎(Y/N)？[N] ", copytitle);
    getdata (b_lines - 1, 1, buf, ans, 3, LCECHO, 0);
    if (ans[0] == 'y')
    {
      char *fname = strrchr (copyfile, '/');
      strcpy (newpath, pm->path);

      strcpy(paste_path, pm->path);
      strcpy(paste_title, pm->mtitle);
      paste_level = pm->level;
      paste_time = time(0);

      if (*copyowner)
      {
/*
	if (fname)
	  strcat (newpath, fname);
	else
	  return;
*/
/* shakalaca.990515: mark 起來, 因為後面會作 timestamp, 不用原來的 filename
 * 如果不用 timestamp 檔名會重覆, 造成刪除任一個 copy 就會全部消施
 */

	if (access (pm->path, X_OK | R_OK | W_OK))
          mkdir (pm->path, 0755);
	  
	memset (&item, 0, sizeof (fileheader));
	strcpy (item.filename, fname + 1);

/*
	if (HAS_PERM (PERM_BBSADM))
	  f_cp (copyfile, newpath,O_TRUNC);
	else
	{
	  sprintf (buf, "/bin/cp %s %s", copyfile, newpath);
	  system (buf);
	}
*//* shakalaca.990515: 怎麼覺得兩個都一樣.. -_- */
      }

/*      else*/ if (dashf (copyfile))
/* shakalaca.990514: mark 起來.. */

      {
	stampfile (newpath, &item);
	unlink(newpath);
	memcpy (copytitle, "◇", 2);
	sprintf (buf, "/bin/cp %s %s", copyfile, newpath);
      }
      else if (dashd (copyfile))
      {
        stampdir (newpath, &item);
	memcpy (copytitle, "◆", 2);
	sprintf (buf, "/bin/cp -r %s/* %s/.D* %s", copyfile, copyfile, newpath);
      }
      else
      {
        outs ("無法拷貝！");
	igetch ();
	return; 
      }
      strcpy (item.owner, *copyowner ? copyowner : cuser.userid);
      strcpy (item.title, copytitle);
	
/*      if (!*copyowner) */
/* shakalaca.990515: 無論如何都要 system 吧 ?? */

        system (buf);
      a_additem (pm, &item);
      copyfile[0] = '\0';
    }
  }
  else
  {
    pressanykey ("請先執行 copy 命令後再 paste");
  }
}


static void
a_appenditem (pm)
     AMENU *pm;
{
  char fname[MAXPATHLEN];
  char buf[512];
  char ans[2];
  FILE *fp, *fin;

  move (b_lines - 1, 1);
  if (copyfile[0])
    {
      if (dashf (copyfile))
	{
	  sprintf (fname, "%s/%s", pm->path, pm->header[pm->now - pm->page].filename);
	  if (dashf (fname))
	    {
	      sprintf (buf, "確定要將[%s]附加於此嗎(Y/N)？[N] ", copytitle);
	      getdata (b_lines - 2, 1, buf, ans, 3, LCECHO, 0);
	      if (ans[0] == 'y')
		{
		  if (fp = fopen (fname, "a+"))
		    {
		      if (fin = fopen (copyfile, "r"))
			{
			  memset (buf, '-', 74);
			  buf[74] = '\0';
			  fprintf (fp, "\n> %s <\n\n", buf);
			  getdata (b_lines - 1, 1, "是否收錄簽名檔部份(Y/N)？[Y] ", ans, 3, LCECHO, 0);
			  while (fgets (buf, sizeof (buf), fin))
			    {
			      if ((ans[0] == 'n' || ans[0] == 'N') && !strcmp (buf, "--\n"))
				break;
			      fputs (buf, fp);
			    }
			  fclose (fin);
			  copyfile[0] = '\0';
			}
		      fclose (fp);
		    }
		}
	    }

	  else
	    {
	      outs ("檔案不得附加於此！");
	      igetch ();
	    }
	}

      else
	{
	  outs ("不得附加整個目錄於檔案後！");
	  igetch ();
	}
    }
  else
    {
      outs ("請先執行 copy 命令後再 append");
      igetch ();
    }
}


static void
a_forward (path, pitem, mode)
     char *path;
     fileheader *pitem;
     int mode;
{
  fileheader fhdr;

  strcpy (fhdr.filename, pitem->filename);
  strcpy (fhdr.title, pitem->title);
  switch (doforward (path, &fhdr, mode))
    {
    case 0:
      outmsg (msg_fwd_ok);
      break;
    case -1:
      outmsg (msg_fwd_err1);
      break;
    case -2:
      outmsg (msg_fwd_err2);
      break;
    }
}


/*
   woju, mgtsai
 */
static int
a_searchtitle (pm, rev)
     AMENU *pm;
     int rev;
{
  static char search_str[40] = "";
  int pos;

  getdata (b_lines - 1, 1, "[搜尋]關鍵字:", search_str, 40, DOECHO, 0);

  if (!*search_str)
    return pm->now;

  str_lower (search_str, search_str);

  rev = rev ? -1 : 1;
  pos = pm->now;
  do
    {
      pos += rev;
      if (pos == pm->num)
	pos = 0;
      else if (pos < 0)
	pos = pm->num - 1;
      if (pos < pm->page || pos >= pm->page + p_lines)
	{
	  pm->page = pos - pos % p_lines;
	  a_loadname (pm);
	}
      if (strstr_lower (pm->header[pos - pm->page].title, search_str))
	return pos;
    }
  while (pos != pm->now);
  return pm->now;
}


/* ===== Added by mgtsai, Sep 10th, '96 ===== */

static void
a_newtitle (pm)
     AMENU *pm;
{
  char buf[MAXPATHLEN];
  fileheader item;

  memcpy (&item, &pm->header[pm->now - pm->page], FHSZ);
  strcpy (buf, item.title + 3);
  if (getdata (b_lines - 1, 1, "新標題：", buf, 60, DOECHO, buf))
    {
      strcpy (item.title + 3, buf);
      setadir (buf, pm->path);
      substitute_record (buf, &item, FHSZ, pm->now + 1);
    }
}


static void
a_editsign (pm)
  AMENU *pm;
{
  char buf[MAXPATHLEN];
  fileheader item;

  memcpy (&item, &pm->header[pm->now - pm->page], FHSZ);
  sprintf (buf, "符號[%c%c]：", item.title[0], item.title[1]);
  if (getdata (b_lines - 1, 1, buf, buf, 5, DOECHO, 0))
    {
      item.title[0] = buf[0] ? buf[0] : ' ';
      item.title[1] = buf[1] ? buf[1] : ' ';
      item.title[2] = buf[2] ? buf[2] : ' ';
      setadir (buf, pm->path);
      substitute_record (buf, &item, FHSZ, pm->now + 1);
    }
}


static void
a_showname (pm)
     AMENU *pm;
{
  char buf[MAXPATHLEN];
  int len;
  int i;
  int sym;

  move (b_lines - 1, 1);
  sprintf (buf, "%s/%s", pm->path, pm->header[pm->now - pm->page].filename);
  if (dashl (buf))
    {
      prints ("此 symbolic link 名稱為 %s\n", pm->header[pm->now - pm->page].filename);
      if ((len = readlink (buf, buf, MAXPATHLEN - 1)) >= 0)
	{
	  buf[len] = '\0';
	  for (i = 0; BBSHOME[i] && buf[i] == BBSHOME[i]; i++);
	  if (!BBSHOME[i] && buf[i] == '/')
	    {
	      if (HAS_PERM (PERM_BBSADM))
		sym = 1;
	      else
		{
		  sym = 0;
		  for (i++; BBSHOME "/man"[i] && buf[i] == BBSHOME "/man"[i]; i++);
		  if (!BBSHOME "/man"[i] && buf[i] == '/')
		    sym = 1;
		}
	      if (sym)
		  pressanykey ("此 symbolic link 指向 %s", &buf[i + 1]);
	    }
	}
    }
  else if (dashf (buf))
    pressanykey ("此文章名稱為 %s", pm->header[pm->now - pm->page].filename);
  else if (dashd (buf))
    pressanykey ("此目錄名稱為 %s", pm->header[pm->now - pm->page].filename);
  else
    pressanykey ("此項目已損毀, 建議將其刪除！");
}


static void
a_newitem (pm, mode)
     AMENU *pm;
{
  static char *mesg[5] =
  {
    "[新增文章] 請輸入標題：",	/* ADDITEM */
    "[新增目錄] 請輸入標題：",	/* ADDGROUP */
    "[新增連線] 請輸入標題：",	/* ADDGOPHER */
    "請輸入標題：",		/* ADDLINK */
    "請輸入檔名："};		/* ADDFILE */

  char fpath[MAXPATHLEN], buf[MAXPATHLEN], lpath[MAXPATHLEN];
  fileheader item;
  int d;

  strcpy (fpath, pm->path);

  switch (mode)
  {
    case ADDITEM:
      stampfile (fpath, &item);
      strcpy (item.title, "◇ ");	/* A1BA */
      break;

    case ADDGROUP:
      stampdir (fpath, &item);
      strcpy (item.title, "◆ ");	/* A1BB */
      break;
    case ADDGOPHER:
      bzero (&item, sizeof (item));
      if (!getdata (b_lines - 2, 1, "輸入URL位址：", item.filename + 2, 61, DOECHO, 0))
	return;
      strcpy (item.title, "★ ");	/* A1BB */
      break;

    case ADDFILE:
      if (!getdata (b_lines - 2, 1, "新增檔案：", buf, 33, DOECHO, 0))
	return;
      if (invalid_pname (buf))
	{
	  pressanykey ("檔名不合法！");
	  return;
	}
      strcpy(item.filename,buf);
      strcpy (item.title, "Σ ");
      strcpy (item.date, "70");
      item.filemode ^= FILE_MARKED;      
      break;

    case ADDLINK:
      stamplink (fpath, &item);
      if (!getdata (b_lines - 2, 1, "新增連線：", buf, 61, DOECHO, 0))
	return;
      if (invalid_pname (buf))
      {
        unlink (fpath);
        pressanykey("目的地路徑不合法！");
        igetch ();
        return;
      }

      item.title[0] = 0;
      for (d = 0; d <= 4; d++)
      {
        switch (d)
        {
          case 0:
            sprintf (lpath, "%s%s%s/%s",
              BBSHOME, "/man/boards/", currboard, buf);
            break;
	  case 1:
	    sprintf (lpath, "%s%s%s", BBSHOME, "/man/boards/", buf);
	    break;
	  case 2:
	    sprintf (lpath, "%s%s%s", BBSHOME, "/boards/", buf);
	    break;
	  case 3:
	    sprintf (lpath, "%s%s%s", BBSHOME, "/etc/", buf);
	    break;
	  case 4:
	    sprintf (lpath, "%s%s%s", BBSHOME, "/", buf);
	    break;
	}
	if (dashf (lpath))
	{
	  strcpy (item.title, "☆ ");	/* A1B3 */
	  break;
	}
	else if (dashd (lpath))
	{
	  strcpy (item.title, "★ ");	/* A1B4 */
	  break;
	}
	if (!HAS_PERM (PERM_SYSOP) && d == 1)
	  break;
      } /* for (d = 0; d <= 4; d++) */

      if (!item.title[0])
      {
        unlink (fpath);
        outs ("目的地路徑不合法！");
        igetch ();
	return;
      }
  }

  if (!getdata (b_lines - 1, 1, mesg[mode], &item.title[3], 55, DOECHO, 0))
    {
      if (mode == ADDGROUP)
	rmdir (fpath);
      else if (mode != ADDGOPHER)
	unlink (fpath);
      return;
    }

  switch (mode)
  {
    case ADDITEM:
      buf[0] = '\0';	
      /* shakalaca.990527: 不然 buf 裡面是亂七八遭的東東. :p */
      if (vedit (buf, 0) == -1)
	{
	  unlink (buf);
	  pressanykey (NULL);
	  return;
	}
      f_mv (buf, fpath);
      break;

    case ADDLINK:
      unlink (fpath);
      if (symlink (lpath, fpath) == -1)
	{
	  outs ("無法建立 symbolic link");
	  igetch ();
	  return;
	}
      break;
    case ADDGOPHER:
      strcpy (item.date, "70");
      strncpy (item.filename, "H.", 2);
      break;
    }

  strcpy (item.owner, cuser.userid);
  a_additem (pm, &item);
}


a_copyitem (char *fpath, char *title, char *owner)
{
  strcpy (copyfile, fpath);
//  sprintf(copytitle, "%s%.72s",dashd(fpath) ? "◆ " : "◇ ",title);
  strcpy (copytitle, title);
  if (owner)
    strcpy (copyowner, owner);
  else
    *copyowner = 0;
  outz ("檔案標記完成。[注意] 拷貝後才能刪除原文!");
}

/* ===== end ===== */

int a_menu (maintitle, path, lastlevel, mode)
     char *maintitle;
     char *path;
     int lastlevel;
     int mode;
{
  static char Fexit;
  AMENU me;
  char fname[MAXPATHLEN];
  int ch;
  extern struct one_key read_comms[];

  trans_buffer[0] = 0;

  Fexit = 0;
  me.header = (fileheader *) calloc (p_lines, FHSZ);
  me.path = path;
  strcpy (me.mtitle, maintitle);
  setadir (fname, me.path);
  me.num = rec_num (fname, FHSZ);

  /* 精華區-tree 中部份結構屬於 cuser ==> BM */

  if (!(me.level = lastlevel))
    {
      char *ptr;

      if (ptr = strrchr (me.mtitle, '['))
	me.level = is_BM (ptr + 1);
    }

  me.page = 9999;
  me.now = 0;
  for (;;)
    {
      if (me.now >= me.num)
	me.now = me.num - 1;
      if (me.now < 0)
	me.now = 0;

      if (me.now < me.page || me.now >= me.page + p_lines)
	{
	  me.page = me.now - ((me.page == 10000 && me.now > p_lines / 2) ?
			      (p_lines / 2) : (me.now % p_lines));
	  a_showmenu (&me);
	}

      ch = cursor_key (2 + me.now - me.page, 0);

      if (ch == 'q' || ch == 'Q' || ch == KEY_LEFT)
	break;

      if (ch >= '1' && ch <= '9')
	{
	  if ((ch = search_num (ch, me.num)) != -1)
	    me.now = ch;
	  me.page = 10000;
	  continue;
	}

      switch (ch)
	{
	case KEY_UP:
	case 'k':
	  if (--me.now < 0)
	    me.now = me.num - 1;
	  break;

	case KEY_DOWN:
	case 'j':
	  if (++me.now >= me.num)
	    me.now = 0;
	  break;

	case KEY_PGUP:
	case Ctrl ('B'):
	  if (me.now >= p_lines)
	    me.now -= p_lines;
	  else if (me.now > 0)
	    me.now = 0;
	  else
	    me.now = me.num - 1;
	  break;

	case ' ':
	case KEY_PGDN:
	case Ctrl ('F'):
	  if (me.now < me.num - p_lines)
	    me.now += p_lines;
	  else if (me.now < me.num - 1)
	    me.now = me.num - 1;
	  else
	    me.now = 0;
	  break;

	case '0':
	  me.now = 0;
	  break;
	case '?':
	case '/':
	  me.now = a_searchtitle (&me, ch == '?');
	  me.page = 9999;
	  break;
	case '$':
	  me.now = me.num - 1;
	  break;

	case KEY_ESC:
	  if (KEY_ESC_arg == 'n')
	    {
	      edit_note ();
	      me.page = 9999;
	    }
	  break;

	case 's':
	  AnnounceSelect ();
	  me.page = 9999;
	  break;

	case 'e':
	case 'E':
	  sprintf (fname, "%s/%s", path, me.header[me.now - me.page].filename);
	  if(a_perm(fname,&me.header[me.now-me.page],&me) < 1)
	  {
	    pressanykey("沒有編輯權限");
	    break;
	  }
	  if (dashf (fname))
	    {
	      *quote_file = 0;
	      if (vedit (fname, (me.level >= MANAGER) ? 0 : 2) != -1)
		{
		  char fpath[200];
		  fileheader fhdr;

		  strcpy (fpath, path);
		  stampfile (fpath, &fhdr);
		  unlink (fpath);
		  f_mv (fname, fpath);
		  strcpy (me.header[me.now - me.page].filename, fhdr.filename);
		  strcpy (me.header[me.now - me.page].owner, cuser.userid);
		  setadir (fpath, path);
		  substitute_record (fpath, me.header + me.now - me.page, sizeof (fhdr), me.now + 1);
		}
	      me.page = 9999;
	    }
	  break;

	case 'P':
	  if (HAS_PERM (PERM_LOGINOK))
	    {
	      FILE *fp;

	      strcpy (paste_path, me.path);
	      strcpy (paste_title, me.mtitle);
	      paste_level = me.level;
	      if (!*paste_fname)
		setuserfile (paste_fname, "paste_path");
	      if (fp = fopen (paste_fname, "w"))
		{
		  fprintf (fp, "%.*s\n%.*s\n%d\n",
			   255, paste_path,
			   STRLEN - 1, paste_title,
			   paste_level);
		  fclose (fp);
		  pressanykey ("目前Paste目錄為 %s", paste_path);
		}
	    }
	  else
	    bell ();
	  break;

	case 'c':
	case 'C':
	case Ctrl ('C'):
	  if(me.level < MANAGER)
	  {
	    pressanykey("板主才可作複製動作");
	    break;
	  }
	  if (me.now < me.num)
	    {
	      fileheader *fhdr = me.header + me.now - me.page;

	      load_paste ();
	      sprintf (fname, "%s/%s", path, fhdr->filename);
	      if (ch == Ctrl ('C') && *paste_path && paste_level && dashf (fname))
		{
		  char newpath[256];
		  fileheader item;

		  strcpy (newpath, paste_path);
		  stampfile (newpath, &item);
		  unlink (newpath);
		  f_cp (fname, newpath,O_TRUNC);
		  strcpy (item.owner, fhdr->owner);
		  sprintf(item.title, "%s%.72s","◇ " , fhdr->title + 3);
//		  strcpy (item.title, fhdr->title + ((currutmp->pager > 1) ? 3 : 0));
		  strcpy (strrchr (newpath, '/') + 1, ".DIR");
		  rec_add (newpath, &item, FHSZ);
		  if (++me.now >= me.num)
		    me.now = 0;
		  break;
		}
	      a_copyitem (fname, fhdr->title , fhdr->owner);
	      if (ch == 'C' && *paste_path)
		a_menu (paste_title, paste_path, paste_level, ANNOUNCE);
	      me.page = 9999;
	    }
	  break;

	case Ctrl ('S'):
	  if (me.now < me.num)
	    {
	      fileheader *fhdr = me.header + me.now - me.page;
	      sprintf (fname, "%s/%s", path, fhdr->filename);
	      if (save_mail (0, fhdr, fname) == POS_NEXT)
		if (++me.now >= me.num)
		  me.now = 0;
	    }
	  break;

	case '\n':
	case '\r':
	case KEY_RIGHT:
	case 'r':
	case 'R':
	  if (me.now < me.num)
	  {
	    int mode;
	    fileheader *fhdr = &me.header[me.now - me.page];

	    sprintf (fname, "%s/%s", path, fhdr->filename);
	    mode = a_perm(fname,fhdr,&me);
	    if(mode < 1)
	    {
	      pressanykey ("這是私人重地 , 你無法進入 , 有需要請向板主申請");
	      me.page = 9999;
	      break;
	    }	      
	    else if(mode >1)
	    {
	      if(answer("是否編輯可看見名單? (y/N)") == 'y')
	      {
	        char buf[MAXPATHLEN];
	        if(dashf(fname))
	          sprintf(buf,"%s.vis",fname);
	        else
	          sprintf(buf,"%s/.vis",fname);
	        ListEdit(buf);
	      }
	    }
	    if (*fhdr->filename == 'H' && fhdr->filename[1] == '.')
	    {
	      ITEM item;
	      strcpy (item.X.G.server, fhdr->filename + 2);
	      strcpy (item.X.G.path, "1/");
	      item.X.G.port = 70;
	      gem (fhdr->title, &item, (ch == 'R') ? 1 : 0);
	    }
	    else if (dashf (fname))
	    {
	      int more_result;
	      while (more_result = more (fname, YEA))
	      {
/*Ptt 範本精靈plugin */
		      if (strstr (fname, "etc/editexp/") || strstr (fname, "etc/SONGBOOK/"))
			{
			  char ans[4];
			  move (22, 0);
			  clrtoeol ();
			  getdata (22, 1,
				   "要把範例 Plugin 到文章嗎?[y/N]"
				   ,ans, 3, LCECHO, 0);
			  if (ans[0] == 'y')
			    {
			      strcpy (trans_buffer, fname);
			      Fexit = 1;
			      free (me.header);
			      return 0;
			    }
			}
		      if (more_result == 1)
			{
                            int more_ok=0;
                            me.now-=1;
                            if (me.now < 0)
			    {
			      me.now = me.num - 1;
			      break;
			    }
                         do{
                            fhdr = &me.header[me.now-me.page];
                            if((fhdr->filemode & FILE_REFUSE) &&
                               (me.level < MANAGER))
                               more_ok=0;
                            else
                               more_ok=1;
                          }while(more_ok!=1 && --me.now >=0 );
                          if (me.now <0 )
                          {
                                me.now = me.num - 1;
                                break;
                          }
			    	
			}
		      else if (more_result == 3)
			{
                         int more_ok=0;
                         me.now+=1;
                         if (me.now >= me.num)
                         {
                                me.now = 0;
                                break;
                          }
                         do{
                            fhdr = &me.header[me.now-me.page];
                            if((fhdr->filemode & FILE_REFUSE) &&
                               (me.level < MANAGER))
                               more_ok=0;
                            else
                               more_ok=1;
                          }while(more_ok!=1 && ++me.now < me.num );
                          if (me.now >= me.num)
                          {
                                me.now = 0;
                                break;
                          }
			}
		      else
			break;

                      // CityLion patch
                      if (me.now < me.page || me.now >= me.page + p_lines)
                        break;

                      sprintf (fname, "%s/%s", path, me.header[me.now - me.page].filename);
                      if (a_perm(fname, &me.header[me.now - me.page], &me) < 1 || !dashf (fname))
			break;
		    }
		}
	      else if (dashd (fname))
		{
		  a_menu (me.header[me.now - me.page].title, fname, me.level);
		  /* Ptt *//* 強力跳出recursive */
		  if (Fexit)
		    {
		      free (me.header);
		      return 0;
		    }
		}
	      me.page = 9999;
	    }
	  break;

	case 'F':
	case 'U':
	  sprintf (fname, "%s/%s", path, me.header[me.now - me.page].filename);
	  if(a_perm(fname,&me.header[me.now-me.page],&me) < 1)
	  {
	    pressanykey("沒有轉寄權限");
	    break;
	  }
	  if (me.now < me.num && HAS_PERM (PERM_BASIC) && dashf (fname))
	    {
            a_forward (path, &me.header[me.now - me.page], ch == 'U');
	    }
	  else
	    outz ("無法轉寄此項目");

	  me.page = 9999;
	  refresh ();
	  break;
	}

      if (me.level >= MANAGER)
	{
	  int page0 = me.page;

	  switch (ch)
	    {
	    case 'n':
	      a_newitem (&me, ADDITEM);
	      me.page = 9999;
	      break;
	    case 'g':
	      a_newitem (&me, ADDGROUP);
	      me.page = 9999;
	      break;
	    case 'G':
	      a_newitem (&me, ADDGOPHER);
	      me.page = 9999;
	      break;
	    case 'p':
	      a_pasteitem (&me);
	      me.page = 9999;
	      break;
	    case 'a':
	      a_appenditem (&me);
	      me.page = 9999;
	      break;
	    default:
	      me.page = page0;
	      break;
	    }

	  if (me.num)
	    switch (ch)
	      {
	      case 'm':
		a_moveitem (&me);
		me.page = 9999;
		break;

	      case 'D':
/* Ptt       me.page = -1; */
		a_delrange (&me);
		me.page = 9999;
		break;
	      case 'd':
		a_delete (&me);
		me.page = 9999;
		break;

	      case 't':
		a_newtitle (&me);
		me.page = 9999;
		break;
	      case 'o':
		a_refusemark (&me);
		me.page = 9999;
		break;
	      }
	}

      if (me.level == SYSOP)
	{
	  switch (ch)
	    {
	    case 'f':
	      a_editsign (&me);
	      me.page = 9999;
	      break;
	    case 'l':
	      a_newitem (&me, ADDLINK);
	      me.page = 9999;
	      break;
	    case 'T':
	      a_newitem (&me, ADDFILE);
	      me.page = 9999;
	      break;
	    case 'N':
	      a_showname (&me);
	      me.page = 9999;
	      break;
	    case 'B':
	      setadir (fname, me.path);
	      a_title = maintitle;
	      i_read (0, fname, atitle, doent, read_comms, NULL);
	      me.page = 9999;
	      break;
	    }
	}
    }

  free (me.header);
}



AnnounceSelect ()
{
  static char xboard[20];
  char buf[20];
  char fpath[256];
  boardheader *bp;
  boardheader *getbcache ();

  move (2, 0);
  clrtoeol ();
  move (3, 0);
  clrtoeol ();
  move (1, 0);
  make_blist ();
  namecomplete ("選擇精華區看板：", buf);
  if (*buf)
    strcpy (xboard, buf);
  if (*xboard && (bp = getbcache (xboard)))
    {
      setapath (fpath, xboard);
      setutmpmode (ANNOUNCE);
      a_menu (xboard, fpath, (HAS_PERM (PERM_ALLBOARD) || HAS_PERM (PERM_BM) && is_BM (bp->BM)) ? 1 : 0);
    }
  return RC_FULL;
}


int
Announce ()
{
  setutmpmode (ANNOUNCE);
  a_menu (mytitle, "man",
    (HAS_PERM (PERM_SYSOP) ? SYSOP : NOBODY));
  return 0;
}

int
Log ()
{
  setutmpmode (LOG);
  a_menu (mytitle, "man/log", (HAS_PERM (PERM_SYSOP) ? SYSOP : NOBODY));
  return 0;
}

int
XFile ()
{
  setutmpmode (ANNOUNCE);
  a_menu (mytitle, "etc/xfile",
	  (HAS_PERM (PERM_SYSOP) ? SYSOP : HAS_PERM (PERM_XFILE) ? MANAGER : NOBODY));
  return 0;
}

int
HELP ()
{
  setutmpmode (ANNOUNCE);
  counter(BBSHOME"/log/counter/count-HELP","使用 HELP 選單",0);
  a_menu (mytitle, "etc/help",
	  (HAS_PERM (PERM_SYSOP) ? SYSOP : HAS_PERM (PERM_XFILE) ? MANAGER : NOBODY));
  return 0;
}

char genbuf[255];

int
user_gem(char *uid)
{
  sethomefile(genbuf, uid, "gem");
  if(!dashd(genbuf))
    mkdir(genbuf, 0755);
  a_menu("私人精華文章",genbuf,
    HAS_PERM(PERM_SYSOP) ? 2 : !strcmp(cuser.userid,uid) ? 1 : 0);
  return 0;
}

int
user_allpost(char *uid)
{
  sethomefile(genbuf, uid, "allpost");
  if(!dashd(genbuf))
    mkdir(genbuf, 0755);
  if(HAS_PERM(PERM_SYSOP) || !strcmp(cuser.userid,uid))
    a_menu("張貼文章",genbuf,
      HAS_PERM(PERM_SYSOP) ? 2 : !strcmp(cuser.userid,uid) ? 1 : 0);
  else
    pressanykey("限制進入");
  return 0;
}

void
my_gem()
{
  more(BBSHOME"/etc/my_gem",YEA);
  user_gem(cuser.userid);
}

void
my_allpost()
{
  more(BBSHOME"/etc/my_allpost",YEA);
  user_allpost(cuser.userid);
}
