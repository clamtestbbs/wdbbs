/* Copyright 1998,1999 NTU CSIE

   You should have received a copy of the GNU General Public License
   along with PttBBS; see the file COPYRIGHT.
   If not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   voteboards' routines
*/
#define TIME_LIMIT	12*60*60

#include "bbs.h"

extern boardheader *bcache;

void
do_voteboardreply(fileheader *fhdr)
{
   char genbuf[1024];
   char reason[60];
   char fpath[80];
   char oldfpath[80];
   char opnion[10];
   char *ptr;
   FILE *fo, *fp;
   fileheader votefile;
   int len;
   int i, j;
   int fd;
   time_t endtime, now = time(NULL);
   int hastime = 0, check = 1;

   if(cuser.totaltime < TIME_LIMIT)
   {
     pressanykey("上站時數不足 , 不得參與連署");
     return;
   }

   log_usies("VB_Reply",NULL);
   clear();

   setbpath(fpath, currboard);
   stampfile(fpath, &votefile);

   setbpath(oldfpath, currboard);

   strcat(oldfpath, "/");
   strcat(oldfpath, fhdr->filename);

   fp = fopen(oldfpath, "r");

   len = strlen(cuser.userid);

   while(fgets(genbuf, sizeof(genbuf), fp))
      {
      if (!strncmp(genbuf, "連署結束時間", 12))
         {
         hastime = 1;
         ptr = strchr(genbuf, '(');
         sscanf(ptr+1, "%ld", &endtime);
         if (endtime < now){
            pressanykey("連署時間已過");
            fclose(fp);
            return;
         }
      }
      else if (!strncmp(genbuf+4, cuser.userid, len))
       {
         move(5, 10);
         prints("您已經連署過本篇了");
         opnion[0] = 'n';
         getdata(7, 0, "要修改您之前的連署嗎？(Y/N) [N]", opnion, 3,LCECHO,0);
         if (opnion[0] != 'y')
         {
            fclose(fp);
            return;
         }
         fgets(genbuf, sizeof(genbuf), fp);
         strcpy(reason, genbuf + 4);
         break;
      }
      else if (!strncmp(genbuf+27, cuser.email, strlen(cuser.email)))
      {
      	fclose(fp);
      	pressanykey("相同的E-Mail已連署過, 下次請早 ^o^");
      	return;
      }
   }
   fclose(fp);

   if((fd = open(oldfpath, O_RDONLY)) == -1)
      return;
   flock(fd, LOCK_EX);

   fo = fopen(fpath, "w");

   if (!fo)
      return;
   i = 0;
   while(fo)
   {
      j = 0;
      do
      {
        if (read(fd, genbuf+j, 1)<=0)
        {
           flock(fd, LOCK_UN);
           close(fd);
           fclose(fo);
           unlink(fpath);
           return;
        }
        j++;
      }
      while(genbuf[j-1] !='\n');
      genbuf[j] = '\0';
      i++;
      if (!strncmp("----------", genbuf, 10))
         break;
      if (i > 3)
         prints(genbuf);
      fprintf(fo, "%s", genbuf);
   }
   if (!hastime)
   {
      now += 7*24*60*60;
      fprintf(fo, "連署結束時間: (%ld)%s", now, ctime(&now));
      now -= 7*24*60*60;
   }

   fprintf(fo, "%s", genbuf);

   do{
      clear();
      if (!getdata(18, 0, "請問您 (Y)支持 (N)反對 這個議題 [C]：", opnion, 3,LCECHO,0))
      {
         flock(fd, LOCK_UN);
         close(fd);
         fclose(fo);
         unlink(fpath);
         return;
      }
   }
   while(opnion[0] != 'y' && opnion[0] != 'n');

   if (!getdata(20, 0, "請問您與這個議題的關係或連署理由為何：\n", reason,60, DOECHO,0))
    {
      flock(fd, LOCK_UN);
      close(fd);
      fclose(fo);
      unlink(fpath);
      return;
   }

   i = 0;

   while(fo)
   {
      i++;
      j = 0;
      do
      {
        if (read(fd, genbuf+j, 1)<=0)
        {
           flock(fd, LOCK_UN);
           close(fd);
           fclose(fo);
           unlink(fpath);
           return;
        }
        j++;
      }
      while(genbuf[j-1] !='\n');
      genbuf[j] = '\0';
      if (!strncmp("----------", genbuf, 10))
         break;
      if (genbuf[3] == '.' && strncmp(genbuf+4, cuser.userid, len))
      {
         fprintf(fo, "%3d.%s", i, genbuf+4);
         check = 0;
      }
      else if (check == 0 && genbuf[3] != '.')
      {
         fprintf(fo, "%s", genbuf);
         check = 1;
         i--;
      }
      else
         i--;
   }
   if (opnion[0] == 'y')
   {
      fprintf(fo, "%3d.%-15sE-Mail: %-50s\n", i, cuser.userid, cuser.email);
      fprintf(fo, "    \x1b[1;37;40m%s\x1b[m\n", reason);
   }
   i = 0;
   fprintf(fo, "%s", genbuf);
   while(fo)
   {
      i++;
      j = 0;
      do{
        if (!read(fd, genbuf+j, 1))
           break;
        j++;
        }
      while(genbuf[j-1] !='\n');
      genbuf[j] = '\0';
      if (j <= 3)
         break;
      if (genbuf[3] == '.' && strncmp(genbuf+4, cuser.userid, len))
      {
         fprintf(fo, "%3d.%s", i, genbuf+4);
         check = 0;
      }
      else if (check == 0 && genbuf[3] != '.')
      {
         fprintf(fo, "%s", genbuf);
         check = 1;
         i--;
      }
      else
         i--;
   }
   if (opnion[0] == 'n')
   {
      fprintf(fo, "%3d.%-15sE-Mail: %-50s\n", i, cuser.userid, cuser.email);
      fprintf(fo, "    \x1b[1;37;40m%s\x1b[m\n", reason);
   }
      
   flock(fd, LOCK_UN);
   close(fd);
   fclose(fo);
   unlink(oldfpath);
   f_mv(fpath, oldfpath);
}

