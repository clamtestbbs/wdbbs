/*-------------------------------------------------------*/
/* edit.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : simple ANSI/Chinese editor                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"

#define KEEP_EDITING    -2
#define SCR_WIDTH       80

enum
{
  NOBODY, MANAGER, SYSOP
};

textline *firstline = NULL;
textline *lastline = NULL;
textline *currline = NULL;
textline *blockline = NULL;
textline *top_of_win = NULL;
textline *deleted_lines = NULL;

extern int local_article;
extern char real_name[20];
char line[WRAPMARGIN  + 2];
int currpnt, currln, totaln;
int curr_window_line;
int redraw_everything;
int insert_character;
int my_ansimode;
int raw_mode;
int phone_mode = 0;
int edit_margin;
int blockln  = -1;
int blockpnt;
int prevln = -1;
int prevpnt;
int line_dirty;
int indent_mode;
int insert_c = ' ';
int star_special_signal=0;
int star_special_mode=0;
int star_special_num=16;

char fp_bak[] = "bak";
char *star_key[2]={
COLOR1"\x1b[1m 符號   A.標點  B.標線  C.數學  D.單位  E.注音1 F.注音2 G.英文s H.英文l\x1b[m",
COLOR1"\x1b[1m         A B C D E F G H I J K L M N O P Q R S T U V X W Y Z 1 2 3 4 5 6 7 \x1b[m"
};

char *star_msg[17] ={
COLOR1"\x1b[1m 選單   I.希臘s J.希臘l K.數字  L.圖塊  M.箭頭  N.括弧  O.標記  P.表格   \x1b[m",
" [標點 ] ，；：、﹑。？！‧﹗（）「」『』‘’“”〝〞‵′                ",
" [標線 ] …‥﹉﹍﹌﹏\︴　＿—∥∣▕／＼╳╱╲∕﹨                       ",
" [數學 ] ＋－×÷√±＝≡≠≒≦≧＜＞∵∴∞～∩∪∫∮＆⊥∠∟⊿﹢﹣﹤﹥﹦",
" [單位 ] ﹩＄￥〒￠￡ぽ﹪℅％㏕㎜㎝㎞㏎㎡㎎㎏㏄°℃℉㏒㏑兞兝\兣兙兡兛嗧瓩糎",
" [注音1] ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙㄧㄨㄩㄚㄛㄜㄝㄞㄟㄠㄡㄢ",
" [注音2] ㄣㄤㄥㄦ˙ˊˇˋ                                               ",
" [英文S] ａｂｃｄｅｆｇｊｈｉｌｋｍｎｏｐｑｒｓｔｕｖｗｘｙｚ           ",
" [英文L] ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ           ",
" [希臘S] α\βγδㄒaηθικλμνξοπρστυφχψω               ",
" [希臘L] ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ                ",
" [數字 ] １２３４５６７８９０〡〢〣〤〥〦〧〨〩十卄卅ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩ",
" [圖塊 ] ▁▂▃▄▅▆▇█◢◣▏▎▍▌▋▊▉▓◥◤                        ",
" [箭頭 ] ↑↓←→↖↗↙↘                                                ",
" [括弧 ] 【】「」『』〈〉《》〔〕｛｝︵﹛g︺︷︸︻︼︿﹀︽︾﹁﹂﹃﹄    ",
" [標記 ] ○⊙◎●☆★□■▼▲▽△◇◆♀♂﹡＊※§＠⊕㊣                  ",
" [表格 ] ├╩┘┤┐╠╬╣╮╰╯│─╝    ┌╔┼╦╭╚┬┴╗└╭╮╰╯    "
};

char *my_edit_mode[2] =
{"取代", "插入"};

char save_title[STRLEN];


int star_ime(int key,char *ans1,char *ans2)
{
  int star_exit;
  int starchar;
  star_exit=0;
  starchar=0;
  if(key=='?' || key=='/')
  {
    screenline *screen;
    int i;
    screen = (screenline *)calloc(t_lines, sizeof(screenline));
    vs_save(screen);
    clrchyiuan(0,23);
    for(i=1;i<star_special_num;i++)
    {
      move(i+2,0);
      prints("%s\x1b[0m",star_msg[i]);
    }
    pressanykey("星空之給我放自然一點輸入法一覽表");
    vs_restore(screen);
    free(screen);
    redoscr();
    *ans1=0;
    *ans2=0;
                return 2;
        }
        else if(key=='+' || key=='=')
        {
                if(star_special_mode<star_special_num)
                        star_special_mode=star_special_mode+1;
                else
                        star_special_mode=1;
                *ans1=0;
                *ans2=0;
                return 2;
        }
        else if(key=='_' || key=='-')
        {
                if(star_special_mode>1)
                        star_special_mode=star_special_mode-1;
                else
                        star_special_mode=star_special_num;
                *ans1=0;
                *ans2=0;
                return 2;
        }
        else if(star_special_mode > 0)
        {
                if(key>='A' && key<='Z')
                {
                        key=key-'A';
                        *ans1=star_msg[star_special_mode][key*2+9];
                        *ans2=star_msg[star_special_mode][key*2+10];
                        return 1;
                }
                else if(key>='a' && key<='z')
                {
                        key=key-'a';
                        *ans1=star_msg[star_special_mode][key*2+9];
                        *ans2=star_msg[star_special_mode][key*2+10];
                        return 1;
                }
                else if(key>='1' && key <='7')
                {
                        key=key-23;
                        *ans1=star_msg[star_special_mode][key*2+9];
                        *ans2=star_msg[star_special_mode][key*2+10];
                        return 1;
                }
                else if(key=='~' || key=='`')
                {
                        star_special_mode=0;
                        *ans1=0;
                        *ans2=0;
                        return 2;
                }

                else
                {
                        star_special_mode=star_special_mode;
                        *ans1=0;
                        *ans2=0;
                        return 0;
                }
        }
        else if(star_special_mode==0)
        {
                if(key>='A' && key<'A'+star_special_num)
                {
                        key=key-'A';
                        star_special_mode=key+1;
                        *ans1=0;
                        *ans2=0;
                        return 2;
                }
                else if(key>='a' && key<'a'+star_special_num)
                {
                        key=key-'a';
                        star_special_mode=key+1;
                        *ans1=0;
                        *ans2=0;
                        return 2;
                }
                else
                {
                        *ans1=0;
                        *ans2=0;
                        return 0;
                }
        }
        else
        {
                *ans1=0;
                *ans2=0;
                return 0;
        }
}

int
check_local(int i)
{
return i;
}


beginsig(char* s)
{
   char buf[10];

   strncpy(buf, s, 9);
   buf[9] = 0;
   strtok(buf, "\n\r");
   trim2(buf);
   return !strcmp(buf, "--");
}

/* ----------------------------------------------------- */
/* 記憶體管理與編輯處理                                  */
/* ----------------------------------------------------- */

static void
indigestion(i)
{
  fprintf(stderr, "嚴重內傷 %d\n", i);
}

/* ----------------------------------------------------- */
/* Thor: ansi 座標轉換  for color 編輯模式               */
/* ----------------------------------------------------- */


static int
ansi2n(int ansix, textline * line)
{
  register char *data, *tmp;
  register char ch;

  data = tmp = line->data;

  while(*tmp) {
    if (*tmp == KEY_ESC)
    {
      while((ch = *tmp) && !isalpha(ch))
        tmp++;
      if (ch)
        tmp++;
      continue;
    }
    if (ansix <= 0) break;
    tmp++;
    ansix--;
  }
  return tmp - data;
}

static int
n2ansi(int nx, textline * line)
{
  register ansix = 0;
  register char *tmp,*nxp;
  register char ch;

  tmp = nxp = line->data;
  nxp += nx;

  while(*tmp){
    if (*tmp == KEY_ESC)
    {
      while((ch = *tmp) && !isalpha(ch))
        tmp++;
      if (ch)
        tmp++;
      continue;
    }
    if (tmp >= nxp) break;
    tmp++;
    ansix++;
  }
  return ansix;
}


/* ----------------------------------------------------- */
/* 螢幕處理：輔助訊息、顯示編輯內容                      */
/* ----------------------------------------------------- */

static void
edit_msg(void)
{
  static char *edit_mode[2] = {"取代", "插入"};
  register int n = currpnt;

  if (my_ansimode)                      /* Thor: 作 ansi 編輯 */
    n = n2ansi(n, currline);
  n++;
  move(b_lines, 0);
  clrtoeol();
  prints(COLOR2" 編輯文章 \x1b[1m"COLOR1" \x1b[33m(Ctrl-Z)\x1b[37m輔助說明 \
\x1b[33m(^G)\x1b[37m插入圖文庫 \x1b[33m(^X,^Q) \x1b[37m離開║%s│%c%c%c%c║ %3d:%3d  \x1b[m",
    edit_mode[insert_character],
    my_ansimode ? 'A' : 'a', indent_mode ? 'I' : 'i',
    phone_mode ? 'P' : 'p', raw_mode ? 'R' : 'r',
    currln + 1, n);
}


