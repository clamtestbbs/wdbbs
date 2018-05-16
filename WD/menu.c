/*-------------------------------------------------------*/
/* menu.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines                     */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"

/* ----------------------------------------------------- */
/* Menu Commands struct                                  */
/* ----------------------------------------------------- */

struct MENU
{
  void *cmdfunc;
//  int (*cmdfunc) ();
  usint level;
  char *desc;                   /* next/key/description */
  int mode;
};
typedef struct MENU MENU;


/* ------------------------------------- */
/* help & menu processring               */
/* ------------------------------------- */
int refscreen = NA;
extern char *boardprefix;


void
showtitle(title, mid)
  char *title, *mid;
{
  char buf[40];
  int spc, pad;

  spc = strlen(mid);

  if (title[0] == 0)
    title++;
   else if (chkmail(0))
  {
    mid = "\033[41;33;5m   «H½c¸Ì­±¦³·s«H­ò¡I  \033[m\033[1m"COLOR1;

/*
 * CyberMax:
 *           spc ¬O¤Ç°t mid ªº¤j¤p.
 */
    spc = 22;
  }
  else if(check_personal_note(1,NULL))
  {
    mid = "\033[43;37;5m    µª¿ý¾÷¤¤¦³¯d¨¥³á   \033[m\033[1m"COLOR1;
    spc = 22;
  }
  else if (dashf(BBSHOME"/register.new") && HAS_PERM(PERM_ACCOUNTS))
  {
    mid = "\033[45;33;5m  ¦³·sªº¨Ï¥ÎªÌµù¥UÅo!  \033[m\033[1m"COLOR1;
    spc = 22;
  }

  spc = 66 - strlen(title) - spc - strlen(currboard);
/*
woju
*/
  if (spc < 0)
     spc = 0;
  pad = 1 - spc & 1;
  memset(buf, ' ', spc >>= 1);
  buf[spc] = '\0';

  move(0,0);
  clrtobot();
//  clear();
  prints(COLOR2"  \033[1;36m%s  "COLOR1"%s\033[33m%s%s%s\033[3%s\033[1m "COLOR2"  \033[36m%s  \033[m\n",
    title, buf, mid, buf, " " + pad,
    currmode & MODE_SELECT ? "1m¨t¦C" :
    currmode & MODE_DIGEST ? "5m¤åºK" : "7m¬ÝªO", currboard);

}

// wildcat : ¤À®æ½u¥ÎªºªÅ¿ï³æ :p
int
null_menu()
{
  pressanykey("³o¬O¤@­ÓªÅ¿ï³æ :p ");
  return 0;
}


/* ------------------------------------ */
/* °Êµe³B²z                              */
/* ------------------------------------ */


#define FILMROW 11
unsigned char menu_row = 3;
unsigned char menu_column = 4;
char mystatus[512];


/* wildcat 1998.8.7 */

void
movie(i)
  int i;
{
  extern struct FILMCACHE *film;
  static short history[MAX_HISTORY];
  static char myweek[] = "¤é¤@¤G¤T¥|¤­¤»";
  static char buf[512],pbuf[512];
  char *msgs[] = {"Ãö", "¶}", "©Þ", "¨¾","¤Í"};
  time_t now = time(NULL);
  struct tm *ptime = localtime(&now);

  resolve_garbage(); /* get film cache */

  if (currstat == GAME) return;
  if (HAVE_HABIT(HABIT_MOVIE))
  {
    if((!film->busystate) && film->max_film) /* race condition */
    {
      do{
        if (i != 1 || i != 0 || i != 999)
          i = 1 + (rand()%film->max_film);

        for (now = film->max_history; now >= 0; now--)
        if (i == history[now])
        {
          i = 0;
          break;
        }
      } while (i == 0);
    }
//    else i = 1;
    memcpy(history, &history[1], film->max_history * sizeof(short));
    history[film->max_history] = now = i;

    if (i == 999)       /* Goodbye my friend */
      i = 0;
    setapath(pbuf, "Note");
    sprintf(buf, "%s/%s", pbuf, film->notes[i]);
    if(film->notes[i][0])
      show_file(buf,13,FILMROW,NO_RELOAD);
  }
  i = ptime->tm_wday << 1;
  update_data();
  sprintf(mystatus, "\033[1;36m[%d:%02d %c%c %0d/%0d]"
"\033[1;37m ID: %-13s ¢C\033[37m%6d%c,\033[33m%5d%c"
"\033[32m[£]]%-2.2s \033[35m[%-20.20s]\033[m",
    ptime->tm_hour, ptime->tm_min, myweek[i], myweek[i + 1],
    ptime->tm_mon + 1, ptime->tm_mday, cuser.userid, 
    (cuser.silvermoney/1000) <= 0 ? cuser.silvermoney : cuser.silvermoney/1000,
    (cuser.silvermoney/1000) <= 0 ? ' ' : 'k',
    (cuser.goldmoney/1000) <= 0 ? cuser.goldmoney : cuser.goldmoney/1000,
    (cuser.goldmoney/1000) <= 0 ? ' ' : 'k',    
    msgs[currutmp->pager],
    currutmp->birth ? "¥Í¤é°O±o­n½Ð«È­ò!!" : film->today_is);
  move(1,0);
  clrtoeol();
  outs(mystatus);
  refresh();
}


