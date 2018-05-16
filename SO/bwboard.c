/*-------------------------------------------------------*/
/* bwboard.c    ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : thor.bbs@bbs.cs.nthu.edu.tw                  */
/* target : Black White Chess Board dynamic link module  */
/* create : 99/02/20                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/

#include "bbs.h"

#define MAXLASTCMD 20

char lastcmd[MAXLASTCMD][80];

static int bwline;            /* Where to display message now */
static int cfd; /* socket number */
static int myColor; /* my chess color */
static int GameOver; /* end game */

/* Thor.990311: dynamic attribute key */
static int key_total; /* attribute key for total */
static int key_win; /* attribute key for win */

#define stop_line (b_lines - 3)

#ifdef EVERY_Z
extern int vio_fd, holdon_fd;
#endif
extern KeyFunc Talk[];
extern KeyFunc myTurn[],yourTurn[];

enum { Empty=0, Black=1, White=2, Deny=3};
static char *icon[] = {"┼","●","○","  " };

/* 19 * 19, standard chess board */
char Board[19][19];
char Allow[19][19];
int numWhite, numBlack;
int rulerRow, rulerCol;

static int yourPass, myPass;
static int yourStep, myStep;

enum {DISCONNECT = -2, LEAVE = -1, NOTHING = 0};

#if 0

  rules util include
      0. Board initialize
         (happen before start, and clear board action)
      1. Board update by "color" down a "chess pos" (without update screen)
                                                    (screen by a main redraw routine, after call this)
          return value, represent win side to end  (for both turn)
         (happen when state change)
      2. Board allow step checking by "color"    (for my turn)
          return value, represent # possible step
         (happen when state change)
         if Game is over, won''t allow any step

#endif

enum{Binit = 0,Bupdate,Ballow};

static int (**rule)();

/* numWhite, numBlack maintained by rule, 0&1 */
/* Board[19][19] Allow[19][19] maintained by rule */

static inline void countBWnum()
{
  int i,j;
  numWhite = numBlack = 0;
  for(i = 0; i<19;i++)
    for(j=0; j<19;j++)
      if(Board[i][j]==White)
        numWhite ++;
      else if(Board[i][j]==Black)
        numBlack ++;
}

/* Rules for othello, 8 x 8 */
static int othInit()
{
  int i,j;
  for(i=0;i<19;i++)
    for(j=0;j<19;j++)
      Board[i][j] = i<8 && j<8 ? Empty : Deny;
  Board[3][3] = Board[4][4] = Black;
  Board[3][4] = Board[4][3] = White;
  numWhite = numBlack = 2;
  rulerRow = rulerCol = 8;
  return 0;
}

static inline int othEatable(int Color,int row, int col, int rowstep, int colstep)
{
  int eat = 0;
  do{
    row += rowstep;
    col += colstep;
    /* check range */
    if(row < 0 || row >=8 || col <0 || col >=8) return 0;
    if(Board[row][col] == Color) return eat;
    eat = 1;
  } while(Board[row][col] == Deny - Color);
  return 0;
}

static int othUpdate(int Color, int row, int col)
{
  int i,j, p,q;
  int winside = Empty;
  Board[row][col] = Color;
  for(i = -1; i <= 1; i++)
    for(j = -1; j<= 1; j++)
      if(i !=0 || j!=0)
        if(othEatable(Color,row,col,i,j))
        {
          p = row + i;
          q = col + j;
          for(;;)
          {
            if(Board[p][q] == Color) break;
            Board[p][q] = Color;
            p+=i;
            q+=j;
          }
        }
  /* count numWhite & numBlack */
  countBWnum();

  /* Thor.990329: 註解, 下滿時 */
  /* if(numWhite + numBlack == 8*8) */
  {
    static int othAllow();
    int my = myColor; /* Thor.990331: 暫存myColor */
    int allowBlack, allowWhite;
    myColor = Black;
    allowBlack = othAllow();
    myColor = White;
    allowWhite = othAllow();
    myColor = my;
    if(allowBlack == 0 && allowWhite == 0)
    {
    
      if(numWhite > numBlack)
        winside = White;
      else if(numWhite < numBlack)
        winside = Black;
      else
        winside = Deny;
    }
  }
#if 0
  /* Thor.990329: 沒子下時 (感謝 ghoster 提供奸招:P) */
  else if (numWhite == 0)
  {
    winside = Black;
  }
  else if (numBlack == 0)
  {
    winside = White;
  }
#endif
  return winside;
}

