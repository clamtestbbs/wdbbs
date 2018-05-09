/*-------------------------------------------------------*/
/* xchatd.c     ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : super KTV daemon for chat server             */
/* create : 95/03/29                                     */
/* update : 97/10/20                                     */
/*-------------------------------------------------------*/


#include "bbs.h"
#include "xchat.h"

#define _BBS_UTIL_C_
#include "cache.c"

#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

#define STAND_ALONE              /* 不搭配 BBS 獨立執行 */

#define SERVER_USAGE
#define WATCH_DOG
#define  MONITOR_                 /* 監督 chatroom 活動以解決糾紛 */
#define  DEBUG                   /* 程式除錯之用 */

#ifdef  DEBUG
#define MONITOR_
#endif

static int gline;

#ifdef  WATCH_DOG
#define MYDOG  gline = __LINE__
#else
#define MYDOG                   /* NOOP */
#endif



#define CHAT_PIDFILE    "log/chat.pid"
#define CHAT_LOGFILE    "log/chat.log"
#define CHAT_INTERVAL   (60 * 30)
#define SOCK_QLEN       1


/* name of the main room (always exists) */


#define MAIN_NAME       "main"
#define MAIN_TOPIC      "Pentium2 跑的快..."


#define ROOM_LOCKED     1
#define ROOM_SECRET     2
#define ROOM_OPENTOPIC  4
#define ROOM_HANDUP     8
#define ROOM_ALL        (NULL)


#define LOCKED(room)    (room->rflag & ROOM_LOCKED)
#define SECRET(room)    (room->rflag & ROOM_SECRET)
#define OPENTOPIC(room) (room->rflag & ROOM_OPENTOPIC)
#define RHANDUP(room)    (room->rflag & ROOM_HANDUP)

#define RESTRICTED(usr) (usr->uflag == 0)       /* guest */
#define CHATSYSOP(usr)  (usr->uflag & ( PERM_SYSOP | PERM_CHATROOM))
/* Thor: SYSOP 與 CHATROOM都是 chat總管 */
#define PERM_ROOMOP     PERM_CHAT	/* Thor: 借 PERM_CHAT為 PERM_ROOMOP */
#define PERM_HANDUP     PERM_BM		/* 借 PERM_BM 為有沒有舉手過 */
#define PERM_SAY        PERM_NOTOP	/* 借 PERM_NOTOP 為有沒有發表權 */

/* 進入時需清空              */
/* Thor: ROOMOP為房間管理員 */
#define ROOMOP(usr)  (usr->uflag & ( PERM_ROOMOP | PERM_SYSOP | PERM_CHATROOM))
#define CLOAK(usr)      (usr->uflag & PERM_CLOAK)
#define HANDUP(usr)  (usr->uflag & PERM_HANDUP) 
#define SAY(usr)      (usr->uflag & PERM_SAY)
 /* Thor: 聊天室隱身術 */


/* ----------------------------------------------------- */
/* ChatRoom data structure                               */
/* ----------------------------------------------------- */

typedef struct ChatRoom ChatRoom;
typedef struct ChatUser ChatUser;
typedef struct UserList UserList;
typedef struct ChatCmd ChatCmd;
typedef struct ChatAction ChatAction;

struct ChatUser
{
  struct ChatUser *unext;
  int sock;                     /* user socket */
  int talksock;                 /* talk socket */
  ChatRoom *room;
  UserList *ignore;
  int userno;
  int uflag;
  int clitype;                  /* Xshadow: client type. 1 for common client,
                                 * 0 for bbs only client */
  time_t uptime;                /* Thor: unused */
  char userid[IDLEN + 1];       /* real userid */
  char chatid[9];               /* chat id */
  char lasthost[30];            /* host address */
  char ibuf[80];                /* buffer for non-blocking receiving */
  int isize;                    /* current size of ibuf */
  int color;
};


struct ChatRoom
{
  struct ChatRoom *next, *prev;
  char name[IDLEN];
  char topic[48];               /* Let the room op to define room topic */
  int rflag;                    /* ROOM_LOCKED, ROOM_SECRET, ROOM_OPENTOPIC */
  int occupants;                /* number of users in room */
  UserList *invite;
};


struct UserList
{
  struct UserList *next;
  int userno;
  char userid[IDLEN + 1];
};


struct ChatCmd
{
  char *cmdstr;
  void (*cmdfunc) ();
  int exact;
};


static ChatRoom mainroom;
static ChatUser *mainuser;
static fd_set mainfds;
static int maxfds;              /* number of sockets to select on */
static int totaluser;           /* current number of connections */
static struct timeval zerotv;   /* timeval for selecting */
static char chatbuf[256];       /* general purpose buffer */
static int common_client_command;

#ifdef STAND_ALONE
static int userno_inc = 0;      /* userno auto-incrementer */
#endif

static char msg_not_op[] = "◆ 您不是這間聊天室的 Op";
static char msg_no_such_id[] = "◆ 目前沒有人使用 [%s] 這個聊天代號";
static char msg_not_here[] = "◆ [%s] 不在這間聊天室";


#define FUZZY_USER      ((ChatUser *) -1)


#ifndef STAND_ALONE
typedef struct userec ACCT;

/* ----------------------------------------------------- */
/* acct_load for check acct                              */
/* ----------------------------------------------------- */

int
acct_load(acct, userid)
  ACCT *acct;
  char *userid;
{
  rec_get(FN_PASSWD, acct, sizeof(ACCT), searchuser(userid));
}


/* ----------------------------------------------------- */
/* str_lower for check acct                              */
/* ----------------------------------------------------- */
void
str_lower(dst, src)
  char *dst, *src;
{
  register int ch;

  do
  {
    ch = *src++;
    if (ch >= 'A' && ch <= 'Z')
      ch |= 0x20;
    *dst++ = ch;
  } while (ch);
}

/*
 * str_ncpy() - similar to strncpy(3) but terminates string always with '\0'
 * if n != 0, and doesn't do padding
 */

void
str_ncpy(dst, src, n)
  char *dst;
  char *src;
  int n;
{
  char *end;

  end = dst + n;

  do
  {
    n = (dst == end) ? 0 : *src++;
    *dst++ = n;
  } while (n);
}


/* ----------------------------------------------------- */
/* usr_fpath for check acct                              */
/* ----------------------------------------------------- */
char *str_home_file = "home/%s/%s";

void
usr_fpath(buf, userid, fname)
  char *buf, *userid, *fname;
{
  sprintf(buf, str_home_file, userid, fname);
}

/* ----------------------------------------------------- */
/* chkpasswd for check passwd                            */
/* ----------------------------------------------------- */
char *crypt();
static char pwbuf[PASSLEN];

int
chkpasswd(passwd, test)
  char *passwd, *test;
{
  char *pw;

  str_ncpy(pwbuf, test, PASSLEN);
  pw = crypt(pwbuf, passwd);
  return (!strncmp(pw, passwd, PASSLEN));
}
#endif                          /* STAND_ALONE */


/* ----------------------------------------------------- */
/* operation log and debug information                   */
/* ----------------------------------------------------- */


static int flog;                /* log file descriptor */


static void
logit(key, msg)
  char *key;
  char *msg;
{
  time_t now;
  struct tm *p;
  char buf[512];

  time(&now);
  p = localtime(&now);
  sprintf(buf, "%02d/%02d %02d:%02d:%02d %-13s%s\n",
    p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, key, msg);
  write(flog, buf, strlen(buf));
}