static textline *
back_line(pos, num)
  register textline *pos;
  int num;
{
  while (num-- > 0)
  {
    register textline *item;

    if (pos && (item = pos->prev))
    {
      pos = item;
      currln--;
    }
  }
  return pos;
}


static textline *
forward_line(pos, num)
  register textline *pos;
  int num;
{
  while (num-- > 0)
  {
    register textline *item;

    if (pos && (item = pos->next))
    {
      pos = item;
      currln++;
    }
  }
  return pos;
}


static int
getlineno()
{
  int cnt = 0;
  textline *p = currline;

  while (p && (p != top_of_win))
  {
    cnt++;
    p = p->prev;
  }
  return cnt;
}


static char *
killsp(s)
  char *s;
{
  while (*s == ' ')
    s++;
  return s;
}


static textline *
alloc_line()
{
  extern void *malloc();
  register textline *p;

  if (p = (textline *) malloc(sizeof(textline)))
  {
    memset(p, 0, sizeof(textline));
    return p;
  }

  indigestion(13);
  abort_bbs();
}


/* ----------------------------------------------------- */
/* append p after line in list. keeps up with last line  */
/* ----------------------------------------------------- */

static void
append(p, line)
  register textline *p, *line;
{
  register textline *n;

  if (p->next = n = line->next)
    n->prev = p;
  else
    lastline = p;
  line->next = p;
  p->prev = line;
}


/* ----------------------------------------------------- */
/* delete_line deletes 'line' from the list,             */
/* and maintains the lastline, and firstline pointers.   */
/* ----------------------------------------------------- */

static void
delete_line(line)
  register textline *line;
{
  register textline *p = line->prev;
  register textline *n = line->next;

  if (!p && !n)
  {
    line->data[0] = line->len = 0;
    return;
  }
  if (n)
    n->prev = p;
  else
    lastline = p;
  if (p)
    p->next = n;
  else
    firstline = n;
  strcat(line->data, "\n");
  line->prev = deleted_lines;
  deleted_lines = line;
  totaln--;
}


int
ask (prompt)
     char *prompt;
{
  int ch;

  move (0, 0);
  clrtoeol ();
  standout ();
  prints ("%s", prompt);
  standend ();
  ch = igetkey ();
  move (0, 0);
  clrtoeol ();
  return (ch);
}


/*
woju
*/

undelete_line()
{
   textline* p = deleted_lines;

   textline* currline0 = currline;
   textline* top_of_win0 = top_of_win;
   int currpnt0 = currpnt;
   int currln0 = currln;
   int curr_window_line0 = curr_window_line;
   int indent_mode0 = indent_mode;

   if (!deleted_lines)
      return 0;

   indent_mode = 0;
   insert_string(deleted_lines->data);
   indent_mode = indent_mode0;
   deleted_lines = deleted_lines->prev;
   free(p);

   currline = currline0;
   top_of_win = top_of_win0;
   currpnt = currpnt0;
   currln = currln0;
   curr_window_line = curr_window_line0;
}


int
indent_spcs()
{
   textline* p;
   int spcs;

   if (!indent_mode)
      return 0;

   for (p = currline; p; p = p->prev) {
      for (spcs = 0; p->data[spcs] == ' '; ++spcs)
         ;
      if (p->data[spcs])
         return spcs;
   }
   return 0;
}

/* ----------------------------------------------------- */
/* split 'line' right before the character pos           */
/* ----------------------------------------------------- */

static void
split(line, pos)
  register textline *line;
  register int pos;
{
  if (pos <= line->len)
  {
    register textline *p = alloc_line();
    register char *ptr;
    int spcs = indent_spcs();

    totaln++;

    p->len = line->len - pos + spcs;
    line->len = pos;

    memset(p->data, ' ', spcs);
    p->data[spcs] = 0;
    strcat(p->data, (ptr = line->data + pos));
    ptr[0] = '\0';
    append(p, line);
    if (line == currline && pos <= currpnt)
    {
      currline = p;
      if (pos == currpnt)
         currpnt = spcs;
      else
         currpnt -= pos;
      curr_window_line++;
      currln++;
    }
    redraw_everything = YEA;
  }
}



/* ----------------------------------------------------- */
/* 1) lines were joined and one was deleted              */
/* 2) lines could not be joined                          */
/* 3) next line is empty                                 */
/* returns false if:                                     */
/* 1) Some of the joined line wrapped                    */
/* ----------------------------------------------------- */

static int
join(line)
  register textline *line;
{
  register textline *n;
  register int ovfl;

  if (!(n = line->next))
    return YEA;
  if (!*killsp(n->data))
    return YEA;

  ovfl = line->len + n->len - WRAPMARGIN;
  if (ovfl < 0)
  {
    strcat(line->data, n->data);
    line->len += n->len;
    delete_line(n);
    return YEA;
  }
  else
  {
    register char *s;

    s = n->data + n->len - ovfl - 1;
    while (s != n->data && *s == ' ')
      s--;
    while (s != n->data && *s != ' ')
      s--;
    if (s == n->data)
      return YEA;
    split(n, (s - n->data) + 1);
    if (line->len + n->len >= WRAPMARGIN)
    {
      indigestion(0);
      return YEA;
    }
    join(line);
    n = line->next;
    ovfl = n->len - 1;
    if (ovfl >= 0 && ovfl < WRAPMARGIN - 2)
    {
      s = &(n->data[ovfl]);
      if (*s != ' ')
      {
        strcpy(s, " ");
        n->len++;
      }
    }
    return NA;
  }
}


static void
insert_char(ch)
  register int ch;
{
  register textline *p = currline;
  register int i = p->len;
  register char *s;
  int wordwrap = YEA;

  if (currpnt > i)
  {
    indigestion(1);
    return;
  }
  if (currpnt < i && !insert_character)
  {
    p->data[currpnt++] = ch;
    if (my_ansimode) /* Thor: ansi 編輯, 可以overwrite, 不蓋到 ansi code */
       currpnt = ansi2n(n2ansi(currpnt, p),p);
  }
  else
  {
    while (i >= currpnt)
    {
      p->data[i + 1] = p->data[i];
      i--;
    }
    p->data[currpnt++] = ch;
    i = ++(p->len);
  }
  if (i < WRAPMARGIN)
    return;
  s = p->data + (i - 1);
  while (s != p->data && *s == ' ')
    s--;
  while (s != p->data && *s != ' ')
    s--;
  if (s == p->data)
  {
    wordwrap = NA;
    s = p->data + (i - 2);
  }
  split(p, (s - p->data) + 1);
  p = p->next;
  i = p->len;
  if (wordwrap && i >= 1)
  {
    if (p->data[i - 1] != ' ')
    {
      p->data[i] = ' ';
      p->data[i + 1] = '\0';
      p->len++;
    }
  }
}


insert_string(str)
  char *str;
{
  int ch;

  while (ch = *str++)
  {

#ifdef BIT8
    if (isprint2(ch) || ch == '\x1b')
#else
    if (isprint(ch))
#endif

    {
      insert_char(ch);
    }
    else if (ch == '\t')
    {
      do
      {
        insert_char(' ');
      } while (currpnt & 0x3);
    }
    else if (ch == '\n')
      split(currline, currpnt);
  }
}


static void
delete_char()
{
  register int len;

  if (len = currline->len)
  {
    register int i;
    register char *s;

    if (currpnt >= len)
    {
      indigestion(1);
      return;
    }
    for (i = currpnt, s = currline->data + i; i != len; i++, s++)
      s[0] = s[1];
    currline->len--;
  }
}


static void
load_file(fp)
  FILE *fp;
{
  int indent_mode0 = indent_mode;

  indent_mode = 0;
  while (fgets(line, WRAPMARGIN + 2, fp))
    insert_string(line);
  fclose(fp);
  indent_mode = indent_mode0;
}


/* ----------------------------------------------------- */
/* 暫存檔                                                */
/* ----------------------------------------------------- */

char *
ask_tmpbuf(int y)
{
  static char fp_buf[10] = "buf.0";
  static char msg[] = "請選擇暫存檔 (0-9)[0]: ";

  msg[19] = fp_buf[4];
  do
  {
    if (!getdata(y, 0, msg, fp_buf + 4, 4, DOECHO,0))
      fp_buf[4] = msg[19];
  } while (fp_buf[4] < '0' || fp_buf[4] > '9');
  return fp_buf;
}