/* ===== end ===== */


static int
show_menu(p)
  MENU *p;
{
  register int n = 0, m = 0;
  int i = 0;
  register char *s;
          char buf[512],fn[512];
  char buf2[11][512] = {"\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0"};
//  char buf2[][];
  FILE *fp;
#ifdef HAVE_NOTE_2
  sprintf(fn,BBSHOME"/m2/%d",(rand()%10)+1);
  if(fp = fopen(fn,"r"))
  {
    while(fgets(buf2[i],512,fp) != NULL)
      i++;
    fclose(fp);
  }
#endif
  movie(0);
  move(2,0);
#ifdef HYPER_BBS
  prints(COLOR1"\033[1m"HB_BACK" ¥\\ ¯à       »¡    ©ú                 «ö \033[1;33m\033[200m\033[444m\033[626m[Ctrl-Z]\033[201m\033[37m \033[31m¨D§U               \033[m");
#else
  prints(COLOR1"\033[1m         ¥\\  ¯à        »¡    ©ú                 «ö [\033[1;33mCtrl-Z\033[37m] \033[31m¨D§U               \033[m");
#endif
  move(menu_row, 0);
  while ((s = p[n].desc)!=NULL || buf2[m][0]!='\0')
  {
    if ( s != NULL )
    {
      if (HAS_PERM(p[n].level))
      {
        sprintf(buf,s+2);
#ifdef HAVE_NOTE_2
        if(currstat == FINANCE || currstat == GAME || currstat == RMENU
          || buf2[m][0]=='\0' )
#endif
  #ifdef HYPER_BBS
          prints("%*s  \033[200m\033[446m[\033[1;36m\033[300m%c\033[302m\033[m]%s\033[201m\n", 
  #else
          prints("%*s  [\033[1;36m%c\033[m]%s\n", 
  #endif
            menu_column, "", s[1], buf);
#ifdef HAVE_NOTE_2
        else
  #ifdef HYPER_BBS
          prints("%*s  \033[200m\033[446m[\033[1;36m\033[300m%c\033[302m\033[m]%-28s\033[201m%s",
  #else
          prints("%*s  [\033[1;36m%c\033[m]%-28s%s",
  #endif
            menu_column, "", s[1], buf,buf2[m++]);
#endif
      }
      n++;
    }
#ifdef HAVE_NOTE_2
    else
    {
      if (currstat == FINANCE || currstat == GAME || currstat == RMENU)
        break;
      prints("%37s%-s", "", buf2[m++] );
    }
#endif
  }
  return n - 1;
}


void
domenu(cmdmode, cmdtitle, cmd, cmdtable)
  char *cmdtitle;
  int cmdmode, cmd;
  MENU *cmdtable;
{

  int lastcmdptr;
  int n, pos, total, i;
  int err;
  int chkmailbox();
  static char cmd0[LOGIN];

  if (cmd0[cmdmode])
     cmd = cmd0[cmdmode];

  setutmpmode(cmdmode);
  sprintf(tmpbuf,"%s [½u¤W %d ¤H]",BOARDNAME,count_ulist());

  showtitle(cmdtitle, tmpbuf);
  total = show_menu(cmdtable);
  move(1,0);
  outs(mystatus);

  lastcmdptr = pos = 0;

  do
  {
    i = -1;

    switch (cmd)
    {
    case KEY_ESC:
       if (KEY_ESC_arg == 'c')
          capture_screen();
       else if (KEY_ESC_arg == 'n') {
          edit_note();
          refscreen = YEA;
       }
       i = lastcmdptr;
       break;
    case Ctrl('N'):
       New();
       refscreen = YEA;
       i = lastcmdptr;
       break;
    case Ctrl('A'):
    {
      int stat0 = currstat;
      currstat = RMAIL;
      if (man() == RC_FULL)
        refscreen = YEA;
      i = lastcmdptr;
      currstat = stat0;
      break;
    }
    case KEY_DOWN:
      i = lastcmdptr;

    case KEY_HOME:
    case KEY_PGUP:
      do
      {
        if (++i > total)
          i = 0;
      } while (!HAS_PERM(cmdtable[i].level));
      break;

    case KEY_END:
    case KEY_PGDN:
      i = total;
      break;

    case KEY_UP:
      i = lastcmdptr;
      do
      {
        if (--i < 0)
          i = total;
      } while (!HAS_PERM(cmdtable[i].level));
      break;

    case KEY_LEFT:
    case 'e':
    case 'E':
      if (cmdmode == MMENU)
        cmd = 'G';
      else if ((cmdmode == MAIL) && chkmailbox())
        cmd = 'R';
      else return;
    default:
       if ((cmd == Ctrl('G') || cmd == Ctrl('S')) && (currstat == MMENU || currstat == TMENU || currstat == XMENU))  {
          if (cmd == Ctrl('S'))
             ReadSelect();
          else if (cmd == Ctrl('G'))
            Read();
          refscreen = YEA;
          i = lastcmdptr;
          break;
        }
      if (cmd == '\n' || cmd == '\r' || cmd == KEY_RIGHT)
      {

        boardprefix = cmdtable[lastcmdptr].desc;

        if(cmdtable[lastcmdptr].mode && DL_get(cmdtable[lastcmdptr].cmdfunc))
        {
          void *p = (void *)DL_get(cmdtable[lastcmdptr].cmdfunc);
          if(p) cmdtable[lastcmdptr].cmdfunc = p;
          else break;
        }

        currstat = XMODE;

        {
          int (*func)() = 0;

          func = cmdtable[lastcmdptr].cmdfunc;
          if(!func) return;
          if ((err = (*func)()) == QUIT)
            return;
        }

        currutmp->mode = currstat = cmdmode;

        if (err == XEASY)
        {
          refresh();
          sleep(1);
        }
        else if (err != XEASY + 1 || err == RC_FULL)
          refscreen = YEA;

        if (err != -1)
          cmd = cmdtable[lastcmdptr].desc[0];
        else
          cmd = cmdtable[lastcmdptr].desc[1];
        cmd0[cmdmode] = cmdtable[lastcmdptr].desc[0];
      }

      if (cmd >= 'a' && cmd <= 'z')
        cmd &= ~0x20;
      while (++i <= total)
      {
        if (cmdtable[i].desc[1] == cmd)
          break;
      }
    }

    if (i > total || !HAS_PERM(cmdtable[i].level))
    {
      continue;
    }

    if (refscreen)
    {
      showtitle(cmdtitle, tmpbuf);
      show_menu(cmdtable);
      move(1,0);
      outs(mystatus);
      refscreen = NA;
    }

    cursor_clear(menu_row + pos, menu_column);
    n = pos = -1;
    while (++n <= (lastcmdptr = i))
    {
      if (HAS_PERM(cmdtable[n].level))
        pos++;
    }
    cursor_show(menu_row + pos, menu_column);
  } while (((cmd = igetkey()) != EOF) || refscreen);

  abort_bbs();
}
/* INDENT OFF */


/* ----------------------------------------------------- */
/* administrator's maintain menu                         */
/* ----------------------------------------------------- */

int m_user(), m_newbrd(), m_board(), m_register(),x_reg(),XFile(),
    search_key_user(),search_bad_id,reload_cache(),adm_givegold();
/*    ,search_bad_id();*/

#ifdef  HAVE_MAILCLEAN
int m_mclean();
#endif

static struct MENU adminlist[] = {
  m_user,       PERM_ACCOUNTS,  "UUser          [¨Ï¥ÎªÌ¸ê®Æ]",0,
  search_key_user,PERM_ACCOUNTS,"FFind User     [·j´M¨Ï¥ÎªÌ]",0,
  m_newbrd,     PERM_BOARD,     "NNew Board     [¶}±Ò·s¬ÝªO]",0,
  m_board,      PERM_BOARD,     "SSet Board     [ ³]©w¬ÝªO ]",0,
  m_register,   PERM_ACCOUNTS,  "RRegister      [¼f®Öµù¥U³æ]",0,
  XFile,        PERM_SYSOP,     "XXfile         [­×§ï¨t²ÎÀÉ]",0,
  reload_cache, PERM_SYSOP,     "CCache Reload  [ §ó·sª¬ºA ]",0,
  adm_givegold, PERM_SYSOP,     "GGive $$       [ µo©ñ¼úª÷ ]",0,
"SO/xyz.so:x_bbsnet",PERM_SYSOP,"BBBSNet        [ ³s½u¤u¨ã ]",1,
/*
  m_mclean, PERM_BBSADM, "MMail Clean    ²M²z¨Ï¥ÎªÌ­Ó¤H«H½c",0,
#endif */
  x_reg,        PERM_ACCOUNTS,  "MMerge         [¼f®Ö­×²z¾÷]",0,

NULL, 0, NULL,0};


/* ----------------------------------------------------- */
/* class menu                                            */
/* ----------------------------------------------------- */

int board(), local_board(), Boards(), ReadSelect() ,
    New(),Favor(),favor_edit(),good_board(),voteboard();

static struct MENU classlist[] = {
   voteboard, 0,      "VVoteBoard    [¬ÝªO³s¸p¨t²Î]",0,
   board, 0,          "CClass        [¥»¯¸¤ÀÃþ¬ÝªO]",0,
   New, 0,            "NNew          [©Ò¦³¬ÝªO¦Cªí]",0,
   local_board,0,     "LLocal        [¯¸¤º¬ÝªO¦Cªí]",0,
   good_board,0,      "GGoodBoard    [  Àu¨}¬ÝªO  ]",0,
   Favor,PERM_BASIC,  "BBoardFavor   [§Úªº³Ì·R¬ÝªO]",0,
favor_edit,PERM_BASIC,"FFavorEdit    [½s¿è§Úªº³Ì·R]",0,
   ReadSelect, 0,     "SSelect       [  ¿ï¾Ü¬ÝªO  ]",0,
   NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* RPG menu                                             */
/* ----------------------------------------------------- */
int /* t_pk(),*/rpg_help();

struct MENU rpglist[] = {
  rpg_help,0,	"HHelp       ¥»¹CÀ¸¤¶²Ð/³W«h»¡©ú",0,
  "SO/rpg.so:rpg_uinfo",
    0,		"UUserInfo   ¦Û¤vªºª¬ºA",1,
  "SO/rpg.so:rpg_race_c",
    PERM_BASIC,	"JJoin       ¥[¤JÂ¾·~¤u·|(»Ý¤­ªTª÷¹ô)",1,
  "SO/rpg.so:rpg_guild",
    0,		"GGuild      Â¾·~¤u·|",1,
  "SO/rpg.so:rpg_train",
    0,		"TTrain      °V½m³õ(¬I¤u¤¤)",1,
  "SO/rpg.so:rpg_top",
    0,		"LListTop    ¨Ï¥ÎªÌ±Æ¦æº]",1,
  "SO/rpg.so:rpg_edit",
    PERM_SYSOP,	"QQuery      ¬d¸ß­×§ïUSER¸ê®Æ",1,
//  "SO:rpg.so:rpg_shop",0,           "SShop       ¸Ë³Æ°Ó©±(¬I¤u¤¤)",1,
//  "SO:rpg.so:t_pk",0,                 "PPK         ¤£ºâ PK",1,
NULL, 0, NULL,0};

int
rpg_menu()
{
  domenu(RMENU, "¨¤¦â§êºt¹CÀ¸", 'U', rpglist);
  return 0;
}
/* ----------------------------------------------------- */
/* Ptt money menu                                        */
/* ----------------------------------------------------- */

static struct MENU finlist[] = {
  "SO/buy.so:bank",     0,      "11Bank           \033[1;36m­·¹Ð»È¦æ\033[m",1,
  "SO/pip.so:pip_money",0,      "22ChickenMoney   Âûª÷§I´«³B     ´«¿úµ¹¹q¤lÂû¥Î",1,
  "SO/song.so:ordersong",0,     "33OrderSong      \033[1;33m­·¹ÐÂIºq¾÷\033[m     $5g/­º",1,
  "SO/buy.so:p_cloak",  0,      "44Cloak          Á{®ÉÁô¨­/²{¨­  $2g/¦¸     (²{¨­§K¶O)",1,
  "SO/buy.so:p_from",   0,      "55From           ­×§ï¬G¶m       $5g/¦¸",1,
  "SO/buy.so:p_exmail", 0,      "66Mailbox        ÁÊ¶R«H½c¤W­­   $100g/«Ê",1,
  "SO/buy.so:p_fcloak", 0,      "77UltimaCloak    ²×·¥Áô¨­¤jªk   $500g      ¥i¥Ã¤[Áô§Î",1,
  "SO/buy.so:p_ffrom",  0,      "88PlaceBook      ¬G¶m­×§ïÄ_¨å   $1000g     User¦W³æ«öF¥i§ï¬G¶m",1,
  "SO/buy.so:p_ulmail", 0,      "99NoLimit        «H½cµL¤W­­     $100000g   «H½c¤W­­µL­­¨î",1,
NULL, 0, NULL,0};

int
finance()
{
  domenu(FINANCE, "ª÷¿Ä¤¤¤ß", '1', finlist);
  return 0;
}

#ifdef HAVE_GAME

/* ----------------------------------------------------- */
/* NetGame menu                                          */
/* ----------------------------------------------------- */

struct MENU netgame_list[] = {
  "SO/xyz.so:x_mj",0,
    "QQkmj      ¡¹ ºô¸ô³Â±N³õ",1,
  "SO/xyz.so:x_big2",0,
    "BBig2      ¡¹ ºô¸ô¤j¦Ñ¤G",1,
  "SO/xyz.so:x_chess",PERM_LOGINOK,
    "CChess     ¡¹ ºô¸ô¤U¶H´Ñ",1,
NULL, 0, NULL,0};

int
netgame_menu()
{
  domenu(NETGAME, "ºô¸ô³s½u¹CÀ¸", 'Q', netgame_list);
  return 0;
}
/* ----------------------------------------------------- */
/* Game menu                                             */
/* ----------------------------------------------------- */

static struct MENU gamelist[] = {
  rpg_menu,0,
    "RRPG        ¡½ ¨¤¦â§êºt¹CÀ¸           ¬I¤u¤¤",0,
  netgame_menu,0,
    "NNetGame    ¡½ ºô¸ô³s½u¹CÀ¸           $100s/¦¸",0,
  "SO/gamble.so:ticket_main",PERM_BASIC,
    "GGamble     ¡¹ ¹ï¹ï¼Ö½ä½L             $100s/±i",1,
  "SO/marie.so:mary_m",0,
    "MMarie      ¡¸ ¤pº¿ÄR¼Ö¶é             ³Ì§C®ø¶O $1s",1,
  "SO/race.so:race",PERM_BASIC,
    "HHorseRace  ¡¸ Ác¬P½ä°¨³õ             ³Ì§C®ø¶O $1s",1,
  "SO/bingo.so:bingo",PERM_BASIC,
    "BBingo      ¡¸ ¬Õ¤ë»«ªG¶é             ³Ì§C®ø¶O $1s",1,
  "SO/gagb.so:gagb",0,
    "??A?B       ¡¸ ²q²q²q¼Æ¦r             ³Ì§C®ø¶O $1s",1,
  "SO/guessnum.so:fightNum",0,
    "FFightNum   ¡¸ ¹ï¾Ô²q¼Æ¦r             ³Ì§C®ø¶O $1s",1,
  "SO/bj.so:BlackJack",0,
    "JJack       ¡¸ ¬Õ¤ë¶Â³Ç§J             ³Ì§C®ø¶O $1s",1,
  "SO/nine.so:p_nine",PERM_BASIC,
    "999         ¡¸ ¤Ñ¦a¤[¤E¤E¢            ³Ì§C®ø¶O $1s",1,
  "SO/dice.so:x_dice",0,
    "DDice       ¡¸ ¦è¤K©Ô½ä³õ             ³Ì§C®ø¶O $1s",1,
  "SO/gp.so:p_gp",0,
    "PPoke       ¡¸ ª÷¼³§J±ô«¢             ³Ì§C®ø¶O $1s",1,
  "SO/pip.so:p_pipple",PERM_LOGINOK,
    "CChicken    ¡» ­·¹Ð¹q¤lÂû             §K¶Oµ¹§Aª±!!",1,
  "SO/xyz.so:x_tetris",0,
    "TTetris     ¡» «XÃ¹´µ¤è¶ô             §K¶Oµ¹§Aª±",1,
  "SO/mine.so:Mine",0,
    "LLandMine   ¡» «lÃz½ò¦a¹p             §K¶Oµ¹§Aª±",1,
  "SO/poker.so:p_dragon",0,
    "11±µÀs      ¡» ´ú¸Õ¤¤ªº±µÀs           §K¶Oµ¹§Aª±",1,
  "SO/chessmj.so:p_chessmj",0,
    "22ChessMJ   ¡¸ ¶H´Ñ³Â±N               ³Ì§C®ø¶O $1s",1,
  "SO/seven.so:p_seven",0,
    "33Seven     ¡¸ ½ä«°¤C±i               ³Ì§C®ø¶O $1s",1,
  "SO/bet.so:p_bet",0,
    "44Bet       ¡¸ ºÆ¨g½ä½L               ³Ì§C®ø¶O $1s",1,
  "SO/stock.so:p_stock",PERM_BASIC,
    "SStock      ¡º ­·¹ÐªÑ¥«",1,

/*  x_bridgem,PERM_LOGINOK,"OOkBridge    ¡i ¾ôµPÄv§Þ ¡j",0,*/
NULL, 0, NULL,0};
#endif

/* ----------------------------------------------------- */
/* Talk menu                                             */
/* ----------------------------------------------------- */

int t_users(), t_idle(), t_query(), t_pager();
// t_chat(), t_list(), t_talk();
/* Thor: for ask last call-in message */
int t_display();

static struct MENU talklist[] = {

  t_users,      0,              "LList          [½u¤W¦W³æ]",0,
  t_pager,      PERM_BASIC,     "PPager         [¤Á´«ª¬ºA]",0,
  t_idle,       0,              "IIdle          [Âê©w¿Ã¹õ]",0,
  t_query,      0,              "QQueryUser     [¬d¸ßUser]",0,
/*
  t_talk,       PERM_PAGE,      "TTalk          [§ä¤H²á¤Ñ]",0,
 */
  "SO/chat.so:t_chat",PERM_CHAT,"CChatRoom      [³s½u²á¤Ñ]",1,
  t_display,    0,              "DDisplay       [¤ô²y¦^ÅU]",0,
  XFile,        PERM_XFILE,     "XXfile         [­×§ï¨t²ÎÀÉ]",0,
NULL, 0, NULL,0};

/*-------------------------------------------------------*/
/* powerbook menu                                        */
/* ----------------------------------------------------- */

int null_menu(),my_gem(),my_allpost();

static struct MENU powerlist[] = {

  "SO/bbcall.so:bbcall_menu",
                       0,	"MMessager      [ ³q°T¿ý ]",1,
  "SO/mn.so:show_mn",
	               0,	"NNoteMoney     [ °O±b¥» ]",1,
  my_gem,              0,       "GGem           [§ÚªººëµØ]",0,
  my_allpost,          0,       "AAllPost       [§Úªº¤å³¹]",0,
  null_menu,           0,	"------ µª¿ý¾÷ ¥\\¯à ------",0,
  "SO/pnote.so:p_read",0,       "PPhone Answer  [Å¥¨ú¯d¨¥]",1,
  "SO/pnote.so:p_call",0,       "CCall phone    [°e¥X¯d¨¥]",1,
  "SO/pnote.so:p_edithint",0,   "RRecord        [¿ýÅwªïµü]",1,

NULL, 0, NULL,0};

int
PowerBook()
{
  domenu(POWERBOOK, "¸U¥Î¤â¥U", 'N', powerlist);
  return 0;
}


/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

extern int u_editfile();
int u_info(), u_cloak(), u_list(), u_habit(), PowerBook(), ListMain();
#ifdef REG_FORM
int u_register();
#endif

#ifdef REG_MAGICKEY
int u_verify();
#endif

#ifdef POSTNOTIFY
int re_m_postnotify();
#endif

static struct MENU userlist[] = {
  PowerBook,	PERM_BASIC,	"PPowerBook     [¸U¥Î¤â¥U]",0,
  u_info,       0,              "IInfo          [­×§ï¸ê®Æ]",0,
  u_habit,      PERM_BASIC,     "HHabit         [³ß¦n³]©w]",0,
  ListMain,     PERM_LOGINOK,   "LList          [³]©w¦W³æ]",0, 

#ifdef POSTNOTIFY
  re_m_postnotify,PERM_BASIC,   "PPostNotify    [¼f®Ö¤å³¹³qª¾]",0,
#endif

  u_editfile,   PERM_LOGINOK,   "FFileEdit      [­Ó¤HÀÉ®×]",0,
  u_cloak,      PERM_CLOAK,     "CCloak         [Áô§Î±Kªk]",0,

#ifdef REG_FORM
  u_register,   PERM_POST,      "RRegister      [¶ñµù¥U³æ]",0,
#endif

#ifdef REG_MAGICKEY
  u_verify, 	PERM_BASIC,	"VVerify        [¶ñµù¥U½X]",0,
#endif

  u_list,       PERM_BASIC,     "UUsers         [µù¥U¦W³æ]",0,
NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* XYZ menu                                              */
/* ----------------------------------------------------- */
#ifdef HAVE_GAME
int
game_list()
{
  domenu(GAME, "ºô¸ô¹C¼Ö³õ", 'R', gamelist);
  return 0;
}
#endif

struct MENU netserv_list[] = {
"SO/bbcall.so:bbcall_main",
                PERM_LOGINOK,   "BB.B.Call      [ºô¸ô¶Ç©I]",1,
"SO/tv.so:catv",PERM_BASIC,     "TTV-Program    [¹qµø¸`¥Ø¬d¸ß]",1,
"SO/railway.so:railway2",
                PERM_BASIC,     "RRailWay       [¤õ¨®®É¨è¬d¸ß]",1,
"SO/fortune.so:main_fortune",
                PERM_BASIC,     "QQueryFortune  [­Ó¤H¹B¶Õ¹w´ú]",1,
#ifdef HAVE_GOPHER
  "SO/xyz.so:x_gopher",PERM_LOGINOK,"GGopher       ¡½ ¥@·s¤p¦a¹«¦øªA¾¹ ¡ö",1,
#endif
#ifdef BBSDOORS
  "SO/xyz.so:x_bbsnet", PERM_LOGINOK, "DDoor      ¡i ¨ä¥L BBS¯¸ ¡j",1,
#endif
NULL, 0, NULL,0};

int
net_serv()
{
  domenu(PMENU, "ºô¸ô³s½uªA°È", 'B', netserv_list);
  return 0;
}

int note();

static struct MENU servicelist[] = {
#ifdef HAVE_GAME
  game_list,    0,              "PPlay          [¨|¼Ö¤¤¤ß]",0,
#endif
  finance,      PERM_LOGINOK,   "FFinance       [°Ó«~¤jµó]",0,
  net_serv,	0,		"SServNet       [ºô¸ôªA°È]",0,
  "SO/xyz.so:KoK",
  		0,		"KKK-Service    [¸U¤ý¤§¤ý]",1,
  "SO/tobuy.so:x_tobuy",
  		PERM_LOGINOK,	"TToBuy         [¤G¤â¥æ©ö]",1,
  "SO/vote.so:all_vote",
                PERM_LOGINOK,   "VVote          [§ë²¼¤¤¤ß]",1,
  note,         PERM_LOGINOK,   "NNote          [¼g¯d¨¥ªO]",0,
  "SO/xyz.so:show_hint_message",
                0,              "HHint          [±Ð¾ÇºëÆF]",1,
/*
"SO/indict.so:x_dict",
                0,              "DDictionary    [¦Ê¬ì¥þ®Ñ]",1,
*/
"SO/xyz.so:x_cdict",
                PERM_BASIC,     "CCD-67         [¹q¤l¦r¨å]",1,

  NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* mail menu                                             */
/* ----------------------------------------------------- */
int m_new(), m_read(), m_send(),m_sysop(),mail_mbox(),mail_all(),
    setforward(),mail_list();

#ifdef INTERNET_PRIVATE_EMAIL
int m_internet();
#endif

static struct MENU maillist[] = {
  m_new,        PERM_READMAIL,  "RNew           [¾\\Åª·s«H]",0,
  m_read,       PERM_READMAIL,  "RRead          [«H¥ó¦Cªí]",0,
  m_send,       PERM_BASIC,     "SSend          [¯¸¤º±H«H]",0,
  mail_list,    PERM_BASIC,     "MMailist       [¸s²Õ±H«H]",0,
  m_internet,   PERM_INTERNET,  "IInternet      [ºô¸ô¶l¥ó]",0,
  setforward,   PERM_LOGINOK,   "FForward       [¦¬«HÂà±H]",0,
  m_sysop,      0,              "OOp Mail       [½Ô´A¯¸ªø]",0,
  mail_mbox,    PERM_INTERNET,  "ZZip           [¥´¥]¸ê®Æ]",0,
  mail_all,     PERM_SYSOP,     "AAll           [¨t²Î³q§i]",0,
NULL, 0, NULL,0};



/* ----------------------------------------------------- */
/* main menu                                             */
/* ----------------------------------------------------- */

static int
admin()
{
  domenu(ADMIN, "¯¸ªø¦Ñ¤j", 'X', adminlist);
  return 0;
}

static int
BOARD()
{
  domenu(CLASS, "¬ÝªO¦Cªí", 'G', classlist);
  return 0;
}

static int
Mail()
{
  domenu(MAIL, "¶l¥ó¿ï³æ", 'R', maillist);
  return 0;
}

int
static Talk()
{
  domenu(TMENU, "²á¤Ñ¿ï³æ", 'L', talklist);
  return 0;
}

static int
User()
{
  domenu(UMENU, "­Ó¤H³]©w", 'H', userlist);
  return 0;
}


static int
Service()
{
  domenu(PMENU, "¦UºØªA°È", 'H', servicelist);
  return 0;
}


int Announce(), Boards(), Goodbye(),Log(),board();


struct MENU cmdlist[] = {
  admin,        PERM_ADMIN,     "00Admin        [¨t²ÎºÞ²z]",0,
  Announce,     0,              "AAnnounce      [¤Ñ¦aºëµØ]",0,
  BOARD,        0,              "BBoard         [¬ÝªO¥\\¯à]",0,
  board,        0,              "CClass         [¤ÀÃþ¬ÝªO]",0,
  Mail,         PERM_BASIC,     "MMail          [­·¹Ð¶l§½]",0,
  Talk,         0,              "TTalk          [½Í¤Ñ»¡¦a]",0,
  User,         PERM_BASIC,     "UUser          [­Ó¤H¤u¨ã]",0,
  Log,          0,              "HHistory       [¾ú¥v­y¸ñ]",0,
  Service,      0,              "SService       [¦UºØªA°È]",0,
  Goodbye,      0,              "GGoodbye       [¦³½t¤d¨½]",0,
NULL, 0, NULL,0};
/* INDENT ON */