static int othAllow(/*int Color,that is myColor*/)
{
  int i,j,p,q, num=0;
  for(i=0;i<8;i++)
    for(j=0;j<8;j++)
    {
      Allow[i][j]=0;
      if(Board[i][j]==Empty)
      {
        for(p=-1;p<=1;p++)
          for(q=-1;q<=1;q++)
            if(p!=0 || q!=0)
            {
              if(othEatable(myColor, i,j, p,q))
              {
                Allow[i][j]=1;
                num++;
                goto next;
              }
            }
      }
next:
    }
  return num;
}

static int (*othRule[])() = {othInit, othUpdate, othAllow};

/* Rules for five chess, 15 x 15 */
static int fivInit()
{
  int i,j;
  for(i=0;i<19;i++)
    for(j=0;j<19;j++)
      Board[i][j] = i<15 && j<15 ? Empty : Deny;
  numWhite = numBlack = 0;
  rulerRow = rulerCol = 15;
  return 0;
}

#if 0
#define LIVE_SIDE 0x10
#endif

static int fivCount(int Color, int row, int col, int rowstep, int colstep)
{
  int count = 0;
  for(;;)
  {
    row += rowstep;
    col += colstep;
    /* check range */
    if(row < 0 || row >=15 || col <0 || col >=15) return count;
#if 0
    /* Thor.990415: 判斷有誤, 見下 */
    /* Thor.990414: check live side */
    if(Board[row][col] == Empty) return count | LIVE_SIDE;
#endif
    if(Board[row][col] != Color) return count;
    count++;
  }
}

static int fivUpdate(int Color, int row, int col)
{
#if 0
  int cnt[4], n3, n4, n5, nL, i;
#endif

  int winside = Empty;
  Board[row][col] = Color;
  if(Color == Black) numBlack++;
  else if(Color == White) numWhite++;

#if 0
 ※ 引述《Daimon (吉事洋蔥酸黃瓜)》之銘言： 
                 /* Thor.990414:註:胖的人nick總是取這種 :p */
  黑棋（先著者）有下列三著禁著(又稱禁手)點：三三(雙活三)、四四、長連。
  在連五之前形成禁著者，裁定為禁著負。
  白棋沒有禁著點，長連或者三三也都可以勝。
  上面這段文章取自於“五子棋學院“
  http://www.tacocity.com.tw/shosho/index2.htm
  裡面有詳細的規則
#endif
#if 0
  cnt[0] = fivCount(Color,row,col,-1,-1) + fivCount(Color,row,col,+1,+1) + 1;
  cnt[1] = fivCount(Color,row,col,-1, 0) + fivCount(Color,row,col,+1, 0) + 1;
  cnt[2] = fivCount(Color,row,col, 0,-1) + fivCount(Color,row,col, 0,+1) + 1;
  cnt[3] = fivCount(Color,row,col,-1,+1) + fivCount(Color,row,col,+1,-1) + 1;

  n3 = 0; /* 雙活三 */
  n4 = 0; /* 雙四 */
  n5 = 0; /* 五 */
  nL = 0; /* 長連 */

  for(i = 0; i < 4; i++)
  {
    if(cnt[i] == (3 | (LIVE_SIDE + LIVE_SIDE))) n3++;
    if((cnt[i] % LIVE_SIDE) == 4) n4++;
    if((cnt[i] % LIVE_SIDE) == 5) n5++;
    if((cnt[i] % LIVE_SIDE) > 5) nL++;
  }

  if(n5 > 0)
    winside = Color;
  else
  {
    if(Color == Black)
    {
      static void printbwline();
      if(n3 >= 2)
      {
        printbwline("◆ 黑方雙三禁著"); 
        winside = White;
      }
      if(n4 >= 2)
      {
        printbwline("◆ 黑方雙四禁著"); 
        winside = White;
      }
      if(nL > 0)
      {
        printbwline("◆ 黑方長連禁著"); 
        winside = White;
      }
    }
    else
    {
      if(nL > 0)
        winside = Color;
    }
  }
#endif
#if 0

  /* Thor.990415: 上面那段又寫錯了, 留待有心人士再慢慢寫吧:pp */

 作者  tanx (逼逼為墮落之本)                                看板  sysop
 標題  Re: 五子棋有bug
 時間  Thu Apr 15 01:37:19 1999
───────────────────────────────────────

    嗯....我個人認為在玩五子棋時,最好可以自由選擇是否要有規則
    對業餘的人來說,有了禁手反而會變得沒有樂趣,有人就覺得ptt的
    五子棋不好玩(有禁手).
    至於規則我到了Daimon提供的網站看了一下,只要是四四是以下幾
    種情形

    (一)●  ●  ●  ●
              ↑
            再放進去就算四四
                ↓
    (二)●●●      ●●●(中間空三格)

    (三)●  ●●○
          ↑放這裡也算
              ●
                ●
                  ●

    (四)●●  ●    ●●
                ↑再放就是四四


    長連的話,只要五子以上就算,不管死活


#endif
#if 1
  if(fivCount(Color,row,col,-1,-1)+fivCount(Color,row,col,+1,+1)>=4
  || fivCount(Color,row,col,-1, 0)+fivCount(Color,row,col,+1, 0)>=4
  || fivCount(Color,row,col, 0,-1)+fivCount(Color,row,col, 0,+1)>=4
  || fivCount(Color,row,col,-1,+1)+fivCount(Color,row,col,+1,-1)>=4)
    winside = Color;
#endif
  return winside;
}

