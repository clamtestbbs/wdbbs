/*-------------------------------------------------------*/
/* chat.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : chat client for xchatd                       */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef lint
#include <sys/uio.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXLASTCMD 6
char chatroom[IDLEN];           /* Chat-Room Name */
int stop_line;                  /* next line of bottom of message window area */
static FILE* flog;
/*
woju
chatid prompt, for call back...needs longer
*/
int chatid_len = 10;


extern char page_requestor[];
extern char *modestring();
extern char *Cdate();

int chatline;


void
printchatline(str)
  char *str;
{
  move(chatline, 0);
  if (*str == '@') {
     char *id, *msg;
     char buf[100];
     user_info *uentp;
     int tuid;
     extern cmpuids();

     id = str + 1;
     (msg = strchr(id, '@') + 1)[-1] = 0;
     if ((tuid = searchuser(id))  && tuid != usernum &&
         (uentp = (user_info *) search_ulistn(cmpuids, tuid, 1))) {

        sprintf(buf, "★ %s 不在聊天室，發送水球★\n", id);
        outs(buf);
        strcpy(buf + 1, msg);
        *buf = 0;
        my_write(uentp->pid, buf);
        sprintf(buf, "%s★ %s", id, msg);
        strcpy(str, buf);
     }
     else {
        sprintf(buf, "msg fail: %s 不在站上\n", id);
        outs(buf);
     }
  }
  else if (*str == '>' && !PERM_HIDE(currutmp))
     return;
  else if (chatline < stop_line - 1) {
     chatline++;
  }
  else {
     region_scroll_up(2, stop_line - 2);
     move(stop_line - 2, 0);
  }
  outs(str);
  outc('\n');
  outs("→");

  if (flog)
     fprintf(flog, "%s\n", str);
}


static void
chat_clear()
{
  for (chatline = 2; chatline < stop_line; chatline++)
  {
    move(chatline, 0);
    clrtoeol();
  }
  move(b_lines, 0);
  clrtoeol();
  move(chatline = 2, 0);
  outs("→");
}


static void
print_chatid(chatid)
  char *chatid;
{
  move(b_lines - 1, 0);
  clrtoeol();
  outs(chatid);
  outc(':');
}


static int
chat_send(fd, buf)
  int fd;
  char *buf;
{
  int len;
  char genbuf[200];

  sprintf(genbuf, "%s\n", buf);
  len = strlen(genbuf);
  return (send(fd, genbuf, len, 0) == len);
}


static int
chat_recv(fd, chatid)
  int fd;
  char *chatid;
{
  static char buf[512];
  static int bufstart = 0;
  char genbuf[200];
  int c, len;
  char *bptr;

  len = sizeof(buf) - bufstart - 1;
  if ((c = recv(fd, buf + bufstart, len, 0)) <= 0)
    return -1;
  c += bufstart;

  bptr = buf;
  while (c > 0)
  {
    len = strlen(bptr) + 1;
    if (len > c && len < (sizeof buf / 2))
      break;

    if (*bptr == '/')
    {
      switch (bptr[1])
      {
      case 'c':
        chat_clear();
        break;

      case 'n':
        strncpy(chatid, bptr + 2, 8);
        print_chatid(chatid);
        clrtoeol();
        break;

      case 'r':
        strncpy(chatroom, bptr + 2, IDLEN - 1);
        break;

      case 't':
        move(0, 0);
        clrtoeol();
        sprintf(genbuf, "談天室 [%s]", chatroom);
        prints("\x1b[1;46m %-21s \x1b[44m 話題：%-48s\x1b[0m", genbuf, bptr + 2);
      }
    }
    else
      printchatline(bptr);

    c -= len;
    bptr += len;
  }

  if (c > 0)
  {
    strcpy(genbuf, bptr);
    strcpy(buf, genbuf);
    bufstart = len - 1;
  }
  else
    bufstart = 0;
  return 0;
}

struct chat_command
{
  char *cmdname;                /* Char-room command length */
  void (*cmdfunc) ();           /* Pointer to function */
};