static void
log_init()
{
  flog = open(CHAT_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logit("START", "chat daemon");
}


static void
log_close()
{
  close(flog);
}


#ifdef  DEBUG
static void
debug_user()
{
  register ChatUser *user;
  int i;
  char buf[80];

  i = 0;
  for (user = mainuser; user; user = user->unext)
  {
    sprintf(buf, "%d) %s %s", ++i, user->userid, user->chatid);
    logit("DEBUG_U", buf);
  }
}


static void
debug_room()
{
  register ChatRoom *room;
  int i;
  char buf[80];

  i = 0;
  room = &mainroom;

  do
  {
    sprintf(buf, "%d) %s %d", ++i, room->name, room->occupants);
    logit("DEBUG_R", buf);
  } while (room = room->next);
}
#endif                          /* DEBUG */


/* ----------------------------------------------------- */
/* string routines                                       */
/* ----------------------------------------------------- */


static int
valid_chatid(id)
  register char *id;
{
  register int ch, len;

  for (len = 0; ch = *id; id++)
  {                             /* Thor: check for endless */
    MYDOG;

    if (ch == '/' || ch == '*' || ch == ':')
      return 0;
    if (++len > 8)
      return 0;
  }
  return len;
}

#if 0
static int
Isspace(ch)
  int ch;
{
  return (ch == ' ' || ch == '\t' || ch == 10 || ch == 13);
}

static char *
nextword(str)
  char **str;
{
  char *head, *tail;
  int ch;

  head = *str;
  for (;;)
  {                             /* Thor: check for endless */
    MYDOG;

    ch = *head;
    if (!ch)
    {
      *str = head;
      return head;
    }
    if (!Isspace(ch))
      break;
    head++;
  }

  tail = head + 1;
  while (ch = *tail)
  {                             /* Thor: check for endless */
    MYDOG;

    if (Isspace(ch))
    {
      *tail++ = '\0';
      break;
    }
    tail++;
  }
  *str = tail;

  return head;
}
#endif

/* Case Independent strcmp : 1 ==> euqal */


static int
str_equal(s1, s2)
  register unsigned char *s1, *s2;      /* Thor: 加上 unsigned,
                                         * 避免中文的問題 */
{
  register int c1, c2;

  for (;;)
  {                             /* Thor: check for endless */
    MYDOG;

    c1 = *s1;
    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 32;

    c2 = *s2;
    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 32;

    if (c1 != c2)
      return 0;

    if (!c1)
      return 1;

    s1++;
    s2++;
  }
}


/* ----------------------------------------------------- */
/* match strings' similarity case-insensitively          */
/* ----------------------------------------------------- */
/* str_match(keyword, string)                            */
/* ----------------------------------------------------- */
/* 0 : equal            ("foo", "foo")                   */
/* -1 : mismatch        ("abc", "xyz")                   */
/* ow : similar         ("goo", "good")                  */
/* ----------------------------------------------------- */


static int
str_match(s1, s2)
  register unsigned char *s1, *s2;      /* Thor: 加上 unsigned,
                                         * 避免中文的問題 */
{
  register int c1, c2;

  for (;;)
  {                             /* Thor: check for endless */
    MYDOG;

    c2 = *s2;
    c1 = *s1;
    if (!c1)
    {
      return c2;
    }

    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 32;

    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 32;

    if (c1 != c2)
      return -1;

    s1++;
    s2++;
  }
}


/* ----------------------------------------------------- */
/* search user/room by its ID                            */
/* ----------------------------------------------------- */


static ChatUser *
cuser_by_userid(userid)
  char *userid;
{
  register ChatUser *cu;

  for (cu = mainuser; cu; cu = cu->unext)
  {
    MYDOG;

    if (str_equal(userid, cu->userid))
      break;
  }
  return cu;
}


static ChatUser *
cuser_by_chatid(chatid)
  char *chatid;
{
  register ChatUser *cu;

  for (cu = mainuser; cu; cu = cu->unext)
  {
    MYDOG;

    if (str_equal(chatid, cu->chatid))
      break;
  }
  return cu;
}


static ChatUser *
fuzzy_cuser_by_chatid(chatid)
  char *chatid;
{
  register ChatUser *cu, *xuser;
  int mode;

  xuser = NULL;

  for (cu = mainuser; cu; cu = cu->unext)
  {
    MYDOG;

    mode = str_match(chatid, cu->chatid);
    if (mode == 0)
      return cu;

    if (mode > 0)
    {
      if (xuser == NULL)
        xuser = cu;
      else
        return FUZZY_USER;      /* 符合者大於 2 人 */
    }
  }
  return xuser;
}


static ChatRoom *
croom_by_roomid(roomid)
  char *roomid;
{
  register ChatRoom *room;

  room = &mainroom;
  do
  {
    MYDOG;

    if (str_equal(roomid, room->name))
      break;
  } while (room = room->next);
  return room;
}


/* ----------------------------------------------------- */
/* UserList routines                                     */
/* ----------------------------------------------------- */


static void
list_free(list)
  UserList *list;
{
  UserList *tmp;

  while (list)
  {
    MYDOG;

    tmp = list->next;

    free(list);
    MYDOG;
    list = tmp;
  }
}


static void
list_add(list, user)
  UserList **list;
  ChatUser *user;
{
  UserList *node;

  MYDOG;

  if (node = (UserList *) malloc(sizeof(UserList)))
  {                             /* Thor: 防止空間不夠 */
    strcpy(node->userid, user->userid);
    node->userno = user->userno;
    node->next = *list;
    *list = node;
  }
  MYDOG;
}


static int
list_delete(list, userid)
  UserList **list;
  char *userid;
{
  UserList *node;

  while (node = *list)
  {
    MYDOG;

    if (str_equal(node->userid, userid))
    {
      *list = node->next;
      MYDOG;
      free(node);
      MYDOG;
      return 1;
    }
    list = &node->next;         /* Thor: list要跟著前進 */
  }

  return 0;
}


static int
list_belong(list, userno)
  UserList *list;
  int userno;
{
  while (list)
  {
    MYDOG;

    if (userno == list->userno)
      return 1;
    list = list->next;
  }
  return 0;
}


/* ------------------------------------------------------ */
/* non-blocking socket routines : send message to users   */
/* ------------------------------------------------------ */


static void
do_send(nfds, wset, msg, number)
  int nfds;
  fd_set *wset;
  char *msg;
  int number;
{
  int sr;

  /* Thor: for future reservation bug */
  zerotv.tv_sec = 0;
  zerotv.tv_usec = 0;

  MYDOG;

  sr = select(nfds + 1, NULL, wset, NULL, &zerotv);

  MYDOG;

  if (sr > 0)
  {
    register int len;

    len = strlen(msg) + 1;
    while (nfds >= 0)
    {
      MYDOG;

      if (FD_ISSET(nfds, wset))
      {
        MYDOG;
        send(nfds, msg, len, 0);/* Thor: 如果buffer滿了, 仍會 block */
        MYDOG;
        if (--sr <= 0)
          return;
      }
      nfds--;
    }
  }
}


static void
send_to_room(room, msg, userno, number)
  ChatRoom *room;
  char *msg;
  int userno;
  int number;
{
  ChatUser *cu;
  fd_set wset, *wptr;
  int sock, max;
  static char sendbuf[256];
  int clitype;                  /* 分為 bbs client 及 common client 兩次處理 */

  for (clitype = (number == MSG_MESSAGE || !number) ? 0 : 1; clitype < 2; clitype++)
  {

    FD_ZERO( (wptr = &wset) );
    max = -1;

    for (cu = mainuser; cu; cu = cu->unext)
    {
      MYDOG;

      if (room == cu->room || room == ROOM_ALL)
      {
        if (cu->clitype == clitype && (!userno || !list_belong(cu->ignore, userno)))
        {
          sock = cu->sock;
          FD_SET(sock, wptr);
          if (max < sock)
            max = sock;
        }
      }
    }

    if (max < 0)
      continue;

    if (clitype)
    {
      if (strlen(msg))
        sprintf(sendbuf, "%3d %s", number, msg);
      else
        sprintf(sendbuf, "%3d", number);

      do_send(max, wptr, sendbuf);
    }
    else
      do_send(max, wptr, msg);
  }
}


static void
send_to_user(user, msg, userno, number)
  ChatUser *user;
  char *msg;
  int userno;
  int number;
{
  if (!user->clitype && number && number != MSG_MESSAGE)
    return;

  if (!userno || !list_belong(user->ignore, userno))
  {
    fd_set wset, *wptr;
    int sock;
    static char sendbuf[256];

    sock = user->sock;
    FD_ZERO( (wptr = &wset) );
    FD_SET(sock, wptr);

    if (user->clitype)
    {
      if (strlen(msg))
        sprintf(sendbuf, "%3d %s", number, msg);
      else
        sprintf(sendbuf, "%3d", number);
      do_send(sock, wptr, sendbuf);
    }
    else
      do_send(sock, wptr, msg);
  }
}

#if 0
static void
send_to_sock(sock, msg)         /* Thor: unused */
  int sock;
  char *msg;
{
  fd_set wset, *wptr;

  FD_ZERO(wptr = &wset);
  FD_SET(sock, wptr);
  do_send(sock, wptr, msg);
}
#endif

/* ----------------------------------------------------- */

static void
room_changed(room)
  ChatRoom *room;
{
  if (!room)
    return;

  sprintf(chatbuf, "= %s %d %d %s", room->name, room->occupants, room->rflag, room->topic);
  send_to_room(ROOM_ALL, chatbuf, 0, MSG_ROOMNOTIFY);
}

static void
user_changed(cu)
  ChatUser *cu;
{
  if (!cu)
    return;

  sprintf(chatbuf, "= %s %s %d %s %s", cu->userid, cu->chatid, cu->color, cu->room->name, cu->lasthost);
  if (ROOMOP(cu))
    strcat(chatbuf, " Op");
  send_to_room(cu->room, chatbuf, 0, MSG_USERNOTIFY);
}

static void
exit_room(user, mode, msg)
  ChatUser *user;
  int mode;
  char *msg;
{
  ChatRoom *room;

  if (room = user->room)
  {
    user->room = NULL;
    user->uflag &= ~PERM_ROOMOP;

    if (--room->occupants > 0)
    {
      char *chatid;

      chatid = user->chatid;
      switch (mode)
      {
      case EXIT_LOGOUT:

        sprintf(chatbuf, "◆ %s 離開了 ...", chatid);
        if (msg && *msg)
        {
          strcat(chatbuf, ": ");
          msg[79] = 0;          /* Thor:防止太長 */
          strncat(chatbuf, msg, 80);
        }
        break;

      case EXIT_LOSTCONN:

        sprintf(chatbuf, "◆ %s 成了斷線的風箏囉", chatid);
        break;

      case EXIT_KICK:

        sprintf(chatbuf, "◆ 哈哈！%s 被踢出去了", chatid);
        break;
      }
      if (!CLOAK(user))         /* Thor: 聊天室隱身術 */
        send_to_room(room, chatbuf, 0, MSG_MESSAGE);

      sprintf(chatbuf, "- %s", user->userid);
      send_to_room(room, chatbuf, 0, MSG_USERNOTIFY);
      room_changed(room);

      return;
    }

    else if (room != &mainroom)
    {                           /* Thor: 人數為0時,不是mainroom才free */
      register ChatRoom *next;

#ifdef  DEBUG
      debug_room();
#endif

      sprintf(chatbuf, "- %s", room->name);
      send_to_room(ROOM_ALL, chatbuf, 0, MSG_ROOMNOTIFY);

      room->prev->next = room->next;
      if (next = room->next)
        next->prev = room->prev;
      list_free(room->invite);

      MYDOG;
      free(room);
      MYDOG;

#ifdef  DEBUG
      debug_room();
#endif
    }
  }
}


/* ----------------------------------------------------- */
/* chat commands                                         */
/* ----------------------------------------------------- */

#ifndef STAND_ALONE
/* ----------------------------------------------------- */
/* (.ACCT) 使用者帳號 (account) subroutines              */
/* ----------------------------------------------------- */

static char datemsg[32];

char *
Ctime(clock)
  time_t *clock;
{
  struct tm *t = localtime(clock);
  static char week[] = "日一二三四五六";

  sprintf(datemsg, "%d年%2d月%2d日%3d:%02d:%02d 星期%.2s",
    t->tm_year - 11, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec, &week[t->tm_wday << 1]);
  return (datemsg);
}

static void
chat_query(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char str[256];
  int i;
  ACCT xuser;
  FILE *fp;

  if (acct_load(&xuser, msg) >= 0)
  {
    sprintf(chatbuf, "%s(%s) 共上站 %d 次，文章 %d 篇",
      xuser.userid, xuser.username, xuser.numlogins, xuser.numposts);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);

    sprintf(chatbuf, "最近(%s)從(%s)上站", Ctime(&xuser.lastlogin),
      (xuser.lasthost[0] ? xuser.lasthost : "外太空"));
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);

    usr_fpath(chatbuf, xuser.userid, "plans");
    fp = fopen(chatbuf, "rt");
    i = 0;
    while (fp && fgets(str, 256, fp))
    {
      if (!strlen(str))
        continue;

      str[strlen(str) - 1] = 0;
      send_to_user(cu, str, 0, MSG_MESSAGE);
      if (++i >= MAXQUERYLINES)
        break;
    }
    fclose(fp);
  }
  else
  {
    sprintf(chatbuf, msg_no_such_id, msg);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
  }
}
#endif

static void
chat_clear(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (cu->clitype)
    send_to_user(cu, "", 0, MSG_CLRSCR);
  else
    send_to_user(cu, "/c", 0, MSG_MESSAGE);
}

static void
chat_date(cu, msg)
  ChatUser *cu;
  char *msg;
{
  time_t thetime;

  time(&thetime);
  sprintf(chatbuf, "◆ 標準時間: %s", Ctime(&thetime));
  send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
}