int do_voteboard(void)
{
    fileheader votefile;
    char topic[100];
    char title[80];
    char genbuf[1024];
    char fpath[80];
    FILE* fp;
    int temp, i;
    time_t now = time(NULL);

    clear();
    if(!(currmode & MODE_POST) || cuser.totaltime < TIME_LIMIT)
    {
        pressanykey("對不起，您目前無法在此發表文章！");
        return RC_FULL;
    }
    log_usies("VB_Make",NULL);
    move(0, 0);
    clrtobot();
    prints("\n\n\n您正在使用連署系統\n");
    prints("本連署系統將詢問您一些問題，請小心回答才能開始連署\n");
    prints("任意提出連署案者，將被列入本系統不受歡迎使用者喔\n\n\n");
    prints("                    此程式原作者為Ptt的CharlieL.\n");
    pressanykey(NULL);
    move(0, 0);
    clrtobot();
    prints("(1)申請新板 (2)廢除舊板 (3)連署板主 (4)罷免板主\n");
    if (!strcmp(currboard, VOTEBOARD))
      prints("(5)連署小組長 (6)罷免小組長 ");
    if (!strcmp(currboard, VOTEBOARD) && HAS_PERM(PERM_SYSOP))
       prints("(7)站民公投");

    do
    {
      getdata(3, 0, "請輸入連署類別:", topic, 3, DOECHO,0);
      temp = atoi(topic);
    }while(temp <= 0 && temp >= 9);

    switch(temp)
    {
    case 1:
       do
       {
          if (!getdata(4, 0, "請輸入看板英文名稱：", topic, IDLEN+1,DOECHO,0))
             return RC_FULL;
          else if (invalid_brdname(topic))
             outs("不是正確的看板名稱");
          else if (getbnum(topic) > 0)
             outs("本名稱已經存在");
          else
             break;
       }while(temp > 0);
       sprintf(title, "[申請新板] %s", topic);
       sprintf(genbuf, "%s\n\n%s%-15s%s%-5d %s%-5d\n%s%s\n%s", "申請新板", "申請人: ", cuser.userid, "上站次數: ", cuser.numlogins, "發表文章: ", cuser.numposts, "英文名稱: ", topic, "中文名稱: ");
       if (!getdata(5, 0, "請輸入看板中文名稱：", topic, 20, DOECHO,0))
          return RC_FULL;
       strcat(genbuf, topic);
       strcat(genbuf, "\n看板類別: ");
       if (!getdata(6, 0, "請輸入看板類別：", topic, 20, DOECHO,0))
          return RC_FULL;
       strcat(genbuf, topic);
       strcat(genbuf, "\n板主名單: ");
       getdata(7, 0, "請輸入板主名單：", topic, IDLEN * 3 + 3, DOECHO,0);
       strcat(genbuf, topic);
       strcat(genbuf, "\n申請原因: \n");
       move(8,0);
       outs("請輸入申請原因(至多十行)，要清楚填寫不然不會核准喔");
       for(i= 9;i<19;i++)
       {
          if (!getdata(i, 0, "：", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    case 2:
       do
       {
          if (!getdata(4, 0, "請輸入看板英文名稱：", topic, IDLEN+1,DOECHO,0))
             return RC_FULL;
          else if (getbnum(topic) <= 0)
             outs("本名稱並不存在");
          else
             break;
       }
       while(temp > 0);
       sprintf(title, "[廢除舊板] %s", topic);
       sprintf(genbuf, "%s\n\n%s%s\n","廢除舊板", "英文名稱: ", topic);
       strcat(genbuf, "\n廢除原因: \n");
       outs("請輸入廢除原因(至多五行)，要清楚填寫不然不會核准喔");
       for(i= 8;i<13;i++)
       {
          if (!getdata(i, 0, "：", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;

       break;
    case 3:
       do
       {
          if (!getdata(4, 0, "請輸入看板英文名稱：", topic, IDLEN+1,DOECHO,0))
             return RC_FULL;
          else if (getbnum(topic) <= 0)
             outs("本名稱並不存在");
          else
             break;
       }
       while(temp > 0);
       sprintf(title, "[連署板主] %s", topic);
       sprintf(genbuf, "%s\n\n%s%s\n%s%s\n%s%-5d %s%-5d","連署板主", "英文名稱: ", topic, "申請 ID : ", cuser.userid, "上站次數: ", cuser.numlogins, "發表文章: ", cuser.numposts);
       strcat(genbuf, "\n申請政見: \n");
       outs("請輸入申請政見(至多五行)，要清楚填寫不然不會核准喔");
       for(i= 8;i<13;i++)
       {
          if (!getdata(i, 0, "：", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    case 4:
       do
       {
          if (!getdata(4, 0, "請輸入看板英文名稱：", topic, IDLEN+1,DOECHO,0))
             return RC_FULL;
          else if ((i = getbnum(topic)) <= 0)
             outs("本名稱並不存在");
          else
             break;
       }
       while(temp > 0);
       sprintf(title, "[罷免板主] %s", topic);
       sprintf(genbuf, "%s\n\n%s%s\n%s","罷免板主", "英文名稱: ", topic, "板主 ID : ");
       do
       {
         if (!getdata(6, 0, "請輸入板主ID：", topic, IDLEN + 1, DOECHO,0))
            return RC_FULL;
         else if (!userid_is_BM(topic, bcache[i-1].BM))
            outs("不是該板的板主");
         else
            break;
       }
       while(temp > 0);
       strcat(genbuf, topic);
       strcat(genbuf, "\n罷免原因: \n");
       outs("請輸入罷免原因(至多五行)，要清楚填寫不然不會核准喔");
       for(i= 8;i<13;i++){
          if (!getdata(i, 0, "：", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    case 5:
       if (!getdata(4, 0, "請輸入小組中英文名稱：", topic, 30, DOECHO,0))
          return RC_FULL;
       sprintf(title, "[連署小組長] %s", topic);
       sprintf(genbuf, "%s\n\n%s%s\n%s%s\n%s%-5d %s%-5d","連署小組長", "小組名稱: ", topic, "申請 ID : ", cuser.userid, "上站次數: ", cuser.numlogins, "發表文章: ", cuser.numposts);
       strcat(genbuf, "\n申請政見: \n");
       outs("請輸入申請政見(至多五行)，要清楚填寫不然不會核准喔");
       for(i= 8;i<13;i++)
       {
          if (!getdata(i, 0, "：", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    case 6:

       if (!getdata(4, 0, "請輸入小組中英文名稱：", topic, 30, DOECHO,0))
          return RC_FULL;
       sprintf(title, "[罷免小組長] %s", topic);
       sprintf(genbuf, "%s\n\n%s%s\n%s","罷免小組長", "小組名稱: ", topic, "小組長 ID : ");
       if (!getdata(6, 0, "請輸入小組長ID：", topic, IDLEN + 1, DOECHO,0))
          return RC_FULL;
       strcat(genbuf, topic);
       strcat(genbuf, "\n罷免原因: \n");
       outs("請輸入罷免原因(至多五行)，要清楚填寫不然不會核准喔");
       for(i= 8;i<13;i++)
       {
          if (!getdata(i, 0, "：", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    case 7:
       if (!HAS_PERM(PERM_SYSOP))
          return RC_FULL;
       if (!getdata(4, 0, "請輸入公投主題：", topic, 30, DOECHO,0))
          return RC_FULL;
       sprintf(title, "%s %s", "[站民公投]", topic);
       sprintf(genbuf, "%s\n\n%s%s\n","站民公投", "公投主題: ", topic);
       strcat(genbuf, "\n公投原因: \n");
       outs("請輸入公投原因(至多五行)，要清楚填寫不然不會核准喔");
       for(i= 8;i<13;i++)
       {
          if (!getdata(i, 0, "：", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    default:
       return RC_FULL;
    }
    strcat(genbuf, "\n連署結束時間: ");
    now += 7*24*60*60;
    sprintf(topic, "(%ld)", now);
    strcat(genbuf, topic);
    strcat(genbuf, ctime(&now));
    now -= 7*24*60*60;
    strcat(genbuf, "\n----------支持----------\n");
    strcat(genbuf, "----------反對----------\n");
    outs("開始連署嘍");
    setbpath(fpath, currboard);
    stampfile(fpath, &votefile);

    if (!(fp = fopen(fpath, "w")))
    {
       outs("開檔失敗，請稍候重來一次");
       return RC_FULL;
    }
    fprintf(fp, "%s%s %s%s\n%s%s\n%s%s", "作者: ", cuser.userid,
                                         "看板: ", currboard,
                                         "標題: ", title,
                                         "時間: ", ctime(&now));
    fprintf(fp, "%s\n", genbuf);
    fclose(fp);
    strcpy(votefile.owner, cuser.userid);
    strcpy(votefile.title, title);
    votefile.savemode = 'S';
    setbdir(genbuf, currboard);
    rec_add(genbuf, &votefile, sizeof(votefile));
    do_voteboardreply(&votefile);
    return RC_FULL;
}



int
all_voteboard()
{

}

#include<stdarg.h>
void
va_do_voteboardreply(va_list pvar)
{
  fileheader *fhdr;
  fhdr = va_arg(pvar, fileheader *);
  return do_voteboardreply(fhdr);
}