char*
select_address()
{
  int c;
  FILE* fp;
  char nametab[25][90];
  char iptab[25][18] , buf[80];
  move(1,0);
  clrtobot();
  outs("\n          \x1b[1;36m【找個地方抬抬槓吧!】\x1b[0m ◎  【以下為本站登記有案的茶樓】          \n");
  trans_buffer[0]=0;
  if (fp = fopen("etc/teashop", "r"))
  {
     for (c=0;fscanf(fp,"%s%s",iptab[c],nametab[c])!=EOF;c++)
          {
            sprintf(buf,"\n            (\x1b[36m%d\x1b[0m) %-30s     %s",c+1,nametab[c],iptab[c]);
            outs(buf);
          }
     getdata(20 , 10, "★ 請選擇，[0]離開：", buf, 3,
                LCECHO,0);
     if(buf[1])
        buf[0]=((buf[0]-'0')*10+(buf[1]-'0'))-1;
     else buf[0] -= '1';
     if( buf[0] >= 0 && buf[0] < c)
      {
        strcpy(trans_buffer,iptab[buf[0]]);
      }
   }
  else
    pressanykey("本站沒有登記任何合格茶樓");
 return trans_buffer;
}

int
t_chat()
{
  char inbuf[80], chatid[20], lastcmd[MAXLASTCMD][80], *ptr;
  struct sockaddr_in sin;
  struct hostent *h;
  int cfd, cmdpos, ch;
  int currchar;
  int newmail;
  extern int dumb_term;
  int chatting = YEA;
  char fpath[80];
  char genbuf[200];
  char roomtype;

  if(currstat == READING)
    strcpy(inbuf, MYIP);
  else
    strcpy(inbuf,select_address());
  if(inbuf[0]==0)
  {
     return -1;
  }

  outs("                     驅車前往 請梢候........         ");
  if (!(h = gethostbyname(inbuf)))
  {
    perror("gethostbyname");
    return -1;
  }
  memset(&sin, 0, sizeof sin);
  sin.sin_family = PF_INET;
  /* sin.sin_family = h->h_addrtype; */
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);
  sin.sin_port = htons(CHATPORT);
  cfd = socket(sin.sin_family, SOCK_STREAM, 0);
  if (!(connect(cfd, (struct sockaddr *) & sin, sizeof sin)))
    {
        roomtype = 1;
    }
  else
    {
      sin.sin_port = CHATPORT;
      cfd = socket(sin.sin_family, SOCK_STREAM, 0);
      if (!(connect(cfd, (struct sockaddr *) & sin, sizeof sin)))
        {
          roomtype = 2;
        }
      else
        {
         pressanykey("哇! 沒人在那邊耶...要有那地方的人先去開門啦!...");
         return -1;
        }
    }


  while (1)
  {
    char room[80];
    getdata(b_lines - 1, 0, "請輸入聊天代號：", inbuf, 9, DOECHO, NULL);
    sprintf(chatid, "%s", (inbuf[0] ? inbuf : cuser.userid));
    chatid[8] = '\0';
/*
         舊格式:    /! 使用者編號 使用者等級 UserID ChatID
         新格式:    /! UserID ChatID Password
*/
    if(currstat == READING) strcpy(room,currboard);
    else strcpy(room,"main");
    if(roomtype == 1)
        sprintf(inbuf, "/! %s %s %s %d %s",
           cuser.userid, chatid, cuser.passwd, usernum, room);
    else
        sprintf(inbuf, "/! %d %d %s %s",
           usernum, cuser.userlevel, cuser.userid, chatid);
    chat_send(cfd, inbuf);
    if (recv(cfd, inbuf, 3, 0) != 3)
    {
      return 0;
    }
    if (!strcmp(inbuf, CHAT_LOGIN_OK))
      break;
    else if (!strcmp(inbuf, CHAT_LOGIN_EXISTS))
      ptr = "這個代號已經有人用了";
    else if (!strcmp(inbuf, CHAT_LOGIN_INVALID))
      ptr = "這個代號是錯誤的";
    else if (!strcmp(inbuf, CHAT_LOGIN_BOGUS))
      ptr = "請勿派遣分身進入聊天室 !!";

    move(b_lines - 2, 0);
    outs(ptr);
    clrtoeol();
    bell();
  }

  log_usies("CHAT ", chatid);

  add_io(cfd, 0);

  newmail = currchar = 0;
  cmdpos = -1;
  memset(lastcmd, 0, MAXLASTCMD * 80);

  setutmpmode(CHATING);
  currutmp->in_chat = YEA;
  strcpy(currutmp->chatid, chatid);

  clear();
  chatline = 2;
  strcpy(inbuf, chatid);
  stop_line = t_lines - 3;


  move(stop_line, 0);
  outs(msg_seperator);
  move(1, 0);
  outs(msg_seperator);
  print_chatid(chatid);
  memset(inbuf, 0, 80);