static void
read_tmpbuf(int n)
{
  FILE *fp;
  char fp_tmpbuf[80];
  char tmpfname[] = "buf.0";
  char *tmpf;
  char ans[4] = "y";

  if (0 <= n && n <= 9) {
     tmpfname[4] = '0' + n;
     tmpf = tmpfname;
  }
  else {
     tmpf = ask_tmpbuf(3);
     n = tmpf[4] - '0';
  }

  setuserfile(fp_tmpbuf, tmpf);

/*
woju
*/
  if (n != 0 && n != 5 && more(fp_tmpbuf, NA) != -1)
     getdata(b_lines - 1, 0, "確定讀入嗎(Y/N)?[Y]",  ans, 4, LCECHO,0);
  if (*ans != 'n' && (fp = fopen(fp_tmpbuf, "r")))
  {
    prevln = currln;
    prevpnt = currpnt;
    load_file(fp);
    while (curr_window_line >= b_lines)
    {
      curr_window_line--;
      top_of_win = top_of_win->next;
    }
  }
}


static void
write_tmpbuf()
{
  FILE *fp;
  char fp_tmpbuf[80], ans[4];
  textline *p;

  setuserfile(fp_tmpbuf, ask_tmpbuf(3));
  if (dashf(fp_tmpbuf))
  {
    more(fp_tmpbuf, NA);
    getdata(b_lines - 1, 0, "暫存檔已有資料 (A)附加 (W)覆寫 (Q)取消？[A] ",
      ans, 4, LCECHO,0);

    if (ans[0] == 'q')
      return;
  }

  fp = fopen(fp_tmpbuf, (ans[0] == 'w' ? "w" : "a+"));
  for (p = firstline; p; p = p->next)
  {
    if (p->next || p->data[0])
      fprintf(fp, "%s\n", p->data);
  }
  fclose(fp);
}


static void
erase_tmpbuf()
{
  char fp_tmpbuf[80];
  char ans[4] = "n";

  setuserfile(fp_tmpbuf, ask_tmpbuf(3));
/*
woju
*/
  if (more(fp_tmpbuf, NA) != -1)
     getdata(b_lines - 1, 0, "確定刪除嗎(Y/N)?[N]",  ans, 4, LCECHO,0);
  if (*ans == 'y')
     unlink(fp_tmpbuf);
}


/* ----------------------------------------------------- */
/* 編輯器自動備份                                        */
/* ----------------------------------------------------- */


void
auto_backup()
{
  if (currline)
  {
    FILE *fp;
    textline *p, *v;
    char bakfile[64];

    setuserfile(bakfile, fp_bak);
    if (fp = fopen(bakfile, "w"))
    {
      for (p = firstline; p; p = v)
      {
        v = p->next;
        fprintf(fp, "%s\n", p->data);
        free(p);
      }
      fclose(fp);
    }
    currline = NULL;
  }
}


void
restore_backup()
{
  char bakfile[80], buf[80];

  setuserfile(bakfile, fp_bak);
  if (dashf(bakfile))
  {
    stand_title("編輯器自動復原");
    getdata(1, 0, "您有一篇文章尚未完成，(S)寫入暫存檔 (Q)算了？[S] ",
      buf, 4, LCECHO,"S");
    if (buf[0] != 'q')
    {
      setuserfile(buf, ask_tmpbuf(3));
      f_mv(bakfile, buf);
    }
    else
      unlink(bakfile);
  }
}


/* ----------------------------------------------------- */
/* 引用文章                                              */
/* ----------------------------------------------------- */


static int
garbage_line(str)               /* quote deletion */
  char *str;
{
  int qlevel = 0;

  while (*str == ':' || *str == '>')
  {
    if (*(++str) == ' ')
      str++;
    if (qlevel++ >= 1)
      return 1;
  }
  while (*str == ' ' || *str == '\t')
    str++;
  if (qlevel >= 1)
  {
    if (!strncmp(str, "※ ", 3) || !strncmp(str, "==>", 3) ||
      strstr(str, ") 提到:\n"))
      return 1;
  }
  return (*str == '\n');
}


static void
do_quote()
{
  int op;
  char buf[512];

  getdata(b_lines - 1, 0, "請問要引用原文嗎(Y/N/All/Repost)？[Y] ", buf, 3, LCECHO,"Y");
  op = buf[0];

  if (op != 'n')
  {
    FILE *inf;

    if (inf = fopen(quote_file, "r"))
    {
      char *ptr;
      int indent_mode0 = indent_mode;

      fgets(buf, 512, inf);
      if (ptr = strrchr(buf, ')'))
        ptr[1] = '\0';
      else if (ptr = strrchr(buf, '\n'))
        ptr[0] = '\0';

      if (ptr = strchr(buf, ':'))
      {
        char *str;

        while (*(++ptr) == ' ');

        /* 順手牽羊，取得 author's address */
        if ((curredit & EDIT_BOTH) && (str = strchr(quote_user, '.')))
        {
          strcpy(++str, ptr);
          str = strchr(str, ' ');
          str[0] = '\0';
        }
      }
      else
        ptr = quote_user;

      indent_mode = 0;
      insert_string("※ 引述《");
      insert_string(ptr);
      insert_string("》之銘言：\n");

      if (op != 'a')            /* 去掉 header */
      {
        while (fgets(buf, 512, inf) && buf[0] != '\n');
      }

      if (op == 'a')
      {
        while (fgets(buf, 512, inf))
        {
          insert_char(':');
          insert_char(' ');
          insert_string(Ptt_prints(buf,ONLY_COLOR));
        }
      }
      else if (op == 'r')
      {
        while (fgets(buf, 512, inf))
          insert_string(Ptt_prints(buf,NO_RELOAD));
      }
      else
      {
        if (curredit & EDIT_LIST)       /* 去掉 mail list 之 header */
        {
          while (fgets(buf, 512, inf) && (!strncmp(buf, "※ ", 3)));
        }

        while (fgets(buf, 512, inf))
        {
          if (!strcmp(buf, "--\n"))
            break;
          if (!garbage_line(buf))
          {
            insert_char(':');
            insert_char(' ');
            insert_string(Ptt_prints(buf,ONLY_COLOR));
          }
        }
      }
      indent_mode = indent_mode0;
      fclose(inf);
    }
  }
}

/* ----------------------------------------------------- */
/* 審查 user 發表文章字數                                */
/* ----------------------------------------------------- */

static long
check_words()
{
  register textline *p = firstline;
  register char *str;
  int  i;
  long words = 0;

  while (p)
  {
//  by shakalaca 990816
//  在 FreeBSD 沒問題的 , 搬去 linux 會掛 , 所以搬出來
//  if (beginsig(str = p->data))
    str = p->data;
    if(!str) break;
    if (beginsig(str))
      break;

    if (!(!strncmp(str, "※ ", 3) || (str[1] == ' ' && 
       ((str[0] == ':') || (str[0] == '>')))))
       for(i = 0; i <= strlen(str); i++)
         if(str[i] != ' ' && str[i] != '\0')
           words ++;
    if (words > 10000000)
    {
      words = 10000000;
      return words;
    }
    p = p->next;
  }
  return words;
}


/* ----------------------------------------------------- */
/* 審查 user 引言的使用                                  */
/* ----------------------------------------------------- */


static int
check_quote()
{
  register textline *p = firstline;
  register char *str;
  int post_line;
  int included_line;

  post_line = included_line = 0;
  while (p)
  {
    if (!strcmp(str = p->data, "--"))
      break;
    if (str[1] == ' ' && ((str[0] == ':') || (str[0] == '>')))
    {
      included_line++;
    }
    else
    {
      while (*str == ' ' || *str == '\t')
        str++;
      if (*str)
        post_line++;
    }
    p = p->next;
  }

  if ((included_line >> 2) > post_line)
  {
    move(4, 0);

    outs("\
        本篇文章的引言比例超過 80%，請您做些微的修正：\n\n\
         \x1b[1;33m1) 增加一些文章 或  2) 刪除不必要之引言\x1b[0m");

    if (1/*HAS_PERM(PERM_SYSOP)*/) /* Kaede */
    {
      char ans[4];
      getdata(12, 12, "(E)繼續編輯 (W)強制寫入？[E] ", ans, 4, LCECHO,0);
      if (ans[0] == 'w')
        return 0;
    }
    else
        pressanykey(NULL);
    return 1;
  }
  return 0;
}


/* ----------------------------------------------------- */
/* 檔案處理：讀檔、存檔、標題、簽名檔                    */
/* ----------------------------------------------------- */


static void
read_file(fpath)
  char *fpath;
{
  FILE *fp;

  if ((fp = fopen(fpath, "r")) == NULL)
  {
    if (fp = fopen(fpath, "w+"))
    {
      fclose(fp);
      return;
    }
    indigestion(4);
    abort_bbs();
  }
  load_file(fp);
}