static int fivAllow(/*int Color*/)
{
  int i,j,num=0;
  for(i=0;i<19;i++)
    for(j=0;j<19;j++)
      num += Allow[i][j] = (Board[i][j] == Empty);
  return num;
}

static int (*fivRule[])() = { fivInit, fivUpdate, fivAllow};

/* Rules for blocking chess, 19 x 19 */
static int blkInit()
{
  memset(Board, 0, sizeof Board);
  numWhite = numBlack = 0;
  rulerRow = rulerCol = 18;
  return 0;
}

/* borrow Allow for traversal, and return region */
/* a recursive procedure, clear Allow before call it*/
/* with row,col range check, return false if out */
static int blkLive(int Color,int row, int col)
{
  if(row<0 || row >=19 || col<0 || col>=19) return 0;
  if(Board[row][col]==Empty) return 1;
  if(Board[row][col]!=Color) return 0;
  if(Allow[row][col]) return 0;
  Allow[row][col] = 1;
  return blkLive(Color, row-1,col) |
         blkLive(Color, row+1,col) |
         blkLive(Color, row, col-1)|
         blkLive(Color, row, col+1);
}

static inline void blkClear()
{
  int i,j;
  for(i=0;i<19;i++)
    for(j=0;j<19;j++)
      if(Allow[i][j]) Board[i][j] = Empty;
}

static int blkUpdate(int Color, int row, int col)
{
  Board[row][col] = Color;

#if 0
  /* Thor.990321: a little mistake:p  
      XO
     XO.O
      XO   X can eat O in "."
   */
  /* check for suiside */
  memset(Allow, 0, sizeof Allow);
  if(!blkLive(Color,row,col))
    blkClear();
  else{
    memset(Allow, 0, sizeof Allow);
    if(!blkLive(Deny-Color,row-1,col))
      blkClear();

    memset(Allow, 0, sizeof Allow);
    if(!blkLive(Deny-Color,row+1,col))
      blkClear();

    memset(Allow, 0, sizeof Allow);
    if(!blkLive(Deny-Color,row,col-1))
      blkClear();

    memset(Allow, 0, sizeof Allow);
    if(!blkLive(Deny-Color,row,col+1))
      blkClear();
  }
#endif
  memset(Allow, 0, sizeof Allow);
  if(!blkLive(Deny-Color,row-1,col))
    blkClear();

  memset(Allow, 0, sizeof Allow);
  if(!blkLive(Deny-Color,row+1,col))
    blkClear();

  memset(Allow, 0, sizeof Allow);
  if(!blkLive(Deny-Color,row,col-1))
    blkClear();

  memset(Allow, 0, sizeof Allow);
  if(!blkLive(Deny-Color,row,col+1))
    blkClear();

  /* check for suiside */
  memset(Allow, 0, sizeof Allow);
  if(!blkLive(Color,row,col))
    blkClear();

  /* count numWhite & numBlack */
  countBWnum();

  return Empty; /* Please check win side by your own */
}