/*
woju
*/
  sethomepath(fpath, cuser.userid);
  strcpy(fpath, tempnam(fpath, "chat_"));
  flog = fopen(fpath, "w");


  while (chatting)
  {
    move(b_lines - 1, currchar + chatid_len);
    ch = igetkey();

    switch (ch)
    {
    case KEY_DOWN:
      cmdpos += MAXLASTCMD - 2;

    case KEY_UP:
      cmdpos++;
      cmdpos %= MAXLASTCMD;
      strcpy(inbuf, lastcmd[cmdpos]);
      move(b_lines - 1, chatid_len);
      clrtoeol();
      outs(inbuf);
      currchar = strlen(inbuf);
      continue;

    case KEY_LEFT:
      if (currchar)
        --currchar;
      continue;

    case KEY_RIGHT:
      if (inbuf[currchar])
        ++currchar;
      continue;
    }

    if (!newmail && chkmail(0))
    {
      newmail = 1;
      printchatline("◆ 噹！郵差又來了...");
    }

    if (ch == I_OTHERDATA)      /* incoming */
    {
      if (chat_recv(cfd, chatid) == -1) {
        chatting = chat_send(cfd, "/b");
        break;
      }
      continue;
    }
    if (isprint2(ch))
    {
      if (currchar < 68)
      {
        if (inbuf[currchar])
        {                       /* insert */
          int i;

          for (i = currchar; inbuf[i] && i < 68; i++);
          inbuf[i + 1 ] = '\0';
          for (; i > currchar; i--)
            inbuf[i] = inbuf[i - 1];
        }
        else
        {                       /* append */
          inbuf[currchar + 1] = '\0';
        }
        inbuf[currchar] = ch;
        move(b_lines - 1, currchar + chatid_len);
        outs(&inbuf[currchar++]);
      }
      continue;
    }

    if (ch == '\n' || ch == '\r')
    {
      if (*inbuf)
      {
        chatting = chat_send(cfd, inbuf);
        if (!strncmp(inbuf, "/b", 2))
          break;

        for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
          strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
        strcpy(lastcmd[0], inbuf);

        inbuf[0] = '\0';
        currchar = 0;
        cmdpos = -1;
      }
      print_chatid(chatid);
      move(b_lines - 1, chatid_len);
      continue;
    }

    if (ch == Ctrl('H') || ch == '\177')
    {
      if (currchar)
      {
        currchar--;
        inbuf[69] = '\0';
        memcpy(&inbuf[currchar], &inbuf[currchar + 1], 69 - currchar);
        move(b_lines - 1, currchar + chatid_len);
        clrtoeol();
        outs(&inbuf[currchar]);
      }
      continue;
    }
    if (ch == Ctrl('Z') || ch == Ctrl('Y'))
    {
      inbuf[0] = '\0';
      currchar = 0;
      print_chatid(chatid);
      move(b_lines - 1, chatid_len);
      continue;
    }

    if (ch == Ctrl('C'))
    {
      chat_send(cfd, "/b");
      break;
    }
/*
woju
*/
    if (ch == Ctrl('D'))
    {
      if (currchar < strlen(inbuf))
      {
        inbuf[69] = '\0';
        memcpy(&inbuf[currchar], &inbuf[currchar + 1], 69 - currchar);
        move(b_lines - 1, currchar + chatid_len);
        clrtoeol();
        outs(&inbuf[currchar]);
      }
      continue;
    }
    if (ch == Ctrl('K')) {
       inbuf[currchar] = 0;
       move(b_lines - 1, currchar + chatid_len);
       clrtoeol();
       continue;
    }
    if (ch == Ctrl('A')) {
       currchar = 0;
       continue;
    }
    if (ch == Ctrl('E')) {
       currchar = strlen(inbuf);
       continue;
    }
    if (ch == Ctrl('Q')) {
      print_chatid(chatid);
      move(b_lines - 1, chatid_len);
      outs(inbuf);
      continue;
    }
  }

  close(cfd);
  add_io(0, 0);
  currutmp->in_chat = currutmp->chatid[0] = 0;


  if (flog) {
     char ans[4];

     fclose(flog);
     more(fpath, NA);
     getdata(b_lines - 1, 0, "清除(C) 移至備忘錄(M) (C/M)?[C]",
        ans, 4, LCECHO,0);
     if (*ans == 'm') {
        fileheader mymail;
        char title[128];

        sethomepath(genbuf, cuser.userid);
        stampfile(genbuf, &mymail);
        mymail.savemode = 'H';        /* hold-mail flag */
        mymail.filemode = FILE_READ;
        strcpy(mymail.owner, "[備.忘.錄]");
        strcpy(mymail.title, "會議\x1b[1;33m記錄\x1b[m");
        sethomedir(title, cuser.userid);
        rec_add(title, &mymail, sizeof(mymail));
        f_mv(fpath, genbuf);
     }
     else
        unlink(fpath);
  }

  return 0;
}