void
write_header(FILE *fp)
{
  time_t now = time(0);

  if (curredit & EDIT_MAIL || curredit & EDIT_LIST)
  {
    fprintf(fp, "%s %s (%s)\n", str_author1, cuser.userid,

#if defined(REALINFO) && defined(MAIL_REALNAMES)
      cuser.realname);
#else
      cuser.username);
#endif
  }
  else
  {
    char *ptr;

    struct
    {
      char author[IDLEN + 1];
      char board[IDLEN + 1];
      char title[66];
      time_t date;              /* last post's date */
      int number;               /* post number */
    }postlog;

    strcpy(postlog.author, cuser.userid);
#ifdef HAVE_ANONYMOUS
    if (currbrdattr& BRD_ANONYMOUS)
     {
      getdata(3, 0, "請輸入你想用的ID，也可直接按[Enter]，或是按[r]用真名：",real_name, 12, DOECHO,0);
      if(!real_name[0])
       {
        strcpy(real_name,"塵埃絮語");
        strcpy(postlog.author,real_name);
       }
      else
        {
        if (!strcmp("r",real_name))
           sprintf(postlog.author,"%s",cuser.userid);
        else
           sprintf(postlog.author,"%s.",real_name);
        }
     }
#endif
    strcpy(postlog.board, currboard);
    ptr = save_title;
    if (!strncmp(ptr, str_reply, 4))
      ptr += 4;
    strncpy(postlog.title, ptr, 65);
    postlog.date = now;
    postlog.number = 1;
    rec_add(".post", &postlog, sizeof(postlog));

#ifdef HAVE_ANONYMOUS
    if (currbrdattr & BRD_ANONYMOUS)
      fprintf(fp, "%s %s (%s) %s %s\n", str_author1, postlog.author ,
        (strcmp(real_name,"r")?"何須問我名":cuser.username), local_article ? str_post2 : str_post1, currboard);
    else
    {
      fprintf(fp, "%s %s (%s) %s %s\n", str_author1, cuser.userid,

#if defined(REALINFO) && defined(POSTS_REALNAMES)
        cuser.realname
#else
        cuser.username
#endif

        ,local_article ? str_post2 : str_post1, currboard);
    }
#else   /*HAVE_ANONYMOUS else*/

    fprintf(fp, "%s %s (%s) %s %s\n", str_author1, cuser.userid,

#if defined(REALINFO) && defined(POSTS_REALNAMES)
      cuser.realname
#else
      cuser.username
#endif

      ,local_article ? str_post2 : str_post1, currboard);
#endif  /*HAVE_ANONYMOUS endif*/
  }
  save_title[72] = '\0';
  fprintf(fp, "標題: %s\n時間: %s\n", save_title, ctime(&now));
}


int
showsignature(fname)
  char *fname;
{
  FILE *fp;
  char buf[512];
  int i, j;
  char ch;

  clear();
  move(2, 0);
  setuserfile(fname, "sig.0");
  j = strlen(fname) - 1;

  for (ch = '1'; ch <= '9'; ch++)
  {
    fname[j] = ch;
    if (fp = fopen(fname, "r"))
    {
      prints("\x1b[36m【 簽名檔.%c 】\x1b[m\n", ch);
      for (i = 0; i++ < MAXSIGLINES && fgets(buf, 512, fp); outs(buf));
      fclose(fp);
    }
  }
  return j;
}


void
addsignature(fp)
  FILE *fp;
{
  FILE *fs;
  int i;
  char buf[WRAPMARGIN + 1];
  char fpath[STRLEN];
  userec xuser;

  static char msg[] = "請選擇簽名檔 (1-9, 0=不加)[0]: ";
  char ch;
  
  extern bad_user(char* name);

  if (!strcmp(cuser.userid, "guest") || bad_user(cuser.userid)) {
     fprintf(fp, "\n--\n\x1b[1;36m※Post by \x1b[37m%-13s \x1b[36mfrom \x1b[33m%-24.24s\x1b[m",cuser.userid, getenv("RFC931"));
     return;
  }

  i = showsignature(fpath);

/* shakalaca.990425 */
/* original */
/*  msg[27] = ch = '0' + (cuser.uflag & SIG_FLAG); */
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  msg[27] = ch = '0' + (xuser.pager >> 4);
//  msg[27] = ch = currsig = cuser.signum ? cuser.signum : '0';
  
  getdata(0, 0, msg, buf, 4, DOECHO,0);

  if (ch != buf[0] && buf[0] >= '0' && buf[0] <= '9')
  {
    ch = buf[0];
//    currsig = buf[0];
//    cuser.signum = buf[0];
    xuser.pager = ((buf[0] - '0') << 4) + (xuser.pager & 0x0F);
    substitute_record(fn_passwd, &xuser, sizeof(userec), usernum);
/* shakalaca.990426: original */
/* cuser.uflag = (cuser.uflag & ~SIG_FLAG) | (ch & SIG_FLAG); */
  }

  if (ch != '0') { 
    fpath[i] = ch;
    if (fs = fopen(fpath, "r"))
    {
      fputs("\n--\n", fp);
      for (i = 0; i < MAXSIGLINES && fgets(buf, sizeof(buf), fs); i++)
        fputs(buf, fp);
      fclose(fs);
    }
  } 
#ifdef  HAVE_ORIGIN
#ifdef  HAVE_ANONYMOUS
  if (currbrdattr & BRD_ANONYMOUS && strcmp(real_name,"r"))
    fprintf(fp, "\n--\n\x1b[1;36m 猜猜我是誰！\n");
  else {
    char temp[25];
    strncpy (temp,fromhost,24);
    temp[25]='\0';
    fprintf(fp, "\n--\n\x1b[1;36m※Post by \x1b[37m%-13s \x1b[36mfrom \x1b[33m%-24.24s\x1b[m\n",cuser.userid,temp);
  }
#else
  strncpy (temp,fromhost,24);
    fprintf(fp, "\n--\n\x1b[1;36m※Post by \x1b[37m%-13s \x1b[36mfrom \x1b[33m%-24.24s\x1b[m\n",cuser.userid,temp);
#endif
#endif
}

long wordsnum;

static int
write_file(fpath, saveheader)
  char *fpath;
  int saveheader;
{
  FILE *fp;
  textline *p, *v;
  char ans[TTLEN], *msg;
  int aborted = 0;

  stand_title("檔案處理");

  if (currstat == SMAIL)
    msg = "[S]儲存 (A)放棄 (T)改標題 (E)繼續 (R/W/D)讀寫刪暫存檔？";
  else
    msg = "[S]儲存 (L)站內信件 (A)放棄 (T)改標題 (E)繼續 (R/W/D)讀寫刪暫存檔？";
  getdata(1, 0, msg, ans, 3, LCECHO,0);

  switch (ans[0])
  {
    case 'a':
    case 'A':
      outs("文章\x1b[1m 沒有 \x1b[0m存入");
      sleep(1);
      aborted = -1;
      break;

    case 'r':
    case 'R':
      read_tmpbuf(-1);

    case 'e':
    case 'E':
      return KEEP_EDITING;

    case 'w':
    case 'W':
      write_tmpbuf();
      return KEEP_EDITING;

    case 'd':
    case 'D':
      erase_tmpbuf();
      return KEEP_EDITING;

    case 't':
    case 'T':
      move(3, 0);
      prints("舊標題：%s", save_title);
      if (getdata(4, 0, "新標題：", ans, TTLEN, DOECHO,save_title))
        strcpy(save_title, ans);
      return KEEP_EDITING;

// wildcat patch : 原本沒有 break  :( 
    case 'l':
    case 'L':
      local_article = 1;
      break; 

// wildcat patch 000102 : default 就是 's'
    default :
      if (!HAS_PERM(PERM_LOGINOK))
      {
        local_article = 1;
        pressanykey("您尚未通過身份確認，只能 Local Save。");
      }
      else
        local_article = 0;
      break;
  }

  if (saveheader == 2 && !aborted) {
     stand_title("寫入失敗: 改寫到暫存檔");
     write_tmpbuf();
     return KEEP_EDITING;
  }

  if (!aborted)
  {

    if (saveheader && !(curredit & EDIT_MAIL) && check_quote())
      return KEEP_EDITING;

    if (!*fpath)
    {
      sethomepath(fpath, cuser.userid);
      strcpy(fpath, tempnam(fpath, "ve_"));
    }

    if ((fp = fopen(fpath, "w")) == NULL)
    {
      indigestion(5);
      abort_bbs();
    }

    if (saveheader)
      write_header(fp);
  }

  wordsnum = check_words();

  for (p = firstline; p; p = v)
  {
    v = p->next;
    if (!aborted)
    {
      msg = p->data;
      if (v || msg[0])
      {
        str_trim(msg);
        fprintf(fp, "%s\n", msg);
      }
    }
/* shakalaca patch            */
/* undef it for Linux 990816  */
#if !defined ( __linux__ ) || !defined ( __CYGWIN__ )
    free(p); 
#endif
  }
  currline = NULL;

  if (!aborted)
  {
    if (currstat == POSTING || currstat == SMAIL)
      addsignature(fp); 
    fclose(fp);

    if (local_article && (currstat == POSTING))
      return 1;

    return 0;
  }

  return aborted;
}