/* borrow fivAllow as blkAllow */

static int (*blkRule[])() = { blkInit, blkUpdate, fivAllow};

/* rule set */            
static int (**ruleSet[])() = {othRule, fivRule, blkRule};
static char *ruleStrSet[]={"黑白棋","五子棋","圍棋"};
static char *ruleStr;

/* board util */
#if 0

  screen:
  [maple BWboard]   xxxx vs yyyy
  ++++++++  talkline(you color, yellow) (40 chars)
  ++++++++  talkline(my color, cryn)
  ++++++++
  ++++++++
  ++++++++
  1 line for simple help, press key to ......
  1 line for nth turn,myColor,num,pass <- youcolor, num,pass  (35)input talk
  2 line for write msg

  state:
                one step
     [my turn]--------[your turn]      mapTalk = NULL
      tab |             | tab
          +--+       +--+
            [talk mode]                mapTalk = map

#endif

static KeyFunc *mapTalk, *mapTurn;

static int bwRow, bwCol;
static int cmdCol, cmdPos;
static char talkBuf[42] = "T";
static char *cmdBuf = &talkBuf[1];

static char *
bw_brdline(int row)
{
  static char buf[80] = "\033[30;43m";
  static char rTxtY[] = " A B C D E F G H I J K L M N O P Q R S";
  static char rTxtX[] = " 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9";
  char txt[3];
  char *ptr = buf + 8, *t;
  int i;

  for(i = 0; i < 19; i++)
  {
    t = icon[Board[row][i]];
    if(t == icon[Empty])
    {
      if(row==0)
      {
        if(i == 0)
          t = "╔";
        else if(i >= 18 || Board[row][i+1] == Deny)
          t = "╗";
        else
          t = "╤";
      }
      else if(row >= 18 || Board[row+1][i] == Deny)
      {
        if(i == 0)
          t = "╚";
        else if(i >= 18 || Board[row][i+1] == Deny)
          t = "╝";
        else
          t = "╧";
      }
      else
      {
        if(i == 0)
          t = "╟";
        else if(i >= 18 || Board[row][i+1] == Deny)
          t = "╢";
      }
    }
    if(t != icon[Black] && t != icon[White])
    {
      if(row == rulerRow && i < rulerCol){
        str_ncpy(txt, rTxtX + 2 *i , 3);
        t = txt; 
      }
      else if(i == rulerCol && row < rulerRow){
        str_ncpy(txt, rTxtY + 2 * row , 3);
        t = txt;
      }
    }
    strcpy(ptr, t);
    ptr += 2;
  }
  strcpy(ptr,"\033[m");
  return buf;
}

static char*
bw_coorstr(row, col)
  int row;
  int col;
{
  static char coor[10];
  sprintf(coor,"(%c,%d)",row + 'A', col + 1);
  return coor;
} 

static void
printbwline(msg)
  char *msg;
{
  int line;

  line = bwline;
  move(line, 0);
  outs(bw_brdline(line-1)); outs(msg); clrtoeol();
  if (++line == stop_line)
    line = 1;
  move(line, 0);
  outs(bw_brdline(line-1)); outs("→"); clrtoeol();
  bwline = line;
}

static void
bw_draw()
{
  int i, myNum, yourNum;
  char buf[80];
  for(i=0;i<19;i++)
  {
    move(1+i,0);
    outs(bw_brdline(i));
  }
  myNum = myColor == Black ? numBlack : numWhite;
  yourNum = myColor == Black ? numWhite : numBlack;

  sprintf(buf,"第%d回 %s%d子%d讓"
              " (%s) %s%d子%d讓",
              BMIN(myStep,yourStep) + 1, icon[Deny - myColor], myNum, myPass,
              (mapTurn == myTurn ? "←" : "→"), icon[myColor], yourNum, yourPass);
  /* Thor.990219: 特別注意, 在此因顏色關係, 故用子的icon, 剛好與原本相反 */
  move(21,0); outs(buf);
  /* nth turn,myColor,num,pass <- youcolor, num,pass */
}

