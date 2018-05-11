#include "bbs.h"

char ordersongfile[120];

int orderabort()
{
  vmsg("取消點歌");
  return 0;
}

int ordersong()
     // wrapper of ordersong
{
  add_pos(POS_ALL, "KK點歌機");
  ordersong_p();
  remove_pos();
}


int ordersong_p()
{
  char userid[80], nick[20], *out, fpath[80], say[80];
  int ans, fd;
  HDR hdr;

  vs_head("KK點歌機", str_site);
  move(2, 0);
  outs("想要點給誰呢? (EMail 或是站上的 id 都可以)");
  if(!vget(3, 0, "> ", userid, 75, DOECHO))
    return orderabort();

  out = strchr(userid, '@');
  if(!out) {
    if(!acct_userno(userid)) {
      vmsg("沒有這個人哦, 放棄點歌");
      return 0;
    }
  } else
    *out = '\0';

  str_ncpy(nick, userid, 20);

  if(!vget(5, 0, "對方的暱稱是什麼呢? ", nick, 20, GCARRY))
    str_ncpy(nick, userid, 20);

  if(out)
    *out = '@';

  move(7, 0);
  outs("想要對他(她)說什麼話呢?");
  vget(8, 0, "> ", say, 75, DOECHO);

  ans = vans("請問要不要放上動態看板? (Y: 要  n: 不要) [Y] ");

  vmsg("要點歌了, 請按任意一鍵進入點歌目錄");
  if(!XoOrderSong("gem/brd/KKCity/Song/@/@songs", "KK 點歌系統"))
    return orderabort();

  usr_fpath(fpath, cuser.userid, fn_note);
  {
    FILE *from, *to;
    char buf[512], buf2[1024];
    int i = 0, j, k;

    from = fopen(ordersongfile, "r");
    to = fopen(fpath, "w");

    while(i <= 11 && fgets(buf, 512, from)) {
      j = k = 0;
      while(k < 512) {
	if(buf[j] == '<') {
	  register char *tmp = NULL;
	  if(!str_ncmp(&buf[j], "<~src~>", 7))
	    tmp = cuser.userid;
	  else if(!str_ncmp(&buf[j], "<~des~>", 7))
	    tmp = nick;
	  else if(!str_ncmp(&buf[j], "<~say~>", 7))
	    tmp = say;
	  if(tmp) {
	    j += 7;
	    while(*tmp)
	      buf2[k++] = *tmp++;
	    continue;
	  }
	}
	buf2[k++] = buf[j++];
      }
      buf2[k] = 0;
      fputs(buf2, to);
    }
    fclose(to);
    fclose(from);
  }

  if(ans != 'n') {
    hdr_stamp("gem/brd/KKCity/Song/@/@ordered", HDR_LINK | 'A', &hdr, fpath);
    sprintf(hdr.title, "%s 點給 %s", cuser.userid, nick);
    strcpy(hdr.owner, cuser.userid);
    rec_add("gem/brd/KKCity/Song/@/@ordered", &hdr, sizeof(HDR));
  }

  if(out) {
    char title[80];
    sprintf(title, "%s 點給您的歌", cuser.userid);
    outs("請稍候, 點歌傳遞中...\n");
    refresh();
    fd = bsmtp(fpath, title, userid, 0);
    if (fd < 0)
      vmsg("點歌內文無法寄達");
    mail_hold(fpath, userid);
  } else {
    char folder[80];

    usr_fpath(folder, userid, fn_dir);
    hdr_stamp(folder, HDR_LINK, &hdr, fpath);
    strcpy(hdr.owner, cuser.userid);
    strcpy(hdr.nick, cuser.username);    /* :chuan: 加入 nick */
    sprintf(hdr.title, "%s 點給您的歌", cuser.userid);
    fd = rec_add(folder, &hdr, sizeof(hdr));
    if (!fd) {
      mail_hold(fpath, folder);
    }
  }
  usr_fpath(fpath, cuser.userid, fn_note);
  unlink(fpath);
  vmsg("點歌完成");

  return 0;
}