static void
chat_topic(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatRoom *room;
  char *topic;

  if (!ROOMOP(cu) && !OPENTOPIC(cu->room))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  if (*msg == '\0')
  {
    send_to_user(cu, "※ 請指定話題", 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;
  topic = room->topic;          /* Thor: room 有可能 NULL嗎?? */
  strncpy(topic, msg, 47);
  topic[47] = '\0';

  if (cu->clitype)
    send_to_room(room, topic, 0, MSG_TOPIC);
  else
  {
    sprintf(chatbuf, "/t%s", topic);
    send_to_room(room, chatbuf, 0, 0);
  }

  room_changed(room);

  sprintf(chatbuf, "◆ %s 將話題改為 \x1b[1;32m%s\x1b[m", cu->chatid, topic);
  if (!CLOAK(cu))               /* Thor: 聊天室隱身術 */
    send_to_room(room, chatbuf, 0, MSG_MESSAGE);
}


static void
chat_version(cu, msg)
  ChatUser *cu;
  char *msg;
{
  sprintf(chatbuf, "%d %d", XCHAT_VERSION_MAJOR, XCHAT_VERSION_MINOR);
  send_to_user(cu, chatbuf, 0, MSG_VERSION);
}

static void
chat_nick(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *chatid, *str;
  ChatUser *xuser;

  chatid = nextword(&msg);
  chatid[8] = '\0';
  if (!valid_chatid(chatid))
  {
    send_to_user(cu, "※ 這個聊天代號是不正確的", 0, MSG_MESSAGE);
    return;
  }

  xuser = cuser_by_chatid(chatid);
  if (xuser != NULL && xuser != cu)
  {
    send_to_user(cu, "※ 已經有人捷足先登囉", 0, MSG_MESSAGE);
    return;
  }

  str = cu->chatid;

  sprintf(chatbuf, "※ %s 將聊天代號改為 \x1b[1;33m%s\x1b[m", str, chatid);
  if (!CLOAK(cu))               /* Thor: 聊天室隱身術 */
    send_to_room(cu->room, chatbuf, cu->userno, MSG_MESSAGE);

  strcpy(str, chatid);

  user_changed(cu);

  if (cu->clitype)
    send_to_user(cu, chatid, 0, MSG_NICK);
  else
  {
    sprintf(chatbuf, "/n%s", chatid);
    send_to_user(cu, chatbuf, 0, 0);
  }
}

static void
chat_color(cu, msg)
  ChatUser *cu;
  char *msg;
{
  int color;

  color = atoi(nextword(&msg));
  if (color >= 8 || color < 1 || color == 4)
  {
    send_to_user(cu, "※ 請輸入 1~7 的數字 , 除了 4 之外", 0, MSG_MESSAGE);
    return;
  }

  sprintf(chatbuf, "※ %s 更改顏色為 \x1b[1;3%dm這樣\x1b[m", cu->chatid, color);
  if (!CLOAK(cu))               /* Thor: 聊天室隱身術 */
    send_to_room(cu->room, chatbuf, cu->userno, MSG_MESSAGE);

  cu->color = color;
}

static void
chat_list_rooms(cuser, msg)
  ChatUser *cuser;
  char *msg;
{
  ChatRoom *cr, *room;

  if (RESTRICTED(cuser))
  {
    send_to_user(cuser, "※ 您沒有權限列出現有的聊天室", 0, MSG_MESSAGE);
    return;
  }

  if (common_client_command)
    send_to_user(cuser, "", 0, MSG_ROOMLISTSTART);
  else
    send_to_user(cuser, "\x1b[7m 談天室名稱  │人數│話題        \x1b[m", 0, MSG_MESSAGE);

  room = cuser->room;
  cr = &mainroom;
  do
  {
    MYDOG;


    if (!SECRET(cr) || CHATSYSOP(cuser) || (cr == room && ROOMOP(cuser)))
    {
      if (common_client_command)
      {
        sprintf(chatbuf, "%s %d %d %s", cr->name, cr->occupants, cr->rflag, cr->topic);
        send_to_user(cuser, chatbuf, 0, MSG_ROOMLIST);
      }
      else
      {
        sprintf(chatbuf, " %-12s│%4d│%s", cr->name, cr->occupants, cr->topic);
        if (LOCKED(cr))
          strcat(chatbuf, " [鎖住]");
        if (SECRET(cr))
          strcat(chatbuf, " [秘密]");
        if (OPENTOPIC(cr))
          strcat(chatbuf, " [話題]");
        send_to_user(cuser, chatbuf, 0, MSG_MESSAGE);
      }

    }
  } while (cr = cr->next);

  if (common_client_command)
    send_to_user(cuser, "", 0, MSG_ROOMLISTEND);
}


static void
chat_do_user_list(cu, msg, theroom)
  ChatUser *cu;
  char *msg;
  ChatRoom *theroom;
{
  ChatRoom *myroom, *room;
  ChatUser *user;

  int start, stop, curr;
  start = atoi(nextword(&msg));
  stop = atoi(nextword(&msg));

  myroom = cu->room;

#ifdef DEBUG
  logit(cu->chatid, "do user list");
#endif

  if (common_client_command)
    send_to_user(cu, "", 0, MSG_USERLISTSTART);
  else
    send_to_user(cu, "\x1b[7m 聊天代號│使用者代號  │聊天室 \x1b[m", 0, MSG_MESSAGE);

  for (user = mainuser; user; user = user->unext)
  {
    MYDOG;

#ifdef DEBUG
    logit(cu->chatid, "list in for");
#endif

    room = user->room;
    if ((theroom != ROOM_ALL) && (theroom != room))
      continue;

    if (myroom != room)
    {
      if (RESTRICTED(cu) ||
        (room && SECRET(room) && !CHATSYSOP(cu)))
        continue;
    }

    if (CLOAK(user))            /* Thor: 隱身術 */
      continue;

#ifdef DEBUG
    logit(cu->chatid, "list in for 2");
#endif

    curr++;
    if (start && curr < start)
      continue;
    else if (stop && (curr > stop))
      break;

    if (common_client_command)
    {
      if (!room)
        continue;               /* Xshadow: 還沒進入任何房間的就不列出 */

      sprintf(chatbuf, "%s %s %s %s", user->chatid, user->userid, room->name, user->lasthost);
      if (ROOMOP(user))
        strcat(chatbuf, " Op");
    }
    else
    {
      sprintf(chatbuf, " %-8s│%-12s│%s", user->chatid, user->userid, room ? room->name : "[在門口徘徊]");
      if (ROOMOP(user))
        strcat(chatbuf, " [Op]");
    }

#ifdef  DEBUG
    logit("list_U", chatbuf);
#endif

    send_to_user(cu, chatbuf, 0, common_client_command ? MSG_USERLIST : MSG_MESSAGE);
  }
  if (common_client_command)
    send_to_user(cu, "", 0, MSG_USERLISTEND);
}

static void
chat_list_by_room(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatRoom *whichroom;
  char *roomstr;

  roomstr = nextword(&msg);
  if (*roomstr == '\0')
    whichroom = cu->room;
  else
  {
    if ((whichroom = croom_by_roomid(roomstr)) == NULL)
    {
      sprintf(chatbuf, "※ 沒有 [%s] 這個聊天室", roomstr);
      send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
      return;
    }

    if (whichroom != cu->room && SECRET(whichroom) && !CHATSYSOP(cu))
    {                           /* Thor: 要不要測同一room雖SECRET但可以列?
                                 * Xshadow: 我改成同一 room 就可以列 */
      send_to_user(cu, "※ 無法列出在秘密聊天室的使用者", 0, MSG_MESSAGE);
      return;
    }
  }
  chat_do_user_list(cu, msg, whichroom);
}


static void
chat_list_users(cu, msg)
  ChatUser *cu;
  char *msg;
{
  chat_do_user_list(cu, msg, ROOM_ALL);
}

static void
chat_chatroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (common_client_command)
    send_to_user(cu, "風塵茶樓 4 21", 0, MSG_CHATROOM);
}

static void
chat_map_chatids(cu, whichroom)
  ChatUser *cu;                 /* Thor: 還沒有作不同間的 */
  ChatRoom *whichroom;
{
  int c;
  ChatRoom *myroom, *room;
  ChatUser *user;

  /* myroom = cu->room; */
  myroom = whichroom;
  send_to_user(cu,
    "\x1b[7m 聊天代號 使用者代號  │ 聊天代號 使用者代號  │ 聊天代號 使用者代號 \x1b[m", 0, MSG_MESSAGE);

  c = 0;

  for (user = mainuser; user; user = user->unext)
  {
    MYDOG;

    room = user->room;
    MYDOG;
    if (whichroom != ROOM_ALL && whichroom != room)
      continue;
    MYDOG;
    if (myroom != room)
    {
      if (RESTRICTED(cu) ||     /* Thor: 要先check room 是不是空的 */
        (room && SECRET(room) && !CHATSYSOP(cu)))
        continue;
    }
    MYDOG;
    if (CLOAK(user))            /* Thor:隱身術 */
      continue;
    sprintf(chatbuf + (c * 24), " %-8s%c%-12s%s",
      user->chatid, ROOMOP(user) ? '*' : ' ',
      user->userid, (c < 2 ? "│" : "  "));
    MYDOG;
    if (++c == 3)
    {
      send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
      c = 0;
    }
    MYDOG;
  }
  if (c > 0)
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
}


static void
chat_map_chatids_thisroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  chat_map_chatids(cu, cu->room);
}