static void
bw_init()
{
  char *t,msg[160], buf[80];
  int i, myTotal=0, yourTotal=0, myWin=0, yourWin=0;

  /* Initialize BWboard */
  (*rule[Binit])();

  yourPass = myPass= yourStep = myStep = 0;
  /* Initialize state */
  mapTalk = Talk;

  if(myColor==Black)
  {
    (*rule[Ballow])();
    mapTurn = myTurn;
  }
  else
  {
    mapTurn = yourTurn;
  }
  /* Initialize screen */
  clear();

  /* sprintf(buf,"☆%s vs ★%s \033[m",cuser.userid, currutmp->chatid); */
  /* Thor.990311: dyanmic attribute */
  attr_get(cuser.userid, key_total, &myTotal);
  attr_get(cuser.userid, key_win, &myWin);
  attr_get(currutmp->chatid, key_total, &yourTotal);
  attr_get(currutmp->chatid, key_win, &yourWin);

  sprintf(buf,"☆%s(%d戰%d勝) vs ★%s(%d戰%d勝) \033[m",cuser.userid, myTotal, myWin, currutmp->chatid, yourTotal, yourWin);

  sprintf(msg,"\033[1;33;44m【 對奕%s 】", ruleStr);
  i = 80-strlen(buf)+3-strlen(msg)+10;
  t = str_tail(msg);
  for(;i;i--) *t++=' ';
  strcpy(t,buf);
  outs(msg);

#if 1
  sprintf(msg,"(sock:%d,icon:%s)%s %s",cfd,icon[myColor],ruleStr,buf);
  log_usies("BWBOARD",msg);
#endif

  move(20,0); 
  outs("\033[34;46m 對奕模式 \033[31;47m ( ,Enter)\033[30m落子 \033[31m(TAB)\033[30m切換棋盤/交談 \033[31m(^P)\033[30m讓手 \033[31m(^C)\033[30m重玩 \033[31m(^D)\033[30m離開 \033[m");
  bw_draw();

  bwRow = bwCol = 0;
  cmdCol = 0;
  *cmdBuf = 0;

  bwline = 1;
  GameOver = Empty;
  cmdPos = -1;
}

static inline void
overgame()
{
  if(GameOver == Black)
    printbwline("\033[1;32m◆ 黑方獲勝\033[m");
  else if(GameOver == White)
    printbwline("\033[1;32m◆ 白方獲勝\033[m");
  else if(GameOver==Deny)
    printbwline("\033[1;32m◆ 雙方平手\033[m");

  /* Thor.990311: dyanmic attribute */
  attr_step(cuser.userid, key_total, 0, +1); 
#if 0
  myTotal++;
  attr_put(cuser.userid,key_total, &myTotal);
#endif
  if(myColor == GameOver) 
  {
    attr_step(cuser.userid, key_win, 0, +1); 
#if 0
    myWin++;
    attr_put(cuser.userid,key_win, &myWin);
#endif
  }
}

static inline int
bw_send(buf)
  char *buf;
{
  int len;

  len = strlen(buf) + 1; /* with trail 0 */
  return (send(cfd, buf, len, 0) == len);
}

#ifdef EVERY_BIFF
static void check_biff()
{
      /* Thor.980805: 有人在旁邊按enter才需要check biff */
      static int old_biff;
      int biff = currutmp->ufo & UFO_BIFF;
      if (biff && !old_biff)
        printbwline("◆ 噹! 郵差來按鈴了!");
      old_biff = biff;
}
#endif