edit_outs(text)
  char *text;
{
  register int column = 0;
  register char ch;

  while ((ch = *text++) && (++column < SCR_WIDTH))
  {
    outch(ch == 27 ? '*' : ch);
  }
}


static void
display_buffer()
{
  register textline *p;
  register int i;
  int inblock;
  char buf[WRAPMARGIN + 2];
  int min, max;

  if (currpnt > blockpnt) {
     min = blockpnt;
     max = currpnt;
  }
  else {
     min = currpnt;
     max = blockpnt;
  }

  for (p = top_of_win, i = 0; i < b_lines; i++)
  {

    move(i, 0);
    clrtoeol();
/*
woju
*/
    if (blockln >= 0
        && (blockln <= currln
              && blockln <= (currln - curr_window_line + i)
              &&            (currln - curr_window_line + i) <= currln
           ||
                 currln  <= (currln - curr_window_line + i)
              &&            (currln - curr_window_line + i) <= blockln)) {
       outs("\x1b[7m");
       inblock = 1;
    }
    else
       inblock = 0;
    if (p)
    {
      if (my_ansimode)
         if (currln == blockln && p == currline && max > min) {
             outs("\x1b[0m");
             strncpy(buf, p->data, min);
             buf[min] = 0;
             outs(buf);
             outs("\x1b[7m");
             strncpy(buf, p->data + min, max - min);
             buf[max - min] = 0;
             outs(buf);
             outs("\x1b[0m");
             outs(p->data + max);
         }
         else
            outs(p->data);
      else
         if (currln == blockln && p == currline && max > min) {
             outs("\x1b[0m");
             strncpy(buf, p->data, min);
             buf[min] = 0;
             edit_outs(buf);
             outs("\x1b[7m");
             strncpy(buf, p->data + min, max - min);
             buf[max - min] = 0;
             edit_outs(buf);
             outs("\x1b[0m");
             edit_outs(p->data + max);
         }
         else
            edit_outs(&p->data[edit_margin]);
      p = p->next;
      if (inblock)
         outs("\x1b[0m");
    }
    else
      outch('~');
  }
  edit_msg();
}



/*
woju
lino == 0 for prompt
*/

goto_line(int lino)
{
   char buf[10];

   if (lino > 0 || getdata(b_lines - 1, 0, "跳至第幾行:", buf, 10, DOECHO,0)
       && sscanf(buf, "%d", &lino) && lino > 0) {
      textline* p;

      prevln = currln;
      prevpnt = currpnt;
      p = firstline;
      currln = lino - 1;

      while (--lino && p->next)
         p = p->next;

      if (p)
         currline = p;
      else {
         currln = totaln;
         currline = lastline;
      }
      currpnt = 0;
      if (currln < 11) {
         top_of_win = firstline;
         curr_window_line = currln;
      }
      else {
         int i;
         curr_window_line = 11;

         for (i = curr_window_line; i; i--)
            p = p->prev;
            top_of_win = p;
      }
   }
   redraw_everything = YEA;
}



char* strcasestr(const char* big, const char* little)
{
   char* ans = (char*)big;
   int len = strlen(little);
   char* endptr = (char*)big + strlen(big) - len;

   while (ans <= endptr)
      if (!strncasecmp(ans, little, len))
         return ans;
      else
         ans++;
   return 0;
}


/*
woju
mode:
    0: prompt
    1: forward
    -1: backward
*/
search_str(int mode)
{
   static char str[80];
   typedef char* (*FPTR)();
   static FPTR fptr;
   char ans[4] = "n";

   if (!mode) {
      if (getdata(b_lines - 1, 0,"[搜尋]關鍵字:",str, 65, DOECHO,0))
      if (*str)
         if (getdata(b_lines - 1, 0, "區分大小寫(Y/N/Q)? [N] ", ans, 4, LCECHO,0)
             && *ans == 'y')
            fptr = strstr;
         else
            fptr = strcasestr;
   }

   if (*str && *ans != 'q') {
      textline* p;
      char* pos;
      int lino;

      if (mode >= 0) {
         for (lino = currln, p = currline; p; p = p->next, lino++)
            if ((pos = fptr(p->data + (lino == currln ? currpnt + 1 : 0), str))
                && (lino != currln || pos - p->data != currpnt))
               break;
      }
      else {
         for (lino = currln, p = currline; p; p = p->prev, lino--)
            if ((pos = fptr(p->data, str))
                && (lino != currln || pos - p->data != currpnt))
               break;
      }
      if (pos) {
         prevln = currln;
         prevpnt = currpnt;
         currline = p;
         currln = lino;
         currpnt = pos - p->data;
         if (lino < 11) {
            top_of_win = firstline;
            curr_window_line = currln;
         }
         else {
            int i;
            curr_window_line = 11;

            for (i = curr_window_line; i; i--)
               p = p->prev;
            top_of_win = p;
         }
         redraw_everything = YEA;
      }
   }
   if (!mode)
      redraw_everything = YEA;
}



match_paren()
{
   static char parens[] = "()[]{}";
   int type;
   int parenum = 0;
   char *ptype;
   textline* p;
   int lino;
   int c, i;

   if (!(ptype = strchr(parens, currline->data[currpnt])))
      return 0;

   type = (ptype - parens) / 2;
   parenum += ((ptype - parens) % 2) ? -1 : 1;


   if (parenum > 0) {
     for (lino = currln, p = currline; p; p = p->next, lino++) {
         lino = lino;
         for (i = (lino == currln) ? currpnt + 1 : 0; i < strlen(p->data); i++)
            if (p->data[i] == '/' && p->data[++i] == '*') {
               ++i;
               while (1) {
                  while(i < strlen(p->data) - 1
                      && !(p->data[i] == '*' && p->data[i + 1] == '/'))
                     i++;
                  if (i >= strlen(p->data) - 1 && p->next) {
                     p = p->next;
                     ++lino;
                     i = 0;
                  }
                  else
                     break;
               }
            }
            else if ((c = p->data[i]) == '\'' || c == '"') {
               while (1) {
                  while (i < (int)(strlen(p->data) - 1))
                     if (p->data[++i] == '\\' && i < strlen(p->data) - 2)
                        ++i;
                     else if (p->data[i] == c)
                        goto end_quote;
                  if (i >= strlen(p->data) - 1 && p->next) {
                     p = p->next;
                     ++lino;
                     i = -1;
                  }
                  else
                     break;
               }
end_quote:
               ;
            }
            else if ((ptype = strchr(parens, p->data[i]))
                     && (ptype - parens) / 2 == type)
               if (!(parenum += ((ptype - parens) % 2) ? -1 : 1))
                  goto p_outscan;
     }
   }
   else {
      for (lino = currln, p = currline; p; p = p->prev, lino--)
         for (i = (lino == currln) ? currpnt - 1 : strlen(p->data) - 1;
              i >= 0; i--)
            if (p->data[i] == '/' && p->data[--i] == '*' && i > 0) {
               --i;
               while (1) {
                  while(i > 0
                      && !(p->data[i] == '*' && p->data[i - 1] == '/'))
                     i--;
                  if (i <= 0 && p->prev) {
                     p = p->prev;
                     --lino;
                     i = strlen(p->data) - 1;
                  }
                  else
                     break;
               }
            }
            else if ((c = p->data[i]) == '\'' || c == '"') {
               while (1) {
                  while (i > 0)
                     if (i > 1 && p->data[i - 2] == '\\')
                        i -= 2;
                     else if ((p->data[--i]) == c)
                        goto begin_quote;
                  if (i <= 0 && p->prev) {
                     p = p->prev;
                     --lino;
                     i = strlen(p->data);
                  }
                  else
                     break;
               }
begin_quote:
               ;
            }
            else if ((ptype = strchr(parens, p->data[i]))
                     && (ptype - parens) / 2 == type)
               if (!(parenum += ((ptype - parens) % 2) ? -1 : 1))
                  goto p_outscan;
   }

p_outscan:

   if (!parenum) {
      int top = currln - curr_window_line;
      int bottom = currln - curr_window_line + b_lines - 1;

      currpnt = i;
      currline = p;
      curr_window_line += lino - currln;
      currln = lino;

      if (lino < top || lino > bottom) {
         if (lino < 11) {
            top_of_win = firstline;
            curr_window_line = currln;
         }
         else {
            int i;
            curr_window_line = 11;

            for (i = curr_window_line; i; i--)
               p = p->prev;
            top_of_win = p;
         }
         redraw_everything = YEA;
      }
   }
}