static void
chat_setroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *modestr;
  ChatRoom *room;
  char *chatid;
  int sign;
  int flag;
  char *fstr;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  modestr = nextword(&msg);
  sign = 1;
  if (*modestr == '+')
    modestr++;
  else if (*modestr == '-')
  {
    modestr++;
    sign = 0;
  }
  if (*modestr == '\0')
  {
    send_to_user(cu,
      "※ 請指定狀態: {[+(設定)][-(取消)]}{[l(鎖住)][s(秘密)][t(開放話題)}", 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;
  chatid = cu->chatid;

  while (*modestr)
  {
    flag = 0;
    switch (*modestr)
    {
    case 'l':
    case 'L':
      flag = ROOM_LOCKED;
      fstr = "鎖住";
      break;

    case 's':
    case 'S':
      flag = ROOM_SECRET;
      fstr = "秘密";
      break;

    case 't':
    case 'T':
      flag = ROOM_OPENTOPIC;
      fstr = "開放話題";
      break;
    case 'h':
    case 'H':
      flag = ROOM_OPENTOPIC;
      fstr = "舉手發言";
      break;

    default:
      sprintf(chatbuf, "※ 狀態錯誤：[%c]", *modestr);
      send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
    }

    /* Thor: check room 是不是空的, 應該不是空的 */
    if (flag && (room->rflag & flag) != sign * flag)
    {
      room->rflag ^= flag;
      sprintf(chatbuf, "※ 本聊天室被 %s %s [%s] 狀態",
        chatid, sign ? "設定為" : "取消", fstr);
      if (!CLOAK(cu))           /* Thor: 聊天室隱身術 */
        send_to_room(room, chatbuf, 0, MSG_MESSAGE);
    }
    modestr++;
  }
  room_changed(room);
}

static char *chat_msg[] =
{
  "[//]help", "MUD-like 社交動詞",
  "[/h]elp op", "談天室管理員專用指令",
  "[/a]ct <msg>", "做一個動作",
  "[/b]ye [msg]", "道別",
  "[/c]lear  [/d]ate", "清除螢幕  目前時間",
  "[/co]lor", "改變顏色",

#if 0
  "[/f]ire <user> <msg>", "發送熱訊",   /* Thor.0727: 和 flag 衝key */
#endif

  "[/i]gnore [user]", "忽略使用者",
  "[/j]oin <room>", "建立或加入談天室",
  "[/l]ist [start [stop]]", "列出談天室使用者",
  "[/m]sg <id|user> <msg>", "跟 <id> 說悄悄話",
  "[/n]ick <id>", "將談天代號換成 <id>",
  "[/p]ager", "切換呼叫器",
  "[/q]uery <user>", "查詢網友",
  "[/r]oom  [/t]ape", "列出一般談天室  開關錄音機",
  "[/u]nignore <user>", "取消忽略",

#if 0
  "[/u]sers", "列出站上使用者",
#endif

  "[/w]ho", "列出本談天室使用者",
  "[/w]hoin <room>", "列出談天室<room> 的使用者",
  NULL
};


static char *room_msg[] =
{
  "[/f]lag [+-][lsth]", "設定鎖定、秘密、開放話題、舉手發言",
  "[/i]nvite <id>", "邀請 <id> 加入談天室",
  "[/kick] <id>", "將 <id> 踢出談天室",
  "[/o]p <id>", "將 Op 的權力轉移給 <id>",
  "[/topic] <text>", "換個話題",
  "[/w]all", "廣播 (站長專用)",
  NULL
};


static void
chat_help(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char **table, *str;

  if (str_equal(nextword(&msg), "op"))
  {
    send_to_user(cu, "談天室管理員專用指令", 0, MSG_MESSAGE);
    table = room_msg;
  }
  else
  {
    table = chat_msg;
  }

  while (str = *table++)
  {
    sprintf(chatbuf, "  %-20s- %s", str, *table++);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
  }
}


static void
chat_private(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *recipient;
  ChatUser *xuser;
  int userno;

  userno = 0;
  recipient = nextword(&msg);
  xuser = (ChatUser *) fuzzy_cuser_by_chatid(recipient);
  if (xuser == NULL)
  {                             /* Thor.0724: 用 userid也可傳悄悄話 */
    xuser = cuser_by_userid(recipient);
  }
  if (xuser == NULL)
  {
    sprintf(chatbuf, msg_no_such_id, recipient);
  }
  else if (xuser == FUZZY_USER)
  {                             /* ambiguous */
    strcpy(chatbuf, "※ 請指明聊天代號");
  }
  else if (*msg)
  {
    userno = cu->userno;
    sprintf(chatbuf, "\x1b[1m*%s*\x1b[m ", cu->chatid);
    msg[79] = 0;                /* Thor:防止太長 */
    strncat(chatbuf, msg, 80);
    send_to_user(xuser, chatbuf, userno, MSG_MESSAGE);

    if (xuser->clitype)
    {                           /* Xshadow: 如果對方是用 client 上來的 */
      sprintf(chatbuf, "%s %s ", cu->userid, cu->chatid);
      msg[79] = 0;
      strncat(chatbuf, msg, 80);
      send_to_user(xuser, chatbuf, userno, MSG_PRIVMSG);
    }
    if (cu->clitype)
    {
      sprintf(chatbuf, "%s %s ", xuser->userid, xuser->chatid);
      msg[79] = 0;
      strncat(chatbuf, msg, 80);
      send_to_user(cu, chatbuf, 0, MSG_MYPRIVMSG);
    }

    sprintf(chatbuf, "%s> ", xuser->chatid);
    strncat(chatbuf, msg, 80);
  }
  else
  {
    sprintf(chatbuf, "※ 您想對 %s 說什麼話呢？", xuser->chatid);
  }
  send_to_user(cu, chatbuf, userno, MSG_MESSAGE);       /* Thor: userno 要改成 0
                                                         * 嗎? */
}


static void
chat_cloak(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (CHATSYSOP(cu))
  {
    cu->uflag ^= PERM_CLOAK;
    sprintf(chatbuf, "◆ %s", CLOAK(cu) ? MSG_CLOAKED : MSG_UNCLOAK);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
  }
}



/* ----------------------------------------------------- */


static void
arrive_room(cuser, room)
  ChatUser *cuser;
  ChatRoom *room;
{
  char *rname;

  /* Xshadow: 不必送給自己, 反正換房間就會重新 build user list */
  sprintf(chatbuf, "+ %s %s %s %s", cuser->userid, cuser->chatid, room->name, cuser->lasthost);
  if (ROOMOP(cuser))
    strcat(chatbuf, " Op");
  send_to_room(room, chatbuf, 0, MSG_USERNOTIFY);

  cuser->room = room;
  room->occupants++;
  rname = room->name;

  room_changed(room);

  if (cuser->clitype)
  {
    send_to_user(cuser, rname, 0, MSG_ROOM);
    send_to_user(cuser, room->topic, 0, MSG_TOPIC);
  }
  else
  {
    sprintf(chatbuf, "/r%s", rname);
    send_to_user(cuser, chatbuf, 0, 0);
    sprintf(chatbuf, "/t%s", room->topic);
    send_to_user(cuser, chatbuf, 0, 0);
  }

  sprintf(chatbuf, "※ \x1b[32;1m%s\x1b[m 進入 \x1b[33;1m[%s]\x1b[m 包廂",
    cuser->chatid, rname);
  if (!CLOAK(cuser))            /* Thor: 聊天室隱身術 */
    send_to_room(room, chatbuf, cuser->userno, MSG_MESSAGE);
}


static int
enter_room(cuser, rname, msg)
  ChatUser *cuser;
  char *rname;
  char *msg;
{
  ChatRoom *room;
  int create;

  create = 0;
  room = croom_by_roomid(rname);
  if (room == NULL)
  {
    /* new room */

#ifdef  MONITOR_
    logit(cuser->userid, "create new room");
#endif

    MYDOG;

    room = (ChatRoom *) malloc(sizeof(ChatRoom));
    MYDOG;
    if (room == NULL)
    {
      send_to_user(cuser, "※ 無法再新闢包廂了", 0, MSG_MESSAGE);
      return 0;
    }

    memset(room, 0, sizeof(ChatRoom));
    memcpy(room->name, rname, IDLEN - 1);
    strcpy(room->topic, "這是一個新天地");

    sprintf(chatbuf, "+ %s 1 0 %s", room->name, room->topic);
    send_to_room(ROOM_ALL, chatbuf, 0, MSG_ROOMNOTIFY);

    if (mainroom.next != NULL)
      mainroom.next->prev = room;
    room->next = mainroom.next;
    mainroom.next = room;
    room->prev = &mainroom;

    create = 1;
  }
  else
  {
    if (cuser->room == room)
    {
      sprintf(chatbuf, "※ 您本來就在 [%s] 聊天室囉 :)", rname);
      send_to_user(cuser, chatbuf, 0, MSG_MESSAGE);
      return 0;
    }

    if (!CHATSYSOP(cuser) && LOCKED(room) && !list_belong(room->invite, cuser->userno))
    {
      send_to_user(cuser, "※ 內有惡犬，非請莫入", 0, MSG_MESSAGE);
      return 0;
    }
  }

  exit_room(cuser, EXIT_LOGOUT, msg);
  arrive_room(cuser, room);

  if (create)
    cuser->uflag |= PERM_ROOMOP;

  return 0;
}


static void
logout_user(cuser)
  ChatUser *cuser;
{
  int sock;
  ChatUser *xuser, *prev;

#ifdef  DEBUG
  logit("before", "logout");
  debug_user();
#endif

  sock = cuser->sock;
  shutdown(sock, 2);
  close(sock);

  MYDOG;

  FD_CLR(sock, &mainfds);

#if 0   /* Thor: 也許不差這一個 */
   if (sock >= maxfds)
     maxfds = sock - 1;
#endif

  list_free(cuser->ignore);

#ifdef DEBUG
  debug_user();
#endif

  xuser = mainuser;
  if (xuser == cuser)
  {
    mainuser = cuser->unext;
  }
  else
  {
    do
    {
      prev = xuser;
      xuser = xuser->unext;
      if (xuser == cuser)
      {
        prev->unext = cuser->unext;
        break;
      }
    } while (xuser);
  }

  MYDOG;

#ifdef DEBUG
  sprintf(chatbuf, "%p", cuser);
  logit("free cuser", chatbuf);
#endif

  free(cuser);

#ifdef  DEBUG
  logit("after", "logout");
  debug_user();
#endif

#if 0
  next = cuser->next;
  prev = cuser->prev;
  prev->next = next;
  if (next)
    next->prev = prev;

  if (cuser)
    free(cuser);
  MYDOG;

#endif

  totaluser--;
}


static void
print_user_counts(cuser)
  ChatUser *cuser;
{
  ChatRoom *room;
  int num, userc, suserc, roomc, number;

  userc = suserc = roomc = 0;

  room = &mainroom;
  do
  {
    MYDOG;

    num = room->occupants;
    if (SECRET(room))
    {
      suserc += num;
      if (CHATSYSOP(cuser))
        roomc++;
    }
    else
    {
      userc += num;
      roomc++;
    }
  } while (room = room->next);

  number = (cuser->clitype) ? MSG_MOTD : MSG_MESSAGE;

  sprintf(chatbuf,
    "⊙ 歡迎光臨【風塵茶樓】，目前開了 \x1b[1;31m%d\x1b[m 間包廂", roomc);
  send_to_user(cuser, chatbuf, 0, number);

  sprintf(chatbuf, "⊙ 共有 \x1b[1;36m%d\x1b[m 人來擺\龍門陣", userc);
  if (suserc)
    sprintf(chatbuf + strlen(chatbuf), " [%d 人在秘密聊天室]", suserc);
  send_to_user(cuser, chatbuf, 0, number);
}


static int
login_user(cu, msg)
  ChatUser *cu;
  char *msg;
{
  int utent;

  int level;
  char *userid;
  char *chatid,*passwd,*room;
  struct sockaddr_in from;
  int fromlen;
  struct hostent *hp;

#ifndef STAND_ALONE
  char buf[20];
  ACCT acct;
#endif

  /*
   * Thor.0819: SECURED_CHATROOM : /! userid chatid passwd , userno
   * el 在check完passwd後取得
   */
  /* Xshadow.0915: common client support : /-! userid chatid password */

  /* 傳參數：userlevel, userid, chatid */

  /* client/server 版本依據 userid 抓 .PASSWDS 判斷 userlevel */

  userid = nextword(&msg);
  chatid = nextword(&msg);
  passwd = nextword(&msg);

#ifdef  DEBUG
  logit("ENTER", userid);
#endif

#ifndef STAND_ALONE
  /* Thor.0730: parse space before passwd */

  /* Thor.0813: 跳過一空格即可, 因為反正如果chatid有空格, 密碼也不對 */
  /* 就算密碼對, 也不會怎麼樣:p */
  /* 可是如果密碼第一個字是空格, 那跳太多空格會進不來... */
  if (*passwd == ' ')
    passwd++;

  /* Thor.0729: load acct */
  if (!*userid || (acct_load(&acct, userid) < 0))
  {

#ifdef  DEBUG
    logit("noexist", chatid);
#endif

    if (cu->clitype)
      send_to_user(cu, "錯誤的使用者代號", 0, ERR_LOGIN_NOSUCHUSER);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, 0);

    return -1;
  }
  else if(strncmp(passwd, acct.passwd, PASSLEN) &&
          !chkpasswd(acct.passwd, passwd))
  {

#ifdef  DEBUG
    logit("fake", chatid);
#endif

    if (cu->clitype)
      send_to_user(cu, "密碼錯誤", 0, ERR_LOGIN_PASSERROR);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, 0);
    return -1;
  }
  else
  {
    /* Thor.0729: if ok, read level.  */
    level = acct.userlevel;
    /* Thor.0819: read userno for client/server bbs */
    utent = searchuser(acct.userid);
  }
#else                           /* STAND_ALONE */
  {
    userec muser;
    int uid;

    nextword(&msg);
    room = nextword(&msg);
    uid = atoi(nextword(&msg));
    rec_get(BBSHOME"/.PASSWDS", &muser, sizeof(muser), uid);
    level = muser.userlevel;
    utent = ++userno_inc;
  }
#endif                          /* STAND_ALONE */

  /* Thor.0819: for client/server bbs */
/*
  for (xuser = mainuser; xuser; xuser = xuser->unext)
  {
    MYDOG;

    if (xuser->userno == utent)
    {

#ifdef  DEBUG
      logit("enter", "bogus");
#endif
      if (cu->clitype)
        send_to_user(cu, "請勿派遣分身進入聊天室 !!", 0, ERR_LOGIN_USERONLINE);
      else
        send_to_user(cu, CHAT_LOGIN_BOGUS, 0, 0);
      return -1;    
    }
  }
*/
  if (!valid_chatid(chatid))
  {

#ifdef  DEBUG
    logit("enter", chatid);
#endif

    if (cu->clitype)
      send_to_user(cu, "不合法的聊天室代號 !!", 0, ERR_LOGIN_NICKERROR);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, 0);
    return 0;
  }

#ifdef  DEBUG
  debug_user();