#if 0

  communication protocol:
      Ctrl('A') enter BWboard mode, (pass to another)
      first hand specify rule set (pass #Rule later)
      then start

      clear chess board, C.....\0
      talk line by line, T.....\0
      specify down pos, Dxxx\0 , y = xxx / 19, x = xxx % 19
      pass one turn, P.....\0
      leave BWboard mode, Q.....\0

#endif

static inline int
bw_recv()
{
  static char buf[512];
  static int bufstart = 0;
  int cc, len;
  char *bptr, *str;
  char msg[80];
  int i;

  bptr = buf;
  cc = bufstart; 
  len = sizeof(buf) - cc - 1;

  if ((len = recv(cfd, bptr + cc, len, 0)) <= 0)
    return DISCONNECT;

  cc += len;

  for (;;)
  {
    len = strlen(bptr);

    if (len >= cc)
    {				/* wait for trailing data */
      memcpy(buf, bptr, len);
      bufstart = len;
      break;
    }
    str = bptr + 1;
    switch(*bptr)
    {
     /* clear chess board, C.....\0 */
     case 'C':
      bw_init();
      break;

     /* talk line by line, T.....\0 */
     case 'T':
      sprintf(msg,"\033[1;33m★%s\033[m",str);
      printbwline(msg);
      break;

     /* specify down pos, Dxxx\0 , y = xxx / 19, x = xxx % 19 */
     case 'D':
      yourStep++;
      /* get pos */
      i = atoi(str);
      sprintf(msg,"◆ 對方落子 %s",bw_coorstr(i/19,i%19));
      /* update board */
      GameOver = (*rule[Bupdate])(Deny-myColor, i / 19, i % 19);

      mapTurn = myTurn;

      bw_draw();

      printbwline(msg);

      if(GameOver)
      {
        overgame();
        memset(Allow,0,sizeof Allow);
      }
      else
      {
        if((*rule[Ballow])()<=0)
          printbwline("◆ 您走投無路了");
      }
      break;

     /* pass one turn, P.....\0 */
     case 'P':
      yourPass++; yourStep++;

      mapTurn = myTurn;
      bw_draw();
      printbwline("◆ 對方讓手");
      if(GameOver)
      {
        /* overgame(); */
        memset(Allow,0,sizeof Allow);
      }
      else
      {
        if((*rule[Ballow])()<=0)
          printbwline("◆ 您走投無路了"); /* Thor.990329: ending game? */
      }
      break;

     /* leave BWboard mode, Q.....\0 */
     case 'Q':
      return LEAVE;
    }

    cc -= ++len;
    if (cc <= 0)
    {
      bufstart = 0;
      break;
    }
    bptr += len;
  }

  return NOTHING;
}

static int ftkCtrlC()
{
  *cmdBuf = '\0';
  cmdCol = 0;
  move(b_lines - 2, 35);
  clrtoeol();
  return NOTHING;
}

static int fCtrlD()
{
  /* send Q.....\0 cmd */
  if(!bw_send("Q"))
    return DISCONNECT;
  return LEAVE;
}

static int ftkCtrlH()
{
  if (cmdCol)
  {
    int ch = cmdCol--;
    memcpy(&cmdBuf[cmdCol], &cmdBuf[ch], sizeof talkBuf - ch -1);
    move(b_lines - 2, cmdCol + 35);
    outs(&cmdBuf[cmdCol]);
    clrtoeol();
  }
  return NOTHING;
}

static int ftkEnter()
{
  char msg[80];
#ifdef EVERY_BIFF
  check_biff();
#endif

  if (*cmdBuf)
  {
#if 0
        /* Thor.990218: reserve for "/" command */
	if (ch == '/')
	  ch = chat_cmd(cfd, ptr);
#endif
    for (cmdPos = MAXLASTCMD - 1; cmdPos; cmdPos--)
      strcpy(lastcmd[cmdPos], lastcmd[cmdPos - 1]);
    strcpy(lastcmd[0], cmdBuf);

    if(!bw_send(talkBuf))
      return DISCONNECT;

    sprintf(msg,"\033[1;36m☆%s\033[m",cmdBuf);
    printbwline(msg);

    *cmdBuf = '\0';
    cmdCol = 0;
    cmdPos = -1;
    move(b_lines - 2, 35);
    clrtoeol();
  }
  return NOTHING;
}

static int ftkLEFT()
{
  if (cmdCol)
    --cmdCol;
  return NOTHING;
}

static int ftkRIGHT()
{
  if (cmdBuf[cmdCol])
    ++cmdCol;
  return NOTHING;
}

static int ftkUP()
{
  cmdPos++;
  cmdPos %= MAXLASTCMD;
  str_ncpy(cmdBuf, lastcmd[cmdPos], 41);
  move(b_lines - 2, 35);
  outs(cmdBuf);
  clrtoeol();
  cmdCol = strlen(cmdBuf);
  return NOTHING;
}

static int ftkDOWN()
{
  cmdPos += MAXLASTCMD - 2;
  return ftkUP();
}

static int ftkDefault(int ch)
{
  if(isprint2(ch))
  {
    if (cmdCol < 40)
    {
      if (cmdBuf[cmdCol])
      {                       /* insert */
        int i;
        for (i = cmdCol; cmdBuf[i] && i < 39; i++);
        cmdBuf[i + 1] = '\0';
        for (; i > cmdCol; i--)
          cmdBuf[i] = cmdBuf[i - 1];
      }
      else
      {                       /* append */
        cmdBuf[cmdCol + 1] = '\0';
      }
      cmdBuf[cmdCol] = ch;
      move(b_lines - 2, cmdCol + 35);
      outs(&cmdBuf[cmdCol++]);
    }
    return NOTHING;
  }
}

static int ftnCtrlC()
{
#if 0
  int ch;
  /* Thor.980602: 由於 answer會用到igetch,
                  為防 I_OTHERDATA造成當住, 在此用 ctrlZ_everywhere方式,
                  保存vio_fd, 待問完後再還原 */
  /* Thor.980602: 暫存 vio_fd */
  holdon_fd = vio_fd;
  vio_fd = 0;

  ch = answer("do you want clear brd?[y/N]");

  /* Thor.980602: 還原 vio_fd */
  vio_fd = holdon_fd;
  holdon_fd = 0;

  if(ch == 'y')
  {
    if(!bw_send("C"))
      return DISCONNECT;
    bw_init();

  }
#endif
  if(!bw_send("C"))
    return DISCONNECT;
  bw_init();
  return NOTHING;
}

static int ftnUP()
{
  if(bwRow)
    bwRow--;
  return NOTHING;
}

static int ftnDOWN()
{
  if(bwRow < 18)
    if(Board[bwRow+1][bwCol]!=Deny)
      bwRow ++;
  return NOTHING;
}

static int ftnLEFT()
{
  if(bwCol)
    bwCol--;
  return NOTHING;
}

static int ftnRIGHT()
{
  if(bwCol < 18)
    if(Board[bwRow][bwCol+1]!=Deny)
      bwCol++;
  return NOTHING;
}

static int ftnPass()
{
  /* Thor.990220: for chat mode to enter ^P pass */
  if(mapTurn == myTurn)
  {
    myPass++; myStep++;
    if(!bw_send("P"))
      return DISCONNECT;
    mapTurn = yourTurn;
    bw_draw();
    printbwline("◆ 我方讓手");
  }
  return NOTHING;
}

static int ftnEnter()
{
  char msg[80];
  char buf[20];

  if(!Allow[bwRow][bwCol]) return NOTHING;
  
  sprintf(msg,"◆ 我方落子 %s",bw_coorstr(bwRow,bwCol));

  myStep++;
  sprintf(buf,"D%d",bwRow * 19 + bwCol);

  if(!bw_send(buf))
    return DISCONNECT;

  /* update board */
  GameOver = (*rule[Bupdate])(myColor, bwRow, bwCol);

  mapTurn = yourTurn;

  bw_draw();

  printbwline(msg);

  if(GameOver)
    overgame();

  return NOTHING;
}

static int fTAB()
{
  mapTalk = mapTalk ? NULL : Talk;
  return NOTHING;
}

static int fNoOp()
{
  return NOTHING;
}

static KeyFunc
Talk[] =
{
 Ctrl('C'), ftkCtrlC,
 Ctrl('D'), fCtrlD,
 Ctrl('H'), ftkCtrlH,
 Ctrl('P'), ftnPass,
 '\n', ftkEnter,
 KEY_LEFT, ftkLEFT,
 KEY_RIGHT, ftkRIGHT,
 KEY_UP, ftkUP,
 KEY_DOWN, ftkDOWN,
 KEY_TAB, fTAB,
 0, ftkDefault
},
yourTurn[] =
{
 Ctrl('C'), ftnCtrlC,
 Ctrl('D'), fCtrlD,
 KEY_LEFT, ftnLEFT,
 KEY_RIGHT, ftnRIGHT,
 KEY_UP, ftnUP,
 KEY_DOWN, ftnDOWN,
 KEY_TAB, fTAB,
 0, fNoOp
},
myTurn[] =
{
 Ctrl('C'), ftnCtrlC,
 ' ', ftnEnter,
 '\n', ftnEnter,
 Ctrl('P'), ftnPass,
 Ctrl('D'), fCtrlD,
 KEY_LEFT, ftnLEFT,
 KEY_RIGHT, ftnRIGHT,
 KEY_UP, ftnUP,
 KEY_DOWN, ftnDOWN,
 KEY_TAB, fTAB,
 0, fNoOp
};

/* bwboard main */
int BWboard(int sock, int later)
{
  screenline sl[b_lines];
  int ch;
  char c;
  KeyFunc *k;

  vs_save(sl);

  cfd = sock;
  if(!later)
  {
    /* ask for which rule set */
    /* assume: peer won't send char until setup */
    c = answer("想下哪種棋? 0)取消 1)黑白棋 2)五子棋 3)圍棋 [0]");
    if(c) c-='0';
    else 
      vs_restore(sl);	/* lkchu.990428: 把 foot restore 回來 */

    /* transmit rule set */
    if(send(cfd, &c , 1, 0) != 1)
      return -2;

    myColor = Black;
  }
  else
  {
    /* prompt for waiting rule set */
    outmsg("對方要求進入對奕模式,選擇中請稍候.....");
    refresh();
    /* receive rule set */
    if(recv(cfd, &c , 1, 0) != 1)
      return -2;
    
    vs_restore(sl);		/* lkchu.990428: 把 foot restore 回來 */
    myColor = White;
  }

  if(!c--) return -1;
  rule = ruleSet[c];
  ruleStr = ruleStrSet[c];

  /* Thor.990311: dynamic attribute */
  key_total = ATTR_OTHELLO_TOTAL + ((unsigned)c << 8);
  key_win = ATTR_OTHELLO_WIN + ((unsigned)c << 8);

  /* initialize all */
  bw_init();

  for(;;)
  {
    if(mapTalk)
    {
      move(b_lines - 2, cmdCol + 35);
      k = mapTalk;
    }
    else
    {
#if 0
      move(bwRow + 1, bwCol * 2);
#endif
      /* Thor.990222: for crxvt */
      move(bwRow + 1, bwCol * 2 + 1);
      k = mapTurn;
    }

    ch = igetch();
    if (ch == I_OTHERDATA)
    {				/* incoming */
      if((ch = bw_recv()) >= NOTHING)  /* -1 for exit bwboard, -2 for exit talk */
        continue;
      vs_restore(sl);
      return ch;
    }
#ifdef EVERY_Z
    /* Thor: Chat 中按 ctrl-z */
    else if (ch == Ctrl('Z'))
    {
      char buf[IDLEN + 1];

      /* Thor.0731: 暫存 chatid, 因為出去時可能會用掉 chatid(like query) */
      strcpy(buf, currutmp->chatid);

      /* Thor.0727: 暫存 vio_fd */
      holdon_fd = vio_fd;
      vio_fd = 0;
      every_Z();
      /* Thor.0727: 還原 vio_fd */
      vio_fd = holdon_fd;
      holdon_fd = 0;

      /* Thor.0731: 還原 chatid, 因為出去時可能會用掉 chatid(like query) */
      strcpy(currutmp->chatid, buf);
      continue;
    }
#endif

    for(;;k++)
    {
      if(!k->key || ch == k->key)
        break;
    }

    /* -1 for exit bwboard, -2 for exit talk */
    if((ch = k->key ? (*k->func)() : (*k->func)(ch)) >= NOTHING)
      continue;
    vs_restore(sl);
    return ch;
  }
}

#include<stdarg.h>
int vaBWboard(va_list pvar)
{
  int sock, later;
  sock = va_arg(pvar, int);
  later = va_arg(pvar,int);
  return BWboard(sock, later);
}