block_del(int hide)
{
   if (blockln < 0) {
      blockln = currln;
      blockpnt = currpnt;
      blockline = currline;
   }
   else {
      char fp_tmpbuf[80];
      FILE* fp;
      textline *begin, *end, *p;
      char tmpfname[10] = "buf.0";
      char ans[6] = "w+n";

      move(b_lines - 1, 0);
      clrtoeol();
      if (hide == 1)
         tmpfname[4] = 'q';
      else if (!hide && !getdata(b_lines - 1, 0, "把區塊移至暫存檔 (0:Cut, 5:Copy, 6-9, q: Cancel)[0] ",  tmpfname + 4, 4, LCECHO,0))
         tmpfname[4] = '0';
      if (tmpfname[4] < '0' || tmpfname[4] > '9')
         tmpfname[4] = 'q';
      if ('1' <= tmpfname[4] && tmpfname[4] <= '9') {
         setuserfile(fp_tmpbuf, tmpfname);
         if (tmpfname[4] != '5' && dashf(fp_tmpbuf)) {
            more(fp_tmpbuf, NA);
            getdata(b_lines - 1, 0, "暫存檔已有資料 (A)附加 (W)覆寫 (Q)取消？[W] ", ans, 4, LCECHO,"W");
            if (*ans == 'q')
               tmpfname[4] = 'q';
            else if (*ans != 'a')
               *ans = 'w';
          }
          if (tmpfname[4] != '5') {
             getdata(b_lines - 1, 0, "刪除區塊(Y/N)?[N] ", ans + 2, 4, LCECHO,0);
             if (ans[2] != 'y')
                ans[2] = 'n';
          }
      }
      else if (hide != 3)
         ans[2] = 'y';

      tmpfname[5] = ans[1] = ans[3] = 0;
      if (tmpfname[4] != 'q') {
         if (currln >= blockln) {
            begin = blockline;
            end = currline;
            if (ans[2] == 'y' && !(begin == end && currpnt != blockpnt)) {
               curr_window_line -= (currln - blockln);
               if (curr_window_line < 0) {
                  curr_window_line = 0;
               if (end->next)
                   (top_of_win = end->next)->prev = begin->prev;
                else
                   top_of_win = (lastline = begin->prev);
                }
                 currln -= (currln - blockln);
            }
         }
         else {
            begin = currline;
            end = blockline;
         }


         if (ans[2] == 'y' && !(begin == end && currpnt != blockpnt)) {
            if (begin->prev)
               begin->prev->next = end->next;
            else if (end->next)
               top_of_win = firstline = end->next;
            else {
               currline = top_of_win = firstline = lastline = alloc_line();
               currln = curr_window_line = edit_margin = 0;
            }

            if (end->next)
               (currline = end->next)->prev = begin->prev;
            else if (begin->prev) {
               currline = (lastline = begin->prev);
               currln--;
               if (curr_window_line > 0)
                  curr_window_line--;
            }
         }

         setuserfile(fp_tmpbuf, tmpfname);
         if (fp = fopen(fp_tmpbuf, ans)) {
            if (begin == end && currpnt != blockpnt) {
               char buf[WRAPMARGIN + 2];

               if (currpnt > blockpnt) {
                  strcpy(buf, begin->data + blockpnt);
                  buf[currpnt - blockpnt] = 0;
               }
               else {
                  strcpy(buf, begin->data + currpnt);
                  buf[blockpnt - currpnt] = 0;
               }
               fputs(buf, fp);
            }
            else {
               for (p = begin; p != end; p = p->next)
                  fprintf(fp, "%s\n", p->data);
               fprintf(fp, "%s\n", end->data);
            }
            fclose(fp);
         }

         if (ans[2] == 'y') {
            if (begin == end && currpnt != blockpnt) {
               int min, max;

               if (currpnt > blockpnt) {
                  min = blockpnt;
                  max = currpnt;
               }
               else {
                  min = currpnt;
                  max = blockpnt;
               }
               strcpy(begin->data + min, begin->data + max);
               begin->len -= max - min;
               currpnt = min;
            }
            else {
               for (p = begin; p != end; totaln--)
                  free((p = p->next)->prev);
               free(end);
               totaln--;
               currpnt = 0;
            }
         }
      }
      blockln = -1;
      redraw_everything = YEA;
   }
}

block_shift_left()
{
   textline *begin, *end, *p;

   if (currln >= blockln) {
      begin = blockline;
      end = currline;
   }
   else {
      begin = currline;
      end = blockline;
   }
   p = begin;
   while (1) {
      if (p->len) {
         strcpy(p->data, p->data + 1);
         --p->len;
      }
      if (p == end)
         break;
      else
         p = p->next;
   }
   if (currpnt > currline->len)
      currpnt = currline->len;
   redraw_everything = YEA;
}

block_shift_right()
{
   textline *begin, *end, *p;

   if (currln >= blockln) {
      begin = blockline;
      end = currline;
   }
   else {
      begin = currline;
      end = blockline;
   }
   p = begin;
   while (1) {
      if (p->len < WRAPMARGIN) {
         int i = p->len + 1;

         while (i--)
            p->data[i + 1] = p->data[i];
         p->data[0] = insert_character ? ' ' : insert_c;
         ++p->len;
      }
      if (p == end)
         break;
      else
         p = p->next;
   }
   if (currpnt > currline->len)
      currpnt = currline->len;
   redraw_everything = YEA;
}

transform_to_color(char* line)
{
   while (line[0] && line[1])
      if (line[0] == '*' && line[1] == '[') {
         line[0] = KEY_ESC;
         line += 2;
      }
      else
         ++line;
}


block_color()
{
   textline *begin, *end, *p;

   if (currln >= blockln) {
      begin = blockline;
      end = currline;
   }
   else {
      begin = currline;
      end = blockline;
   }
   p = begin;
   while (1) {
      transform_to_color(p->data);
      if (p == end)
         break;
      else
         p = p->next;
   }
   block_del(1);
}


/* ----------------------------------------------------- */
/* 編輯處理：主程式、鍵盤處理                            */
/* ----------------------------------------------------- */


extern int my_write();