#endif

  if (cuser_by_chatid(chatid) != NULL)
  {
    /* chatid in use */

#ifdef  DEBUG
    logit("enter", "duplicate");
#endif

    if (cu->clitype)
      send_to_user(cu, "這個代號已經有人使用", 0, ERR_LOGIN_NICKINUSE);
    else
      send_to_user(cu, CHAT_LOGIN_EXISTS, 0, 0);
    return 0;
  }

  cu->userno = utent;
  cu->uflag = level & ~(PERM_ROOMOP | PERM_CLOAK | PERM_HANDUP | PERM_SAY);
  strcpy(cu->userid, userid);
  memcpy(cu->chatid, chatid, 8);
  cu->chatid[8] = '\0';

  /* Xshadow: 取得 client 的來源 */
  fromlen = sizeof(from);
  if (!getpeername(cu->sock, (struct sockaddr *) & from, &fromlen))
  {
    if ((hp = gethostbyaddr((char *) &from.sin_addr, sizeof(struct in_addr), from.sin_family)))
    {
      strcpy(cu->lasthost, hp->h_name);
    }
    else
      strcpy(cu->lasthost, (char *) inet_ntoa(from.sin_addr));

  }
  else
  {
    strcpy(cu->lasthost, "[外太空]");
  }

  if (cu->clitype)
    send_to_user(cu, "順利", 0, MSG_LOGINOK);
  else
    send_to_user(cu, CHAT_LOGIN_OK, 0, 0);
  if(strcmp(room,MAIN_NAME))
    enter_room(cu, room, (char *)NULL);
  else
    arrive_room(cu, &mainroom);

  send_to_user(cu, "", 0, MSG_MOTDSTART);
  print_user_counts(cu);
  send_to_user(cu, "", 0, MSG_MOTDEND);

#ifdef  DEBUG
  logit("enter", "OK");
#endif
  while(cu->color == 0 || cu->color == 4)
    cu->color = rand()%8;
  return 0;
}


static void
chat_act(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (*msg && (!RHANDUP(cu->room) || SAY(cu) || ROOMOP(cu)))
  {
    sprintf(chatbuf, "%s \x1b[36m%s\x1b[m", cu->chatid, msg);
    send_to_room(cu->room, chatbuf, cu->userno, MSG_MESSAGE);
  }
}


static void
chat_ignore(cu, msg)
  ChatUser *cu;
  char *msg;
{

  if (RESTRICTED(cu))
  {
    strcpy(chatbuf, "※ 您沒有 ignore 別人的權利");
  }
  else
  {
    char *ignoree;

    ignoree = nextword(&msg);
    if (*ignoree)
    {
      ChatUser *xuser;

      xuser = cuser_by_userid(ignoree);

      if (xuser == NULL)
      {

        sprintf(chatbuf, msg_no_such_id, ignoree);

#if 0
        sprintf(chatbuf, "◆ 談天室現在沒有 [%s] 這號人物", ignoree);
#endif
      }
      else if (xuser == cu || CHATSYSOP(xuser) ||
        (ROOMOP(xuser) && (xuser->room == cu->room)))
      {
        sprintf(chatbuf, "◆ 不可以 ignore [%s]", ignoree);
      }
      else
      {

        if (list_belong(cu->ignore, xuser->userno))
        {
          sprintf(chatbuf, "※ %s 已經被凍結了", xuser->chatid);
        }
        else
        {
          list_add(&(cu->ignore), xuser);
          sprintf(chatbuf, "◆ 將 [%s] 打入冷宮了 :p", xuser->chatid);
        }
      }
    }
    else
    {
      UserList *list;

      if (list = cu->ignore)
      {
        int len;
        char buf[16];

        send_to_user(cu, "◆ 這些人被打入冷宮了：", 0, MSG_MESSAGE);
        len = 0;
        do
        {
          sprintf(buf, "%-13s", list->userid);
          strcpy(chatbuf + len, buf);
          len += 13;
          if (len >= 78)
          {
            send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
            len = 0;
          }
        } while (list = list->next);

        if (len == 0)
          return;
      }
      else
      {
        strcpy(chatbuf, "◆ 您目前並沒有 ignore 任何人");
      }
    }
  }

  send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
}


static void
chat_unignore(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *ignoree;

  ignoree = nextword(&msg);

  if (*ignoree)
  {
    sprintf(chatbuf, (list_delete(&(cu->ignore), ignoree)) ?
      "◆ [%s] 不再被你冷落了" :
      "◆ 您並未 ignore [%s] 這號人物", ignoree);
  }
  else
  {
    strcpy(chatbuf, "◆ 請指明 user ID");
  }
  send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
}


void
chat_join(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (RESTRICTED(cu))
  {
    send_to_user(cu, "※ 您沒有加入其他聊天室的權限", 0, MSG_MESSAGE);
  }
  else
  {
    char *roomid = nextword(&msg);

    if (*roomid)
      enter_room(cu, roomid, msg);
    else
      send_to_user(cu, "※ 請指定聊天室的名字", 0, MSG_MESSAGE);
  }
}