int
vedit(fpath, saveheader)
  char *fpath;
  int saveheader;
{
  FILE *fp1;  /* Ptt */
  char last;
  int ch, foo;
  int lastindent = -1;
  int last_margin;
  int mode0 = currutmp->mode;
  int destuid0 = currutmp->destuid;
  unsigned short int interval=0;
  unsigned short int count=0;
  time_t now=0,th;

  textline* firstline0 = firstline;
  textline* lastline0 = lastline;
  textline* currline0 = currline;
  textline* blockline0 = blockline;
  textline* top_of_win0 = top_of_win;
  int local_article0 = local_article;
  int currpnt0 = currpnt;
  int currln0 = currln;
  int totaln0 = totaln;
  int curr_window_line0 = curr_window_line;
  int insert_character0 = insert_character;
  int my_ansimode0 = my_ansimode;
  int edit_margin0 = edit_margin;
  int blockln0 = blockln;
  char ans1,ans2,ans[2];
  int star_return;
  currutmp->mode = EDITING;
  currutmp->destuid = currstat;
  insert_character = redraw_everything = 1;
  prevln = blockln = -1;

  line_dirty = currpnt = totaln = my_ansimode = 0;
  currline = top_of_win = firstline = lastline = alloc_line();

  if (*fpath)
  {
    read_file(fpath);
  }

  if (*quote_file)
  {
    do_quote();
    *quote_file = '\0';
    if (quote_file[79] == 'L')
      local_article = 1;
  }

  currline = firstline;
  currpnt = currln = curr_window_line = edit_margin = last_margin = 0;

  while (1)
  {
    if(phone_mode)
    {
      clrchyiuan(21,23);
      move(b_lines-2,0);
      prints("%s\n",(star_special_mode>0)?star_key[1]:star_key[0]);
      prints("%s",star_msg[star_special_mode]);
    }
    if (redraw_everything || blockln >=0)
    {
      display_buffer();
      redraw_everything = NA;
      if(phone_mode)
      {
        clrchyiuan(21,23);
        move(b_lines-2,0);
        prints("%s\n",(star_special_mode>0)?star_key[1]:star_key[0]);
        prints("%s",star_msg[star_special_mode]);
      }
    }
    if (my_ansimode)
       ch = n2ansi(currpnt, currline);
    else
       ch = currpnt - edit_margin;
    move(curr_window_line, ch);
    if (!line_dirty && strcmp(line, currline->data))
       strcpy(line, currline->data);
    ch = igetkey();

    if(interval=(unsigned short int)((th=currutmp->lastact) - now))
        {
         if (interval==1)
           count++;
         else
           count=0;
         now = th;
         if ((char)ch!=last)
            last=(char)ch;
        }
/* Jaky */
    if (count >= 200)
       abort_bbs();              /* 連續兩百秒不鬆手,分明是在斂財*/

    if (raw_mode)
       switch (ch) {
       case Ctrl('S'):
       case Ctrl('Q'):
       case Ctrl('T'):
          continue;
          break;
       }

    if (ch == Ctrl('J') && !raw_mode)
       goto_line(0);
    if(phone_mode && (star_return = star_ime(ch,&ans1,&ans2))
       || ch < 0x100 && isprint2(ch))
    {
      if (phone_mode && star_return==1)
      {
        sprintf(ans,"%c%c",ans1,ans2);
        insert_string(ans);
      }
      else if(phone_mode && star_return==2)
        redraw_everything = YEA;
      else
        insert_char(ch);
      lastindent = -1;
      line_dirty = 1;
    }
    else
    {
      if (ch == Ctrl('P') || ch == KEY_UP || ch == KEY_DOWN || ch == Ctrl('N'))
      {
        if (lastindent == -1)
          lastindent = currpnt;
      }
      else
        lastindent = -1;

/*
woju
*/
      if (ch == KEY_ESC)
         switch (KEY_ESC_arg) {
         case ',':
            ch = Ctrl(']');
            break;
         case '.':
            ch = Ctrl('T');
            break;
         case 'v':
            ch = KEY_PGUP;
            break;
         case 'a':
         case 'A':
            ch = Ctrl('V');
            break;
         case 'X':
            ch = Ctrl('X');
            break;
         case 'q':
            ch = Ctrl('Q');
            break;
         case 'o':
            ch = Ctrl('O');
            break;
         case '-':
            ch = Ctrl('_');
            break;
         case 's':
            ch = Ctrl('S');
            break;
         }

      switch (ch)
      {
      case Ctrl('X'):           /* Save and exit */
        foo = write_file(fpath, saveheader);
        if (foo != KEEP_EDITING)
        {
          currutmp->mode = mode0;
          currutmp->destuid = destuid0;
          firstline = firstline0;
          lastline = lastline0;
          currline = currline0;
          blockline = blockline0;
          top_of_win = top_of_win0;
          local_article = local_article0;
          currpnt = currpnt0;
          currln = currln0;
          totaln = totaln0;
          curr_window_line = curr_window_line0;
          insert_character = insert_character0;
          my_ansimode = my_ansimode0;
          edit_margin = edit_margin0;
          blockln = blockln0;
          return foo;
        }
        line_dirty = 1;
        redraw_everything = YEA;
        break;

      case Ctrl('W'):
         if (blockln >= 0)
            block_del(2);
         line_dirty = 1;
         break;

      case Ctrl('Q'):           /* Quit without saving */
        ch = ask("結束但不儲存 (Y/N)? [N]: ");
        if (ch == 'y' || ch == 'Y') {
          currutmp->mode = mode0;
          currutmp->destuid = destuid0;
          firstline = firstline0;
          lastline = lastline0;
          currline = currline0;
          blockline = blockline0;
          top_of_win = top_of_win0;
          local_article = local_article0;
          currpnt = currpnt0;
          currln = currln0;
          totaln = totaln0;
          curr_window_line = curr_window_line0;
          insert_character = insert_character0;
          my_ansimode = my_ansimode0;
          edit_margin = edit_margin0;
          blockln = blockln0;
          return -1;
        }
        line_dirty = 1;
        redraw_everything = YEA;
        break;

      case Ctrl('C'):
        ch = insert_character;
        insert_character = redraw_everything = YEA;

        if (!my_ansimode)
        {
          insert_string(reset_color);
        }
        else
        {
          char ans[4];
          move(b_lines - 2, 55);
          outs("\x1b[1;33;40mB\x1b[41mR\x1b[42mG\x1b[43mY\x1b[44mL\x1b[45mP\x1b[46mC\x1b[47mW\x1b[m");
          if (getdata(b_lines - 1, 0, "請輸入  亮度/前景/背景[正常白字黑底][0wb]：", ans, 4, LCECHO,0))
          {
            char t[] = "BRGYLPCW";
            char color[15];
            char *tmp, *apos = ans;
            int fg, bg;
            strcpy(color, "\x1b[");
            if (isdigit(*apos))
            {
              sprintf(color, "%s%c", color, *(apos++));
              if (*apos)
                sprintf(color, "%s;", color);
            }
            if (*apos)
            {
              if (tmp = strchr(t, toupper(*(apos++))))
                fg = tmp - t + 30;
              else
                fg = 37;
              sprintf(color, "%s%d", color, fg);
            }
            if (*apos)
            {
              if (tmp = strchr(t, toupper(*(apos++))))
                bg = tmp - t + 40;
              else
                bg = 40;
              sprintf(color, "%s;%d", color, bg);
            }
            sprintf(color, "%sm", color);
            insert_string(color);
          }
          else
            insert_string(reset_color);
        }
        insert_character = ch;
        line_dirty = 1;
        break;

      case KEY_ESC:
         line_dirty = 0;
         switch (KEY_ESC_arg) {
         case 'U':
         {
            int mode0 = currutmp->mode;
            int stat0 = currstat;
            screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
            vs_save(screen);
            t_users();
            vs_restore(screen);
            redraw_everything = YEA;
            line_dirty = 1;
            currutmp->mode = mode0;
            currstat = stat0;
            break;
         }
         case 'i':
            t_idle();
            redraw_everything = YEA;
            line_dirty = 1;
            break;
         case 'n':
            search_str(1);
            break;
         case 'p':
            search_str(-1);
            break;
         case 'L':
         case 'J':
            goto_line(0);
            break;
         case ']':
            match_paren();
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
            read_tmpbuf(KEY_ESC_arg - '0');
            redraw_everything = YEA;
            break;
         case 'l':                       /* block delete */
         case ' ':
            block_del(0);
            line_dirty = 1;
            break;
         case 'u':
            if (blockln >= 0)
               block_del(1);
            line_dirty = 1;
            break;
         case 'c':
            if (blockln >= 0)
               block_del(3);
            line_dirty = 1;
            break;
         case 'y':
            undelete_line();
            break;
         case 'P':
            phone_mode ^= 1;
            line_dirty = 1;
            break;
         case 'R':
            raw_mode ^= 1;
            line_dirty = 1;
            break;
         case 'I':
            indent_mode ^= 1;
            line_dirty = 1;
            break;
         case 'j':
            if (blockln >= 0)
               block_shift_left();
            else if (currline->len) {
               int currpnt0 = currpnt;
               currpnt = 0;
               delete_char();
               currpnt = (currpnt0 <= currline->len) ? currpnt0 : currpnt0 - 1;
               if (my_ansimode)
                   currpnt = ansi2n(n2ansi(currpnt, currline),currline);
            }
            line_dirty = 1;
            break;
         case 'k':
            if (blockln >= 0)
               block_shift_right();
            else {
               int currpnt0 = currpnt;
               currpnt = 0;
               insert_char(' ');
               currpnt = currpnt0;
            }
            line_dirty = 1;
            break;
         case 'f':
            while (currpnt < currline->len && isalnum(currline->data[++currpnt]))
               ;
            while (currpnt < currline->len && isspace(currline->data[++currpnt]))
               ;
            line_dirty = 1;
            break;
         case 'b':
            while (currpnt && isalnum(currline->data[--currpnt]))
               ;
            while (currpnt && isspace(currline->data[--currpnt]))
               ;
            line_dirty = 1;
            break;
         case 'd':
            while (currpnt < currline->len) {
               delete_char();
               if (!isalnum(currline->data[currpnt]))
                  break;
            }
            while (currpnt < currline->len) {
               delete_char();
               if (!isspace(currline->data[currpnt]))
                  break;
            }
            line_dirty = 1;
            break;
         default:
            line_dirty = 1;
         }
         break;
      case Ctrl('_'):
         if (strcmp(line, currline->data)) {
            char buf[WRAPMARGIN];

            strcpy(buf, currline->data);
            strcpy(currline->data, line);
            strcpy(line, buf);
            currline->len = strlen(currline->data);
            currpnt = 0;
            line_dirty = 1;
         }
         break;
      case Ctrl('S'):
         search_str(0);
         break;
      case Ctrl('U'):
        insert_char('\x1b');
        line_dirty = 1;
        break;

      case Ctrl('V'):                   /* Toggle ANSI color */
         my_ansimode ^= 1;
         if (my_ansimode && blockln >= 0)
            block_color();
         clear();
         redraw_everything = YEA;
         line_dirty = 1;
         break;

      case Ctrl('I'):
        do
        {
          insert_char(' ');
        }
        while (currpnt & 0x3);
        line_dirty = 1;
        break;

      case '\r':
      case '\n':
        split(currline, currpnt);
        line_dirty = 0;
        break;

/* Ptt */
      case Ctrl('G'):  /* 啟動範本精靈 */
        {
          int mode0 = currutmp->mode;
          int currstat0 = currstat;
          //setutmpmode(EDITEXP);
          a_menu("編輯輔助器", "etc/editexp", (HAS_PERM(PERM_SYSOP) ? SYSOP : NOBODY),EDITEXP);
          currutmp->mode = mode0;
          currstat = currstat0;
        }
        if (trans_buffer[0])
        {
          if (fp1 = fopen(trans_buffer, "r"))
          {
            int indent_mode0 = indent_mode;
            indent_mode = 0;
            prevln = currln;
            prevpnt = currpnt;

            while (fgets(line, WRAPMARGIN + 2, fp1))
            {
              if (!strncmp(line,"作者:",5) ||
                  !strncmp(line,"標題:",5) ||
                  !strncmp(line,"時間:",5)) 
                continue;
              insert_string(line);
            }
            fclose(fp1);
            indent_mode = indent_mode0;

            while (curr_window_line >= b_lines)
            {
              curr_window_line--;
              top_of_win = top_of_win->next;
            }
          }
        }
        redraw_everything = YEA;
        line_dirty = 1;
        break;

      case KEY_LEFT:
        if (currpnt) {
          if (my_ansimode)
             currpnt = n2ansi(currpnt, currline);
          currpnt--;
          if (my_ansimode)
             currpnt = ansi2n(currpnt, currline);
          line_dirty = 1;
        }
        else if (currline->prev)
        {
          curr_window_line--;
          currln--;
          currline = currline->prev;
          currpnt = currline->len;
          line_dirty = 0;
        }
        break;

      case KEY_RIGHT:
        if (currline->len != currpnt) {
          if (my_ansimode)
             currpnt = n2ansi(currpnt, currline);
          currpnt++;
          if (my_ansimode)
             currpnt = ansi2n(currpnt, currline);
          line_dirty = 1;
        }
        else if (currline->next)
        {
          currpnt = 0;
          curr_window_line++;
          currln++;
          currline = currline->next;
          line_dirty = 0;
        }
        break;

      case KEY_UP:
      case Ctrl('P'):
        if (currline->prev)
        {
          if (my_ansimode)
             ch = n2ansi(currpnt,currline);
          curr_window_line--;
          currln--;
          currline = currline->prev;
          if (my_ansimode)
             currpnt = ansi2n(ch , currline);
          else
             currpnt = (currline->len > lastindent) ? lastindent : currline->len;
          line_dirty = 0;
        }
        break;

      case KEY_DOWN:
      case Ctrl('N'):
        if (currline->next)
        {
          if (my_ansimode)
             ch = n2ansi(currpnt,currline);
          currline = currline->next;
          curr_window_line++;
          currln++;
          if (my_ansimode)
             currpnt = ansi2n(ch , currline);
          else
             currpnt = (currline->len > lastindent) ? lastindent : currline->len;
          line_dirty = 0;
        }
        break;
      case Ctrl('B'):
      case KEY_PGUP:
        redraw_everything = currln;
        top_of_win = back_line(top_of_win, 22);
        currln = redraw_everything;
        currline = back_line(currline, 22);
        curr_window_line = getlineno();
        if (currpnt > currline->len)
          currpnt = currline->len;
        redraw_everything = YEA;
        line_dirty = 0;
        break;

      case KEY_PGDN:
      case Ctrl('F'):
        redraw_everything = currln;
        top_of_win = forward_line(top_of_win, 22);
        currln = redraw_everything;
        currline = forward_line(currline, 22);
        curr_window_line = getlineno();
        if (currpnt > currline->len)
          currpnt = currline->len;
        redraw_everything = YEA;
        line_dirty = 0;
        break;

      case KEY_END:
      case Ctrl('E'):
        currpnt = currline->len;
        line_dirty = 1;
        break;

      case Ctrl(']'):   /* start of file */
        prevln = currln;
        prevpnt = currpnt;
        currline = top_of_win = firstline;
        currpnt = currln = curr_window_line = 0;
        redraw_everything = YEA;
        line_dirty = 0;
        break;

      case Ctrl('T'):           /* tail of file */
        prevln = currln;
        prevpnt = currpnt;
        top_of_win = back_line(lastline, 23);
        currline = lastline;
        curr_window_line = getlineno();
        currln = totaln;
        redraw_everything = YEA;
        currpnt = 0;
        line_dirty = 0;
        break;

      case KEY_HOME:
      case Ctrl('A'):
        currpnt = 0;
        line_dirty = 1;
        break;

      case KEY_INS:             /* Toggle insert/overwrite */
      case Ctrl('O'):
        if (blockln >= 0 && insert_character) {
           char ans[4];

           getdata(b_lines - 1, 0, "區塊微調右移插入字元(預設為空白字元)", ans, 4,0);
           insert_c = (*ans) ? *ans : ' ';
        }
        insert_character ^= 1;
        line_dirty = 1;
        break;

      case Ctrl('H'):
      case '\177':              /* backspace */
        line_dirty = 1;
        if (my_ansimode) {
           my_ansimode = 0;
           clear();
           redraw_everything = YEA;
        }
        else {
           if (currpnt == 0)
           {
             textline *p;

             if (!currline->prev)
             {
               break;
             }
             line_dirty = 0;
             curr_window_line--;
             currln--;
             currline = currline->prev;
             currpnt = currline->len;
             redraw_everything = YEA;
             if (*killsp(currline->next->data) == '\0')
             {
               delete_line(currline->next);
               break;
             }
             p = currline;
             while (!join(p))
             {
               p = p->next;
               if (p == NULL)
               {
                 indigestion(2);
                 abort_bbs();
               }
             }
             break;
           }
           currpnt--;
           delete_char();
        }
        break;

      case Ctrl('D'):
      case KEY_DEL:             /* delete current character */
         line_dirty = 1;
        if (currline->len == currpnt)
        {
          textline *p = currline;

          while (!join(p))
          {
            p = p->next;
            if (p == NULL)
            {
              indigestion(2);
              abort_bbs();
            }
          }
          line_dirty = 0;
          redraw_everything = YEA;
        }
        else {
           delete_char();
           if (my_ansimode)
              currpnt = ansi2n(n2ansi(currpnt, currline),currline);
        }
        break;

      case Ctrl('Y'):           /* delete current line */
        currline->len = currpnt = 0;

      case Ctrl('K'):           /* delete to end of line */
        if (currline->len == 0)
        {
          textline *p = currline->next;

          if (!p)
          {
            p = currline->prev;
            if (!p)
            {
              break;
            }
            if (curr_window_line > 0)
            {
              curr_window_line--;
              currln--;
            }
          }
          if (currline == top_of_win)
            top_of_win = p;
          delete_line(currline);
          currline = p;
          redraw_everything = YEA;
          line_dirty = 0;
          break;
        }

        if (currline->len == currpnt)
        {
          textline *p = currline;

          while (!join(p))
          {
            p = p->next;
            if (p == NULL)
            {
              indigestion(2);
              abort_bbs();
            }
          }
          redraw_everything = YEA;
          line_dirty = 0;
          break;
        }
        currline->len = currpnt;
        currline->data[currpnt] = '\0';
        line_dirty = 1;
        break;
      }

      if (currln < 0)
        currln = 0;
      if (curr_window_line < 0)
      {
        curr_window_line = 0;
        if (!top_of_win->prev)
        {
          indigestion(6);
        }
        else
        {
          top_of_win = top_of_win->prev;
          rscroll();
        }
      }
      if (curr_window_line == b_lines ||
         (phone_mode && curr_window_line == b_lines-3))
      {
        if(phone_mode)
        {
          curr_window_line = t_lines - 5;
          redraw_everything = YEA;
        }
        else
          curr_window_line = t_lines - 2;
        if (!top_of_win->next)
          indigestion(7);
        else
        {
          top_of_win = top_of_win->next;
          move(b_lines, 0);
          clrtoeol();
          scroll();
        }
      }
    }
    edit_margin = currpnt < SCR_WIDTH - 1 ? 0 : currpnt / 72 * 72;

    if (!redraw_everything)
    {
      if (edit_margin != last_margin)
      {
        last_margin = edit_margin;
        redraw_everything = YEA;
      }
      else
      {
         move(curr_window_line, 0);
         clrtoeol();
         if (my_ansimode)
           outs(currline->data);
         else
            edit_outs(&currline->data[edit_margin]);
         edit_msg();
      }
    }
  }
}