static void
chat_kick(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *twit;
  ChatUser *xuser;
  ChatRoom *room;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  twit = nextword(&msg);
  xuser = cuser_by_chatid(twit);

  if (xuser == NULL)
  {
    sprintf(chatbuf, msg_no_such_id, twit);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;
  if (room != xuser->room || CLOAK(xuser))
  {                             /* Thor: 聊天室隱身術 */
    sprintf(chatbuf, msg_not_here, twit);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
    return;
  }

  if (CHATSYSOP(xuser))
  {                             /* Thor: 踢不走 CHATSYSOP */
    sprintf(chatbuf, "◆ 不可以 kick [%s]", twit);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
    return;
  }

  exit_room(xuser, EXIT_KICK, (char *) NULL);

  if (room == &mainroom)
    logout_user(xuser);
  else
    enter_room(xuser, MAIN_NAME, (char *) NULL);
}


static void
chat_makeop(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *newop;
  ChatUser *xuser;
  ChatRoom *room;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  newop = nextword(&msg);
  xuser = cuser_by_chatid(newop);

  if (xuser == NULL)
  {
    sprintf(chatbuf, msg_no_such_id, newop);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
    return;
  }

  if (cu == xuser)
  {
    sprintf(chatbuf, "※ 您早就已經是 Op 了啊");
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;

  if (room != xuser->room || CLOAK(xuser))
  {                             /* Thor: 聊天室隱身術 */
    sprintf(chatbuf, msg_not_here, xuser->chatid);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
    return;
  }

  cu->uflag &= ~PERM_ROOMOP;
  xuser->uflag |= PERM_ROOMOP;

  user_changed(cu);
  user_changed(xuser);

  sprintf(chatbuf, "※ %s 將 Op 權力轉移給 %s",
    cu->chatid, xuser->chatid);
  if (!CLOAK(cu))               /* Thor: 聊天室隱身術 */
    send_to_room(room, chatbuf, 0, MSG_MESSAGE, MSG_MESSAGE);
}



static void
chat_invite(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *invitee;
  ChatUser *xuser;
  ChatRoom *room;
  UserList **list;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  invitee = nextword(&msg);
  xuser = cuser_by_chatid(invitee);
  if (xuser == NULL)
  {
    sprintf(chatbuf, msg_no_such_id, invitee);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;              /* Thor: 是否要 check room 是否 NULL ? */
  list = &(room->invite);

  if (list_belong(*list, xuser->userno))
  {
    sprintf(chatbuf, "※ %s 已經接受過邀請了", xuser->chatid);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
    return;
  }
  list_add(list, xuser);

  sprintf(chatbuf, "※ %s 邀請您到 [%s] 聊天室",
    cu->chatid, room->name);
  send_to_user(xuser, chatbuf, 0, MSG_MESSAGE); /* Thor: 要不要可以 ignore? */
  sprintf(chatbuf, "※ %s 收到您的邀請了", xuser->chatid);
  send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
}


static void
chat_broadcast(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (!CHATSYSOP(cu))
  {
    send_to_user(cu, "※ 您沒有在聊天室廣播的權力!", 0, MSG_MESSAGE);
    return;
  }
  if (*msg == '\0')
  {
    send_to_user(cu, "※ 請指定廣播內容", 0, MSG_MESSAGE);
    return;
  }
  sprintf(chatbuf, "\x1b[1m※ " BOARDNAME "談天室廣播中 [%s].....\x1b[m",
    cu->chatid);
  send_to_room(ROOM_ALL, chatbuf, 0, MSG_MESSAGE);
  sprintf(chatbuf, "◆ %s", msg);
  send_to_room(ROOM_ALL, chatbuf, 0, MSG_MESSAGE);
}


static void
chat_goodbye(cu, msg)
  ChatUser *cu;
  char *msg;
{
  exit_room(cu, EXIT_LOGOUT, msg);
  /* Thor: 要不要加 logout_user(cu) ? */
}


/* --------------------------------------------- */
/* MUD-like social commands : action             */
/* --------------------------------------------- */

struct ChatAction
{
  char *verb;                   /* 動詞 */
  char *chinese;                /* 中文翻譯 */
  char *part1_msg;              /* 介詞 */
  char *part2_msg;              /* 動作 */
};


static ChatAction party_data[] =
{
  {
    "aluba", "阿魯巴", "把", "架上柱子阿魯巴!!"
  },
  {
    "bearhug", "緊抱", "用力把", "抱的快要窒息了"
  },
  {
    "bearnod", "用力點頭","對著", "拼命的點頭如搗蒜"
  },
  {
    "blade", "一刀", "一刀啟程把", "送上西天"
  },                            /* Thor.0729:觀眾要求 */
  {
    "bless", "祝福", "深深的祝福", "一切順利"
  },
  {
    "board", "主機板", "把", "抓去跪主機板"
  },                            /* Thor.0730: 觀眾要求 */
  {
    "bow", "鞠躬", "畢躬畢敬的對", "鞠躬"
  },
  {
    "boy", "平底鍋", "從背後拿出了平底鍋，把", "敲昏了"
  },
  {
    "bye", "掰掰", "向", "說掰掰!!"
  },
  {
    "call", "呼喚", "大聲的呼喚,啊~~", "啊~~~你在哪裡啊啊啊啊~~~~"
  },
  {
    "caress", "輕撫", "輕輕的撫摸著", ""
  },
  {
    "catleg", "貓腿", "大喊：「萬能的", "請賜與我神奇的力量」"
  },
  {
    "clap", "鼓掌", "對", "用力的鼓掌到兩手發腫"
  },
  {
    "claw", "抓抓", "跟野貓借來貓爪，把", "的臉上抓出一道一道的血絲"
  },
  {
    "comfort", "安慰", "溫言安慰", "說：「不哭，不哭，我會負責的。」"
  },
  {
    "cong", "恭喜", "從背後拿出了拉炮，呯！呯！恭喜", ""
  },
  {
    "cpr", "口對口", "對著", "做口對口人工呼吸"
  },
  {
    "cringe", "乞憐", "向", "卑躬屈膝，搖尾乞憐"
  },
  {
    "cry", "大哭", "哭倒在", "的懷裡"
  },
  {
    "dance", "跳舞", "拉著", "的手快樂的跳起舞來"
  },
  {
    "destroy", "毀滅", "祭起了『極大毀滅咒文』，轟向", ""
  },
  {
    "dogleg", "狗腿", "對", "的景仰，有如滔滔江水，永不止息…"
  },
  {
    "drivel", "流口水", "看著", "怔怔的發呆，口水都滴下來了"
  },
  {
    "envy", "羨慕", "對著", "流露出羨慕的眼光"
  },
  {
    "eye", "送秋波", "對", "頻送秋波"
  },
  {
    "fire", "銬問", "拿著火紅的鐵棒走向", ""
  },
  {
    "forgive", "原諒", "對", "說：「別太在意啦，小事一樁」"
  },
  { 
    "french", "法式吻", "和", "的舌頭糾纏在一起熱情的親吻著"
  },
  {
    "gag", "封嘴", "用膠帶把", "的嘴巴貼起來不讓他說話"
  },
  {
    "giggle", "吃笑", "對著", "吃吃的偷笑了幾聲"
  },
  {
    "glue", "補心", "用三秒膠，把", "的心黏了起來"
  },                            /* Thor.0731:應觀眾要求 */
  {
    "goodbye", "告別", "淚\眼汪汪的向", "告別"
  },
  {
    "grin", "奸笑", "對", "露出邪惡的笑容"
  },
  {
    "growl", "咆哮", "對", "憤怒的咆哮不已"
  },
  {
    "hand", "握手", "和", "用力的握手"
  },
  {
    "hide", "躲", "害怕的躲在", "背後"
  },
  {
    "hospitl", "送醫院", "看", "已經奄奄一息，趕快把他送去醫院"
  },
  {
    "hug", "擁抱", "溫柔的將", "擁入懷中"
  },
  {
    "jab", "戳人", "拿起冰鑽，往", "的那裡刺了下去"
  },
  {
    "judo", "過肩摔", "抓住了", "的衣襟，給他一記過肩摔！"
  },
  {
    "kickout", "踢出去", "把", "踢回主選單面壁思過"
  },
  {
    "kick", "踢人", "一腳把", "踹進淡水河裡"
  },
  {
    "kiss", "輕吻", "給", "一個溫柔的吻"
  },
  {
    "laugh", "嘲笑", "大聲的嘲笑", "：「真是笨吶」"
  },
  {
    "levis", "給我", "說：給我", "！其餘免談！"
  },
  {
    "lick", "舔", "舔的", "滿身都是口水"
  },
  {
    "lorelie", "海妖", "呼喚出海妖，用歌聲把", "嚇死了！"
  },
  {
    "love", "表白", "深情的對", "說：「我第一次遇到你，就不可自拔的愛上你了....」"
  },
  {
    "marry", "求婚", "捧著九百九十九朵玫瑰向", "求婚"
  },
  {
    "monica", "小夢", "變身為吸血女，往", "的脖子咬了下去"
  },
  {
    "no", "不要啊", "拼命對著", "搖頭~~~~不要啊~~~~"
  },
  {
    "nod", "點頭", "對著", "輕輕的點了一下頭"
  },
  {
    "nudge", "頂肚子", "用手肘往", "的啤酒肚頂下去"
  },
  {
    "pad", "拍肩膀", "輕拍", "的肩膀，說：「好小子！幹的好！」"
  },
  {
    "pettish", "撒嬌", "跟", "嗲聲嗲氣地撒嬌"
  },
  {
    "pili", "霹靂", "使出 君子風 天地根 般若懺 三式合一打向", "~~~~~~"
  },
  /* Thor.0729: 應觀眾要求 */
  {
    "pinch", "擰人", "用力的把", "的屁股擰的黑青"
  },
  {
    "roll", "打滾", "放出多爾袞的音樂,", "在地上滾來滾去"
  },
  {
    "protect", "保護", "對", "說：「沒關係，我會保護你的」"
  },
  {
    "pull", "拉", "死命地拉住", "不放"
  },
  {
    "punch", "揍人", "狠狠揍了", "一頓"
  },
  {
    "rascal", "耍賴", "跟", "耍賴"
  },
  {
    "recline", "入懷", "鑽到", "的懷裡睡著了……"
  },
  {
    "saw", "電鋸", "拿出電鋸，往", "的那裡鋸了下去"
  },
  {
    "shrug", "聳肩", "無奈地向", "聳了聳肩膀"
  },
  {
    "sigh", "歎氣", "對著", "深深的歎了一口氣"
  },
  {
    "slap", "打耳光", "啪啪的巴了", "一頓耳光"
  },
  {
    "smooch", "擁吻", "和", "緊緊的抱在一起接吻"
  },
  {
    "snicker", "竊笑", "嘿嘿嘿..的對", "竊笑"
  },
  {
    "sniff", "不屑", "對", "嗤之以鼻"
  },

  /* Thor.0729: 中國聊天尺-----觀眾修正版 */
  {
    "sob", "沙灘", "對", "憤憤的說 You S...un Of B...each！"
  },

  {
    "spank", "打屁屁", "用巴掌打", "的臀部"
  },
  /* {"spank", "用貓爪抓", "的臀部","抓臀部"}, */
  {
    "thank", "感謝", "由衷的對", "表示感謝之意"
  },
  {
    "tickle", "搔癢", "咕嘰!咕嘰!搔", "的胳肢窩"
  },
  {
    "wake", "搖醒", "拿起一桶冷水，往", "的頭上澆了下去"
  },
  {
    "wave", "揮手", "對著", "拼命揮手說再見"
  },
  {
    "welcome", "歡迎", "歡迎", "進來八卦一番"
  },
  {
    "what", "什麼", "對", "說：「抱歉，我突然失去記憶了！」"
  },
  {
    "whip", "鞭子", "手上拿著蠟燭，用鞭子痛打", ""
  },
  {
    "wink", "眨眼", "對", "神秘的眨眨眼睛"
  },
  {
    "zap", "猛攻", "拿出火箭筒，往", "轟了過去"
  },

  {
    NULL, NULL, NULL, NULL
  }
};


static int
party_action(cu, cmd, party)
  ChatUser *cu;
  char *cmd;
  char *party;
{
  ChatAction *cap;
  char *verb;

  for (cap = party_data; verb = cap->verb; cap++)
  {
    MYDOG;

    if (str_equal(cmd, verb))
    {
      if (*party == '\0')
      {
        party = "大家";
      }
      else
      {
        ChatUser *xuser;

        xuser = fuzzy_cuser_by_chatid(party);
        if (xuser == NULL)
        {                       /* Thor.0724: 用 userid也嘛通 */
          xuser = cuser_by_userid(party);
        }

        if (xuser == NULL)
        {
          sprintf(chatbuf, msg_no_such_id, party);
          send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
          return 0;
        }
        else if (xuser == FUZZY_USER)
        {
          sprintf(chatbuf, "※ 請指明聊天代號");
          send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
          return 0;
        }
        else if (cu->room != xuser->room || CLOAK(xuser))
        {
          sprintf(chatbuf, msg_not_here, party);
          send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
          return 0;
        }
        else
        {
          party = xuser->chatid;
        }
      }
      sprintf(chatbuf, "\x1b[1;32m%s \x1b[31m%s\x1b[33m %s \x1b[31m%s\x1b[m",
        cu->chatid, cap->part1_msg, party, cap->part2_msg);
      send_to_room(cu->room, chatbuf, cu->userno, MSG_MESSAGE);
      return 0;                 /* Thor: cu->room 是否為 NULL? */
    }
  }
  return 1;
}


/* --------------------------------------------- */
/* MUD-like social commands : speak              */
/* --------------------------------------------- */


static ChatAction speak_data[] =
{

  {
    "ask", "詢問", "問說", NULL
  },
  {
    "chant", "歌頌", "高聲歌頌", NULL
  },
  {
    "cheer", "喝采", "喝采", NULL
  },
  {
    "chuckle", "輕笑", "輕笑", NULL
  },
  {
    "curse", "暗幹", "暗幹", NULL
  },
  /* {"curse", "咒罵", NULL}, */
  {
    "demand", "要求", "要求", NULL
  },
  {
    "frown", "皺眉頭", "蹙眉", NULL
  },
  {
    "groan", "呻吟", "呻吟", NULL
  },
  {
    "grumble", "發牢騷", "發牢騷", NULL
  },
  {
    "guitar", "彈唱", "邊彈著吉他，邊唱著", NULL
  },
  {
    "hum", "喃喃", "喃喃自語", NULL
  },
  {
    "moan", "怨嘆", "怨嘆", NULL
  },
  {
    "notice", "強調", "強調", NULL
  },
  {
    "order", "命令", "命令", NULL
  },
  {
    "ponder", "沈思", "沈思", NULL
  },
  {
    "pout", "噘嘴", "噘著嘴說", NULL
  },
  {
    "pray", "祈禱", "祈禱", NULL
  },
  {
    "request", "懇求", "懇求", NULL
  },
  {
    "shout", "大罵", "大罵", NULL
  },
  {
    "sing", "唱歌", "唱歌", NULL
  },
  {
    "smile", "微笑", "微笑", NULL
  },
  {
    "smirk", "假笑", "假笑", NULL
  },
  {
    "swear", "發誓", "發誓", NULL
  },
  {
    "tease", "嘲笑", "嘲笑", NULL
  },
  {
    "whimper", "嗚咽", "嗚咽的說", NULL
  },
  {
    "yawn", "哈欠", "邊打哈欠邊說", NULL
  },
  {
    "yell", "大喊", "大喊", NULL
  },
  {
    NULL, NULL, NULL, NULL
  }
};


static int
speak_action(cu, cmd, msg)
  ChatUser *cu;
  char *cmd;
  char *msg;
{
  ChatAction *cap;
  char *verb;

  for (cap = speak_data; verb = cap->verb; cap++)
  {
    MYDOG;

    if (str_equal(cmd, verb))
    {
      sprintf(chatbuf, "\x1b[1;32m%s \x1b[31m%s：\x1b[33m %s\x1b[m",
        cu->chatid, cap->part1_msg, msg);
      send_to_room(cu->room, chatbuf, cu->userno, MSG_MESSAGE);
      return 0;                 /* Thor: cu->room 是否為 NULL? */
    }
  }
  return 1;
}


/* -------------------------------------------- */
/* MUD-like social commands : condition          */
/* -------------------------------------------- */


static ChatAction condition_data[] =
{
  {
    "applaud", "拍手", "用力拍手，鼓掌叫好", NULL
  },
  {
    "ayo", "唉呦喂", "慘叫：「唉呦喂~~~」", NULL
  },
  {
    "back", "回來", "酷酷的說：「I'm back！」", NULL
  },
  {
    "blood", "在血中", "慘叫一聲，倒在血泊之中", NULL
  },
  {
    "blush", "臉紅", "不好意思的臉都紅了", NULL
  },
  {
    "broke", "心碎", "的心破碎成一片一片的", NULL
  },                            /* Thor.0731:應觀眾要求 */
  {
    "careles", "沒人理", "啜泣：嗚～～都沒有人理我 :~~~~", NULL
  },
  {
    "chew", "嗑瓜子", "很悠閒的嗑起瓜子來了", NULL
  },
  {
    "climb", "爬山", "自己慢慢爬上山來……", NULL
  },
  {
    "cold", "感冒了", "感冒了,媽媽不讓我出去玩 :~~~(", NULL
  },
  {
    "cough", "咳嗽", "咳了幾聲，好像在暗示什麼", NULL
  },
  {
    "die", "暴斃", "倒在地上，當場暴斃", NULL
  },
  {
    "faint", "昏倒", "聽到這個消息，當場昏倒", NULL
  },
  {
    "flop", "跌倒", "噗通一聲，突然跌倒！", NULL
  },
  {
    "fly", "蒼蠅", "大喊：「I'm flying！」", NULL
  },
  {
    "frown", "蹙眉", "眉頭揪的跟包子一樣", NULL
  },
  {
    "gold", "拿金牌", "唱著：『金ㄍㄠˊ金ㄍㄠˊ  出國比賽! 得冠軍，拿金牌，光榮倒鄧來！』", NULL
  },
  {
    "gulu", "肚子餓", "的肚子發出咕嚕~~~咕嚕~~~的聲音", NULL
  },
  {
    "haha", "哇哈哈", "狂笑：哇哈哈哈.....^o^", NULL
  },
  {
    "helpme", "求救", "大喊~~~救命啊~~~~", NULL
  },
  {
    "hoho", "呵呵笑", "發出 喔～呵～呵～呵～ 的笑聲", NULL
  },
  {
    "happy", "高興", "高興得在地上滾來滾去", NULL
  },
  /* {"happy", "高興", "ＹＡ！ *^_^*", NULL}, */
  /* {"happy", "", "r-o-O-m....聽了真爽！", NULL}, */
  /* {"hurricane", "Ｈｏ---Ｒｙｕ--Ｋａｎ！！！", NULL}, */
  {
    "idle", "發呆", "坐在原地發呆", NULL
  },
  {
    "jacky", "晃晃", "痞子般的晃來晃去", NULL
  },
  {
    "lag", "網路慢", "大喊：「有夠龜速的 LowNet 啊！！！」", NULL
  },
  {
    "luck", "幸運", "大喊：「啊！福氣啦！」", NULL
  },
  {
    "macarn", "一種舞", "開始跳起了ＭaＣaＲeＮa～～～～", NULL
  },
  {
    "miou", "喵喵", "開始狂叫：喵喵口苗口苗～～～～～", NULL
  },
  {
    "mouth", "扁嘴", "嘴巴嘟的可以掛兩斤肉了", NULL
  },
  {
    "nani", "怎麼會", "慘叫：奈ㄝ啊捏??", NULL
  },
  {
    "nose", "流鼻血", "鼻血突然噴出來了....", NULL
  },
  {
    "puke", "嘔吐", "嘩啦嘩啦的吐了滿地都是。", NULL
  },
  {
    "rest", "休息", "休息中，請勿打擾", NULL
  },
  {
    "reverse", "翻肚", "翻白肚了", NULL
  },
  {
    "room", "開房間", "呻吟：r-o-O-m-r-O-Ｏ-Mmm-rRＲ........", NULL
  },
  {
    "shake", "搖頭", "搖了搖頭", NULL
  },
  {
    "sleep", "睡著", "趴在鍵盤上睡著了，口水流進去，裡面的螞蟻嚇得趕快搬家", NULL
  },
  {
    "so", "就醬子", "說：「那就先醬吧!!」", NULL
  },
  {
    "sorry", "道歉", "呼吼：嗚啊!!我對不起大家,我對不起國家社會~~~~~~嗚啊~~~~~", NULL
  },
  {
    "story", "講古", "開始講古了", NULL
  },
  {
    "strut", "搖擺\走", "臭屁的大搖大擺\走來走去", NULL
  },
  {
    "suicide", "自殺", "拿出武士刀切腹", NULL
  },
  {
    "tea", "泡茶", "泡了壺好茶，繼續哈拉下去", NULL
  },
  {
    "think", "思考", "歪著頭想了一下", NULL
  },
  {
    "tongue", "吐舌", "吐了吐舌頭", NULL
  },
  {
    "wall", "撞牆", "跑去撞牆", NULL
  },
  {
    "wawa", "哇哇", "放聲大哭：哇哇哇~~~~~!!!!!  ~~~>_<~~~", NULL
  },
  {
    "www", "汪汪", "汪汪汪....叫個不停", NULL
  },
  {
    "ya", "勝利", "比了一個勝利手勢：「Ya!」", NULL
  },
  {
    "zzz", "打呼", "呼嚕~~~~ZZzZzｚＺZZzzZzzzZZ...", NULL
  },

  {
    NULL, NULL, NULL, NULL
  }
};


static int
condition_action(cu, cmd)
  ChatUser *cu;
  char *cmd;
{
  ChatAction *cap;
  char *verb;

  for (cap = condition_data; verb = cap->verb; cap++)
  {
    MYDOG;

    if (str_equal(cmd, verb))
    {
      sprintf(chatbuf, "\x1b[1;32m%s \x1b[31m%s\x1b[m",
        cu->chatid, cap->part1_msg);
      send_to_room(cu->room, chatbuf, cu->userno, MSG_MESSAGE);
      return 1;                 /* Thor: cu->room 是否為 NULL? */
    }
  }
  return 0;
}


/* --------------------------------------------- */
/* MUD-like social commands : help               */
/* --------------------------------------------- */


static char *dscrb[] =
{
  "\x1b[1;37m【 Verb + Nick：   動詞 + 對方名字 】\x1b[36m   例：//kick piggy\x1b[m",
  "\x1b[1;37m【 Verb + Message：動詞 + 要說的話 】\x1b[36m   例：//sing 天天天藍\x1b[m",
  "\x1b[1;37m【 Verb：動詞 】    ↑↓：舊話重提\x1b[m", NULL
};
ChatAction *catbl[] =
{
  party_data, speak_data, condition_data, NULL
};

static void
chat_partyinfo(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (!common_client_command)
    return;                     /* only allow common client to retrieve it */

  sprintf(chatbuf, "3 動作  交談  狀態");
  send_to_user(cu, chatbuf, 0, MSG_PARTYINFO);
}

static void
chat_party(cu, msg)
  ChatUser *cu;
  char *msg;
{
  int kind, i;
  ChatAction *cap;

  if (!common_client_command)
    return;

  kind = atoi(nextword(&msg));
  if (kind < 0 || kind > 2)
    return;

  sprintf(chatbuf, "%d  %s", kind, kind == 2 ? "I" : "");

  /* Xshadow: 只有 condition 才是 immediate mode */
  send_to_user(cu, chatbuf, 0, MSG_PARTYLISTSTART);

  cap = catbl[kind];
  for (i = 0; cap[i].verb; i++)
  {
    sprintf(chatbuf, "%-10s %-20s", cap[i].verb, cap[i].chinese);
    /* for (j=0;j<1000000;j++); */
    send_to_user(cu, chatbuf, 0, MSG_PARTYLIST);
  }

  sprintf(chatbuf, "%d", kind);
  send_to_user(cu, chatbuf, 0, MSG_PARTYLISTEND);
}


#define SCREEN_WIDTH    80
#define MAX_VERB_LEN    8
#define VERB_NO         10

static void
view_action_verb(cu, cmd)       /* Thor.0726: 新加動詞分類顯示 */
  register ChatUser *cu;
  char cmd;
{
  register int i;
  register char *p, *q, *data, *expn;
  register ChatAction *cap;

  send_to_user(cu, "/c", 0, MSG_CLRSCR);

  data = chatbuf;

  if (cmd < '1' || cmd > '3')
  {                             /* Thor.0726: 寫得不好, 想辦法改進... */
    for (i = 0; p = dscrb[i]; i++)
    {
      sprintf(data, "  [//]help %d          - MUD-like 社交動詞   第 %d 類", i + 1, i + 1);
      MYDOG;
      send_to_user(cu, data, 0, MSG_MESSAGE);
      send_to_user(cu, p, 0, MSG_MESSAGE);
      send_to_user(cu, " ", 0, MSG_MESSAGE);    /* Thor.0726: 換行, 需要 " "
                                                 * 嗎? */
    }
  }
  else
  {
    i = cmd - '1';

    send_to_user(cu, dscrb[i], 0, MSG_MESSAGE);

    expn = chatbuf + 100;       /* Thor.0726: 應該不會overlap吧? */

    *data = '\0';
    *expn = '\0';

    cap = catbl[i];

    for (i = 0; p = cap[i].verb; i++)
    {
      MYDOG;
      q = cap[i].chinese;

      strcat(data, p);
      strcat(expn, q);

      if (((i + 1) % VERB_NO) == 0)
      {
        send_to_user(cu, data, 0, MSG_MESSAGE);
        send_to_user(cu, expn, 0, MSG_MESSAGE); /* Thor.0726: 顯示中文註解 */
        *data = '\0';
        *expn = '\0';
      }
      else
      {
        strncat(data, "        ", MAX_VERB_LEN - strlen(p));
        strncat(expn, "        ", MAX_VERB_LEN - strlen(q));
      }
    }
    if (i % VERB_NO)
    {
      send_to_user(cu, data, 0, MSG_MESSAGE);
      send_to_user(cu, expn, 0, MSG_MESSAGE);   /* Thor.0726: 顯示中文註解 */
    }
  }
  /* send_to_user(cu, " ",0); *//* Thor.0726: 換行, 需要 " " 嗎? */
}

#if 0
static void
view_action_verb(cu, cmd)
  register ChatUser *cu;
  char cmd;
{
  register int i, j;
  register char *p, *data;
  register ChatAction *cap;

  send_to_user(cu, "", 0, MSG_CLRSCR);

  data = chatbuf;

  for (i = 0; p = dscrb[i]; i++)
  {
    MYDOG;

    send_to_user(cu, p, 0, MSG_MESSAGE);
    *data = '\0';
    j = 0;
    cap = catbl[i];
    while (p = cap[j++].verb)
    {
      MYDOG;

      strcat(data, p);
      if ((j % VERB_NO) == 0)
      {
        send_to_user(cu, data, 0, MSG_MESSAGE);
        *data = '\0';
      }
      else
      {
        strncat(data, "        ", MAX_VERB_LEN - strlen(p));
      }
    }
    if (j % VERB_NO)
      send_to_user(cu, data, 0);
    send_to_user(cu, " ", 0);
  }
}
#endif


/* ----------------------------------------------------- */
/* chat user service routines                            */
/* ----------------------------------------------------- */


static ChatCmd chatcmdlist[] =
{
  "/", chat_help, 0,
  "act", chat_act, 0,
  "bye", chat_goodbye, 0,
  "chatroom", chat_chatroom, 1, /* Xshadow: for common client */
  "clear", chat_clear, 0,
  "cloak", chat_cloak, 2,
  "color", chat_color, 0,
  "date", chat_date, 0,
  "flags", chat_setroom, 0,
  "help", chat_help, 0,
  "ignore", chat_ignore, 1,
  "invite", chat_invite, 0,
  "join", chat_join, 0,
  "kick", chat_kick, 1,
  "msg", chat_private, 0,
  "nick", chat_nick, 0,
  "operator", chat_makeop, 0,
  "party", chat_party, 1,       /* Xshadow: party data for common client */
  "partyinfo", chat_partyinfo, 1,       /* Xshadow: party info for common
                                         * client */

#ifndef STAND_ALONE
  "query", chat_query, 0,
#endif

  "room", chat_list_rooms, 0,
  "unignore", chat_unignore, 1,
  "whoin", chat_list_by_room, 1,
  "wall", chat_broadcast, 2,

  "who", chat_map_chatids_thisroom, 0,
  "list", chat_list_users, 0,
  "topic", chat_topic, 1,
  "version", chat_version, 1,

  NULL, NULL, 0
};

/* Thor: 0 不用 exact, 1 要 exactly equal, 2 秘密指令 */


static int
command_execute(cu)
  ChatUser *cu;
{
  char *cmd, *msg;
  ChatCmd *cmdrec;
  int match, ch;

  msg = cu->ibuf;
  match = *msg;

  /* Validation routine */

  if (cu->room == NULL)
  {
    /* MUST give special /! or /-! command if not in the room yet */

    if (match == '/' && ((ch = msg[1]) == '!' || (ch == '-' && msg[2] == '!')))
    {
      cu->clitype = (ch == '-') ? 1 : 0;
      return (login_user(cu, msg + 2 + cu->clitype));
    }
    else
      return -1;
  }

  /* If not a /-command, it goes to the room. */

  if (match != '/')
  {
    if (match)
    {
      char buf[16];

      sprintf(buf, "%s:", cu->chatid);
      sprintf(chatbuf, "\x1b[1;3%dm%-10s%s\x1b[m", cu->color,buf, msg);
      if (!CLOAK(cu))           /* Thor: 聊天室隱身術 */
        send_to_room(cu->room, chatbuf, cu->userno, MSG_MESSAGE);
      /* Thor: 要 check cu->room NULL嗎? */

    }
    return 0;
  }

  msg++;
  cmd = nextword(&msg);
  match = 0;

  if (*cmd == '/')
  {
    cmd++;
    if (!*cmd || str_equal(cmd, "help"))
    {
      /* Thor.0726: 動詞分類 */
      cmd = nextword(&msg);
      view_action_verb(cu, *cmd);
      match = 1;
    }
    else if (party_action(cu, cmd, msg) == 0)
      match = 1;
    else if (speak_action(cu, cmd, msg) == 0)
      match = 1;
    else
      match = condition_action(cu, cmd);
  }
  else
  {
    char *str;

    common_client_command = 0;
    if (*cmd == '-')
      if (cu->clitype)
      {
        cmd++;                  /* Xshadow: 指令從下一個字元才開始 */

        common_client_command = 1;
      }
      else;                     /* 不是 common client 但送出 common client
                                 * 指令 -> 假裝沒看到 */

    for (cmdrec = chatcmdlist; str = cmdrec->cmdstr; cmdrec++)
    {
      MYDOG;

      switch (cmdrec->exact)
      {
      case 1:                   /* exactly equal */
        match = str_equal(cmd, str);
        break;
      case 2:                   /* Thor: secret command */
        if (CHATSYSOP(cu))
          match = str_equal(cmd, str);
        break;
      default:                  /* not necessary equal */
        match = str_match(cmd, str) >= 0;
        break;
      }

      if (match)
      {
        cmdrec->cmdfunc(cu, msg);
        break;
      }
    }
  }

  if (!match)
  {
    sprintf(chatbuf, "◆ 指令錯誤：/%s", cmd);
    send_to_user(cu, chatbuf, 0, MSG_MESSAGE);
  }
  return 0;
}


/* ----------------------------------------------------- */
/* serve chat_user's connection                          */
/* ----------------------------------------------------- */


static int
cuser_serve(cu)
  ChatUser *cu;
{
  register int ch, len, isize;
  register char *str, *cmd;
  static char buf[80];

  str = buf;
  len = recv(cu->sock, str, sizeof(buf) - 1, 0);
  if (len <= 0)
  {
    /* disconnected */

    exit_room(cu, EXIT_LOSTCONN, (char *) NULL);
    return -1;
  }

#if 0
  /* Xshadow: 將送達的資料忠實紀錄下來 */
  memcpy(logbuf, buf, sizeof(buf));
  for (ch = 0; ch < sizeof(buf); ch++)
    if (!logbuf[ch])
      logbuf[ch] = '$';

  logbuf[len + 1] = '\0';
  logit("recv: ", logbuf);
#endif

#if 0
  logit(cu->userid, str);
#endif

  isize = cu->isize;
  cmd = cu->ibuf + isize;
  while (len--)
  {
    MYDOG;

    ch = *str++;

    if (ch == '\r' || !ch)
      continue;
    if (ch == '\n')
    {
      *cmd = '\0';

      isize = 0;
      cmd = cu->ibuf;

      if (command_execute(cu) < 0)
        return -1;

      continue;
    }
    if (isize < 79)
    {
      *cmd++ = ch;
      isize++;
    }
  }
  cu->isize = isize;
  return 0;
}


/* ----------------------------------------------------- */
/* chatroom server core routines                         */
/* ----------------------------------------------------- */

static int
start_daemon()
{
  int fd, value;
  char buf[80];
  struct sockaddr_in fsin;
  struct linger ld;
  struct rlimit limit;
  time_t dummy;
  struct tm *dummy_time;

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time(&dummy);
  dummy_time = gmtime(&dummy);
  dummy_time = localtime(&dummy);
  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", dummy_time);

  /* --------------------------------------------------- */
  /* speed-hacking DNS resolve                           */
  /* --------------------------------------------------- */

  gethostname(buf, sizeof(buf));

  /* Thor: 萬一server尚未接受connection, 就回去的話, client 第一次會進入失敗 */
  /* 所以移至 listen 後 */

  /* --------------------------------------------------- */
  /* detach daemon process                               */
  /* --------------------------------------------------- */

  close(0);
  close(1);
  close(2);

  if (fork())
    exit(0);

  chdir(BBSHOME);

  setsid();

  /* --------------------------------------------------- */
  /* adjust the resource limit                           */
  /* --------------------------------------------------- */

  getrlimit(RLIMIT_NOFILE, &limit);
  limit.rlim_cur = limit.rlim_max;
  setrlimit(RLIMIT_NOFILE, &limit);

#if 0
  while (fd)
  {
    close(--fd);
  }

  value = getpid();
  setpgrp(0, value);

  if ((fd = open("/dev/tty", O_RDWR)) >= 0)
  {
    ioctl(fd, TIOCNOTTY, 0);    /* Thor : 為什麼還要用  tty? */
    close(fd);
  }
#endif

  fd = open(CHAT_PIDFILE, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd >= 0)
  {
    /* sprintf(buf, "%5d\n", value); */
    sprintf(buf, "%5d\n", getpid());
    write(fd, buf, 6);
    close(fd);
  }

#if 0
  /* ------------------------------ */
  /* trap signals                   */
  /* ------------------------------ */

  for (fd = 1; fd < NSIG; fd++)
  {

    signal(fd, SIG_IGN);
  }
#endif

  fd = socket(PF_INET, SOCK_STREAM, 0);

#if 0
  value = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, value | O_NDELAY);
#endif

  value = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));

#if 0
  setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &value, sizeof(value));

  value = 81920;
  setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &value, sizeof(value));
#endif

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  memset((char *) &fsin, 0, sizeof(fsin));
  fsin.sin_family = AF_INET;
  fsin.sin_port = htons(CHATPORT);
  fsin.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, (struct sockaddr *) & fsin, sizeof(fsin)) < 0)
    exit(1);

  listen(fd, SOCK_QLEN);

  return fd;
}


static void
free_resource(fd)
  int fd;
{
  static int loop = 0;
  register ChatUser *user;
  register int sock, num;

  num = 0;
  for (user = mainuser; user; user = user->unext)
  {
    MYDOG;

    num++;
    sock = user->sock;
    if (fd < sock)
      fd = sock;
  }

  sprintf(chatbuf, "%d, %d user (%d -> %d)", ++loop, num, maxfds, fd);
  logit("LOOP", chatbuf);

  maxfds = fd + 1;
}


#ifdef  SERVER_USAGE
static void
server_usage()
{
  struct rusage ru;
  char buf[2048];

  if (getrusage(RUSAGE_SELF, &ru))
    return;

  sprintf(buf, "\n[Server Usage]\n\n"
    "user time: %.6f\n"
    "system time: %.6f\n"
    "maximum resident set size: %lu P\n"
    "integral resident set size: %lu\n"
    "page faults not requiring physical I/O: %d\n"
    "page faults requiring physical I/O: %d\n"
    "swaps: %d\n"
    "block input operations: %d\n"
    "block output operations: %d\n"
    "messages sent: %d\n"
    "messages received: %d\n"
    "signals received: %d\n"
    "voluntary context switches: %d\n"
    "involuntary context switches: %d\n"
    "gline: %d\n\n",

    (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
    (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
    ru.ru_maxrss,
    ru.ru_idrss,
    ru.ru_minflt,
    ru.ru_majflt,
    ru.ru_nswap,
    ru.ru_inblock,
    ru.ru_oublock,
    ru.ru_msgsnd,
    ru.ru_msgrcv,
    ru.ru_nsignals,
    ru.ru_nvcsw,
    ru.ru_nivcsw,
    gline);

  write(flog, buf, strlen(buf));
}
#endif


static void
abort_server()
{
  log_close();
  exit(1);
}


static void
reaper()
{
  int state;

  while (waitpid(-1, &state, WNOHANG | WUNTRACED) > 0)
  {
    MYDOG;
  }
}


int
main()
{
  register int msock, csock, nfds;
  register ChatUser *cu;
  register fd_set *rptr, *xptr;
  fd_set rset, xset;
  struct timeval tv;
  time_t uptime, tmaintain;

  msock = start_daemon();

  setgid(BBSGID);
  setuid(BBSUID);

  log_init();

  signal(SIGBUS, SIG_IGN);
  signal(SIGSEGV, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGURG, SIG_IGN);

  signal(SIGCHLD, reaper);
  signal(SIGTERM, abort_server);

#ifdef  SERVER_USAGE
  signal(SIGPROF, server_usage);
#endif

  /* ----------------------------- */
  /* init variable : rooms & users */
  /* ----------------------------- */

  mainuser = NULL;
  memset(&mainroom, 0, sizeof(mainroom));
  strcpy(mainroom.name, MAIN_NAME);
  strcpy(mainroom.topic, MAIN_TOPIC);

  /* ----------------------------------- */
  /* main loop                           */
  /* ----------------------------------- */

#if 0
  /* Thor: 在listen 後才回client, 每次進來就會成功 */
  if (fork())
    exit(0);
#endif

  FD_ZERO(&mainfds);
  FD_SET(msock, &mainfds);
  rptr = &rset;
  xptr = &xset;
  maxfds = msock + 1;

  tmaintain = time(0) + CHAT_INTERVAL;

  for (;;)
  {
    uptime = time(0);
    if (tmaintain < uptime)
    {
      tmaintain = uptime + CHAT_INTERVAL;

      /* client/server 版本利用 ping-pong 方法判斷 user 是不是還活著 */
      /* 如果 client 已經結束了，就釋放其 resource */

      free_resource(msock);
    }

    MYDOG;

    memcpy(rptr, &mainfds, sizeof(fd_set));
    memcpy(xptr, &mainfds, sizeof(fd_set));

    /* Thor: for future reservation bug */

    tv.tv_sec = CHAT_INTERVAL;
    tv.tv_usec = 0;

    MYDOG;

    nfds = select(maxfds, rptr, NULL, xptr, &tv);

    MYDOG;
    /* free idle user & chatroom's resource when no traffic */

    if (nfds == 0)
    {
      continue;
    }

    /* check error condition */

    if (nfds < 0)
    {
      csock = errno;
      if (csock != EINTR)
        logit("select", sys_errlist[csock]);
      continue;
    }

    /* accept new connection */

    if (FD_ISSET(msock, rptr))
    {
      for (;;)
      {
        MYDOG;                          /* Thor: check for endless */
        csock = accept(msock, NULL, NULL);

        if (csock >= 0)
        {
          MYDOG;
          if (cu = (ChatUser *) malloc(sizeof(ChatUser)))
          {
            memset(cu, 0, sizeof(ChatUser));
            cu->sock = csock;

            cu->unext = mainuser;
            mainuser = cu;

#if 0
            if (mainuser.next)
              mainuser.next->prev = cu;
            cu->next = mainuser.next;
            mainuser.next = cu;
            cu->prev = &mainuser;
#endif

            totaluser++;
            FD_SET(csock, &mainfds);
            if (csock >= maxfds)
              maxfds = csock + 1;

#ifdef  DEBUG
            logit("accept", "OK");
#endif
          }
          else
          {
            close(csock);
            logit("accept", "malloc fail");
          }
          MYDOG;

          break;
        }

        csock = errno;
        if (csock != EINTR)
        {
          logit("accept", sys_errlist[csock]);
          break;
        }
      }

      FD_CLR(msock, rptr);

      if (--nfds <= 0)
        continue;
    }

    for (cu = mainuser; cu; cu = cu->unext)
    {
      MYDOG;

      csock = cu->sock;

      if (FD_ISSET(csock, xptr))
      {
        logout_user(cu);
        FD_CLR(csock, xptr);
      }
      else if (FD_ISSET(csock, rptr))
      {
        if (cuser_serve(cu) < 0)
          logout_user(cu);
      }
      else
      {
        continue;
      }

      FD_CLR(csock, rptr);
      if (--nfds <= 0)
        break;
    }

    /* end of main loop */
  }
}
