#include "bbs.h"
#include "rpg.h" 


/* ¤jÀY·Ó */
void
showpicture(char *uid)
{
  char genbuf[200];

  sethomefile(genbuf, uid, "picture");
  if (!show_file(genbuf, 11, 12, ONLY_COLOR))
    prints("[1;32m¡i¤jÀY·Ó¡j[m%s ¥Ø«e¨S¦³¤jÀY·Ó", uid);
}

int
u_editpic()
{
  char genbuf[200];
  stand_title("½s¿è¤jÀY·Ó");
  showpicture(cuser.userid);
  getdata(b_lines - 1, 0, "¤jÀY·Ó (D)§R°£ (E)½s¿è [Q]¨ú®ø¡H[Q] ", genbuf, 3, LCECHO,0);

  if (genbuf[0] == 'e')
  {
    int aborted;

    setutmpmode(EDITPIC);
    setuserfile(genbuf, "picture");
    aborted = vedit(genbuf, NA);
    if (aborted != -1)
    pressanykey("¤jÀY·Ó§ó·s§¹²¦");
    return 0;
  }
  else if (genbuf[0] == 'd')
  {
    setuserfile(genbuf, "picture");
    unlink(genbuf);
    outmsg("¤jÀY·Ó§R°£§¹²¦");
  }
  return 0;
}

/* ­pºâ¦U¶µ§ğ¨¾­È */

int
c_att(char *id)
{
  int i;
  getuser(id);
  i = (((rpgtmp.str*10)+(rpgtmp.dex*7)+(rpgtmp.kar*5))/3);
  return i;
}

int
c_def(char *id)
{
  int i;
  getuser(id);
  i = (((rpgtmp.con*10)+(rpgtmp.dex*7)+(rpgtmp.kar*5))/3);
  return i;
}

int
c_ma(char *id)
{
  int i;
  getuser(id);
  i = (((rpgtmp.wis*10)+(rpgtmp.kar*5))/2);
  return i;
}

int
c_md(char *id)
{
  int i;
  getuser(id);
  i = (((rpgtmp.con*10)+(rpgtmp.dex*7)+(rpgtmp.wis*7)+(rpgtmp.kar*5))/4);
  return i;
}

int
rpg_udisplay(char *id)
{
  int MHP,MMP,Att,MA,Def,MD;
  if(!getuser(id)) return 0;
  MHP = rpgtmp.con*30;
  MMP = rpgtmp.wis*10;
  Att = c_att(id);
  Def = c_def(id);
  MA = c_ma(id);
  MD = c_md(id);

  setutmpmode(RINFO);
  clear();
  outs(COLOR1"[1m ¡³ùùùùùùùùùùùùùùùùùùùùùùùùùù ¨Ï¥ÎªÌ¡@¢à¢Ş¢Õ¡@¸ê®Æ ùùùùùùùùùùùùùùùùùùùùùùùùùù¡³ [m\n");
  prints("[1;32m¡i©m¦W¡j[m%-16.16s[1;32m¡iÂ¾·~¡j[m%-15.15s[1;32m¡iµ¥¯Å¡j[m%-5d[1;32m¡i¸gÅç­È¡j[m%d\n"
       ,rpgtmp.userid,rname[rpgtmp.race],rpgtmp.level,xuser.exp);
  prints("[1;32m¡i¦~ÄÖ¡j[m%6d ·³  [1;32m¡iÅé¤O¡j[m%6d/%-6d [1;32m¡iªk¤O¡j[m%6d/%-6d\n",
       (rpgtmp.age/2880)+10,rpgtmp.hp,MHP,rpgtmp.mp,MMP);
  prints("[1;32m¡iÄİ©Ê¡j[m[1;33m¤O¶q(STR)[m%-3d [1;33m´¼¼z(WIS)[m%-3d\
 [1;33m±Ó±¶(DEX)[m%-3d [1;33mÅé½è(CON)[m%-3d [1;33m¹B®ğ(KAR)[m%-3d\n"
       ,rpgtmp.str,rpgtmp.wis,rpgtmp.dex,rpgtmp.con,rpgtmp.kar);
prints("[1;32m¡i§ğÀ»¤O¡j[31m ¤@¯ë [m%3d [1;36mÅ]ªk[m %3d    \
[1;32m¡i¨¾¿m¤O¡j[31m ¤@¯ë [m%3d [1;36mÅ]ªk[m %3d\n"
       ,Att,MA,Def,MD);
  outs("\n\n\n\n");
  outs(COLOR1"[1m ¡³ùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù¡³ [m\n");
  showpicture(rpgtmp.userid);
  pressanykey(NULL);
  return 1;
}

void
rpg_uinfo()
{
  rpg_udisplay(cuser.userid);
  return;
}

void
rpg_race_c()
{
  int strlv1[7] = {0,5,4,3,5,4,4};
  int wislv1[7] = {0,4,5,4,3,3,4};
  int dexlv1[7] = {0,4,4,4,5,5,4};
  int conlv1[7] = {0,4,4,5,3,4,3};
  int karlv1[7] = {0,3,3,4,4,4,5};
  int ans;
  char buf[2],ans2[2];

  setutmpmode(RCHOOSE);
  more(BBSHOME"/game/rpg/choose_race",YEA);
  stand_title("¬D¿ïÂ¾·~");
  if(rpguser.race)
  {
    move(2,4);
    outs("§A¤w¸g¦³Â¾·~Åo,Âà´«Â¾·~·|±qµ¥¯Å¤@­«·s¶}©l­ò¡I");
    getdata(10,5,"¬O§_½T©w¡H (y/N)",buf,3,DOECHO,"n");
    if(!buf[0] || buf[0] == 'n' || buf[0] == 'N') return;
  }
  move(4,0);
  outs(       "¬D¿ïÂ¾·~¡G (1)Äé¤ô±j¤H (2)Åª¤å§Ö¤â (3)±`¾nµ{¦¡");
  getdata(5,0,"           (4)²á¤Ñ²r±N (5)¤ô²y«a­x (6)¹CÀ¸«g¬P (0)©ñ±ó",buf,3,DOECHO,0);
  if(!buf[0] || buf[0] < '1' || buf[0] > '7') return;
  else ans = atoi(buf);
  if(ans == 7) ans =0;
  clear();
  prints("\n§A¿ï¾Ü¤F %s ¡I ¨ä¦U¶µ¼Æ­È¦p¤U¡G\n",rname[ans]);
  prints("ùúùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùû\n");
  prints("ùø      µ¥¯Å¤@®É¦U¶µ¼Æ­È        ùø\n");
  prints("ùüùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùı\n");
  prints("  ¤O¶q¡G%d\n",strlv1[ans]);
  prints("  ´¼¼z¡G%d\n",wislv1[ans]);
  prints("  ±Ó±¶¡G%d\n",dexlv1[ans]);
  prints("  Åé½è¡G%d\n",conlv1[ans]);
  prints("  ¹B®ğ¡G%d\n",karlv1[ans]);
  prints("  Åé¤O¡G%3d       ªk¤O¡G%d\n",conlv1[ans]*30,wislv1[ans]*10);
  prints("  ¤@¯ë§ğÀ»¤O¡G%3d ¤@¯ë¨¾¿m¤O¡G%d\n",
  ((strlv1[ans]*10)+(dexlv1[ans]*7+karlv1[ans]*5))/3,((conlv1[ans]*10)+(dexlv1[ans]*7)+(karlv1[ans]*5))/3);
  prints("  Å]ªk§ğÀ»¤O¡G%3d Å]ªk¨¾¿m¤O¡G%d\n",
  ((wislv1[ans]*10)+(karlv1[ans]*5))/2,((conlv1[ans]*7)+(dexlv1[ans]*5)+(wislv1[ans]*5)+(karlv1[ans]*3))/4);

  getdata(14,5,"¬O§_½T©w¡H (y/N)",ans2,2,LCECHO,0);
  if(ans2[0] == 'y' || ans2[0] == 'Y')
  {
    degold(5);
    rpguser.race = ans;
    rpguser.level = 1;
    rpguser.str=strlv1[ans];
    rpguser.wis=wislv1[ans];
    rpguser.dex=dexlv1[ans];
    rpguser.con=conlv1[ans];
    rpguser.kar=karlv1[ans];
    rpguser.hp=conlv1[ans]*30;
    rpguser.mp=wislv1[ans]*10;
    rpg_rec(cuser.userid,rpguser);
    rpg_uinfo();
  }
  else
    rpg_race_c();
}

/* Â¾·~¤u·| */

rpg_guild()
{
  char ans[5],buf[200];
  int l = rpguser.level;   
  update_data();
  if (!rpguser.race)
  {
    pressanykey("±z¨S¦³Â¾·~¡A½Ğ¥Ñ Join ¥[¤J¥»¹CÀ¸");
    return 0;
  }
  setutmpmode(RGUILD);
  clear();
  prints("[1m[33;42m¡i %s ¦P·~¤u·|¡j[m  §A²{¦bµ¥¯Å¬° %d¡A¦³¸gÅç­È %d ÂI\n\n"
    ,rname[rpguser.race],rpguser.level,cuser.exp);
  prints("\
    ([1;36m0[m)´_¬¡            ´_¬¡·|´î¥h©Ò¦³Äİ©Ê¤@ÂI,¸gÅç­È¦©¤@¥b
    ([1;36m1[m)¤É¯Å            ¤É¯Å¶·¸gÅç­È %d
    ([1;36m2[m)«ì´_Åé¤O        Åé¤O³Ñ %d¡A¥[º¡¶·ªá¶O %d ¤¸
    ([1;36m3[m)«ì´_ªk¤O        ªk¤O³Ñ %d¡A¥[º¡¶·ªá¶O %d ¤¸
    ([1;36m4[m)´£ª@Äİ©Ê        ±zªº¦U¶µÄİ©Ê¤W­­¬° %d ÂI(¨CÂI»İ %d ¸gÅç­È)
    ([1;36m5[m)¸ê¾ú¬d¸ß¤¤¤ß    ¬d¸ß§O¤H¸ê®Æ¡A¨C¦¸ 1000 exp
    ([1;36m6[m)·Ó¹³À]          ¥»À]¥u­t³d¬~·Ó¤ù¡A©³¤ù½Ğ¦Û¦æ´£¨Ñ"
  ,LVUP,rpguser.hp,((rpguser.con*30)-rpguser.hp)*10
  ,rpguser.mp,((rpguser.wis*10)-rpguser.mp)*20,l*3,LVUP/10);
  getdata(10,0,"½Ğ¿ï¾Ü§A»İ­nªºªA°È¡G ",ans,3,LCECHO,0);
  if(!ans[0]) return 0;
  switch(ans[0])
  {
    default:
      break;
    case '!':
      if(HAS_PERM(PERM_SYSOP))
      {
        rpguser.hp = rpguser.con*30;
        rpguser.mp = rpguser.wis*10;
      }
      break;  
    case '0':
      if(rpguser.hp > 0)
      {
        pressanykey("ºÆ°Õ¡H¨S¦ºÁÙ¨Ó´_¬¡¡I");
        break;
      }
      update_data();
      if(rpguser.str > 1)
        rpguser.str -= 1;
      if(rpguser.wis > 1)
        rpguser.wis -= 1;
      if(rpguser.dex > 1)
        rpguser.dex -= 1;
      if(rpguser.con > 1)
        rpguser.con -= 1;
      if(rpguser.kar > 1)
        rpguser.kar -= 1;
      cuser.exp /= 2;
      rpguser.hp = rpguser.con * 15;
      {
        time_t now = time(0);
        sprintf(buf," %s ªá¥h¤F %d ÂI¸gÅç­È¡A±q¦º¤`ªºÃä½t³Q©ì¤F¦^¨Ó¡C%s"
          ,cuser.userid,cuser.exp,Cdate(&now));
        f_cat(BBSHOME"/game/rpg/resurge.log",buf);
      }
      sleep(5);
      pressanykey("¤@¹D¥ú¨~Å¢¸n¦b§A¨­Åéªº©P³ò,§AºCºC¦³¤Fª¾Ä±....");  
      break; 
    case '1':
    {
      if(rpguser.hp <= 0)
      {
        pressanykey("§A¤w¸g°}¤`¤F,­n¥ı´_¬¡¤~¯à¤É¯Å");
        break;
      }
      if(rpguser.level >= 99)
      {
        pressanykey("§A¤w¸g¤É¨ì¥Ø«eªº³Ì°ªµ¥¯Å¤F");
        break;
      }  
      if(check_exp(LVUP)) break;
      else deexp(LVUP);
      rpguser.str += lvup[0][rpguser.race];
      rpguser.wis += lvup[1][rpguser.race];
      rpguser.dex += lvup[2][rpguser.race];
      rpguser.con += lvup[3][rpguser.race];
      rpguser.kar += lvup[4][rpguser.race];
      rpguser.hp = rpguser.con*30;
      rpguser.mp = rpguser.wis*10;
      if(rpguser.str > (rpguser.level*3)+5)
        rpguser.str = (rpguser.level*3)+5;
      if(rpguser.dex > (rpguser.level*3)+5)
        rpguser.dex = (rpguser.level*3)+5;
      if(rpguser.wis > (rpguser.level*3)+5)
        rpguser.wis = (rpguser.level*3)+5;
      if(rpguser.con > (rpguser.level*3)+5)
        rpguser.con = (rpguser.level*3)+5;
      if(rpguser.kar > (rpguser.level*3)+5)
        rpguser.kar = (rpguser.level*3)+5;
      rpguser.level++;
      substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
      rpg_rec(cuser.userid,rpguser);
      pressanykey("µ¥¯Å´£¤É¦Ü %d",rpguser.level);
      rpg_uinfo();
    }
    break;

    case '2':
      if(rpguser.hp <= 0)
      {
        pressanykey("§A¤w¸g°}¤`¤F,­n¥ı´_¬¡¤~¯à¸ÉÅé¤O");
        break;
      }  
      if(check_money(((rpguser.con*30)-rpguser.hp)*10,SILVER)) return 0;
      demoney(((rpguser.con*30)-rpguser.hp)*10);
      rpguser.hp = rpguser.con*30;
      pressanykey("Åé¤O«ì´_¦Ü¥şº¡ª¬ºA¡I");
      break;

    case '3':
      if(check_money(((rpguser.wis*10)-rpguser.mp)*20,SILVER)) return 0;
      demoney(((rpguser.wis*10)-rpguser.mp)*20);
      rpguser.mp = rpguser.wis*10;
      pressanykey("ªk¤O«ì´_¦Ü¥şº¡ª¬ºA¡I");
      break;

    case '4':
    {
      char ans2[3],genbuf[100] = "¿é¤J¿ù»~©Î¬O¤w¹F¨ì³Ì¤j­È";
      if(rpguser.hp <= 0)
      {
        pressanykey("§A¤w¸g°}¤`¤F,­n¥ı´_¬¡¤~¯à´£ª@Äİ©Ê");
        break;
      }  
      getdata(15,0,
        "­n´£ª@ : [1]¤O¶q [2]´¼¼z [3]±Ó±¶ [4]Åé½è [5]¹B®ğ ?",ans2,2,LCECHO,0);
      if(ans2[0] < '1' || ans2[0] > '5') break;
      if(check_exp(LVUP/10)) break;
      switch(ans2[0])
      {
        case '1':
          if(rpguser.str < rpguser.level*3)
          {
            deexp(LVUP/10);
            sprintf(genbuf,"¤O¶q´£¤É¬° %d ÂI",++rpguser.str);
          }
          break;
        case '2':
          if(rpguser.wis < rpguser.level*3)
          {
            deexp(LVUP/10);
            sprintf(genbuf,"´¼¼z´£¤É¬° %d ÂI",++rpguser.wis); 
          }
          break;
        case '3':
          if(rpguser.dex < rpguser.level*3)
          {        
            deexp(LVUP/10);
            sprintf(genbuf,"±Ó±¶´£¤É¬° %d ÂI",++rpguser.dex); 
          }
          break;
        case '4':
          if(rpguser.con < rpguser.level*3)
          {        
            deexp(LVUP/10);
            sprintf(genbuf,"Åé½è´£¤É¬° %d ÂI",++rpguser.con);
          }
          break;
        case '5':
          if(rpguser.kar < rpguser.level*3)
          {        
            deexp(LVUP/10);
            sprintf(genbuf,"¹B®ğ´£¤É¬° %d ÂI",++rpguser.kar);
          }
          break;
      }
      pressanykey(genbuf);
      break;  
    }        

    case '5':
    {
      char genbuf[20];
      if(check_exp(1000)) break;
      stand_title("¬d¸ß§O¤Hªº¸ê®Æ");
      usercomplete(msg_uid,genbuf);
      if(genbuf[0])
      {
        deexp(1000);
        rpg_udisplay(genbuf);
      }
    }
    break;

    case '6':
      u_editpic();
      break;

  }
  substitute_record(fn_passwd,&cuser,sizeof(userec),usernum);
  rpg_rec(cuser.userid,rpguser);
  rpg_guild();
}

void
rpg_top()
{
  int i; 
  char buf[5],genbuf[200];
  stand_title("RPG ±Æ¦æº]");
  setutmpmode(RTOPLIST); 
  move(4,0); 
  outs(       "­n¬İ¤°»òÂ¾·~ªº±Æ¦æ¡G(1)Äé¤ô±j¤H (2)Åª¤å§Ö¤â (3)±`¾nµ{¦¡");
  getdata(5,0,"(t)²Î­p  (a)¥ş³¡    (4)²á¤Ñ²r±N (5)¤ô²y«a­x (6)¹CÀ¸«g¬P",buf,2,DOECHO,0);
  if(buf[0] == 'a' || buf[0] == 'A') buf[0] = '7';
  if(buf[0] == 't' || buf[0] == 'T') buf[0] = '8';
  if(!buf[0] || buf[0] < '0' || buf[0] > '8') return;
  i = atoi(buf);
  sprintf(genbuf,"%s/bin/toprpg %d %d %s/log/toprpg%d",
    BBSHOME, i, i == 7 ? 100 : 50, BBSHOME, i);
  system(genbuf);
  sprintf(genbuf,BBSHOME"/log/toprpg%d",i);  
  more(genbuf); 
}

/* ½m¥\ */
int
rpg_kmob(int mode)
{
  mobattr m; 
  int MHP = rpguser.con*30,MMP= rpguser.wis*10;  /* ¦Û¤vªº³Ì¤jmp,hp */
  int Att = c_att(cuser.userid);  /* §ğÀ»¤O */
  int Def = c_def(cuser.userid);  /* ¨¾¿m¤O */
  int MA = c_ma(cuser.userid);    /* Å]ªk§ğÀ» */
  int MD = c_md(cuser.userid);    /* Å]ªk¨¾¿m */
  int money,exp;  /* ¥´§¹«á·|¼W¥[or´î¤Öªº$$¸òexp */
  char ans[4],buf[256]= "\0"; /* ansµ¹user input¥Î,bufµ¹¿Ã¹õoutput¥Î */
  int j=0,k=0,l=0,over=0,attack=0; /* overµ²§ô¾Ô°«,attack¼Ä¤H§ğÀ» */

  if(rpguser.hp <= 0)
  {
    pressanykey("§A¤w¸g°}¤`¤F¡ã¡ã¡ã");
    return 0;
  }
//  if(lockutmpmode(RTRAIN)) return 0;  /* ¤£Åıuser multi ª± */
  setutmpmode(RTRAIN); 
  if(mode == hard)
    m.LV = rpguser.level + rand()%10;
  if(mode == medium)
    m.LV = rand()%2 ? rpguser.level + rand()%3 : rpguser.level - rand()%3;
  if(mode == easy)
    m.LV = rand()%3 < 1 ? rpguser.level : rpguser.level - rand()%5;
  if(m.LV < 1) m.LV = 1;  
  m.maxhp = m.hp = m.LV * 100;
  m.Att = m.LV * (10 + rand()%15);
  m.Def = m.LV * (5 + rand()%10);
  m.MA = m.LV * (20 + rand()%5);
  m.MD = m.LV * (5 + rand()%5); 
  m.EXP = (m.LV*m.LV + mode) * ((rand()%5) + 5);
  m.money = (m.LV*m.LV + mode) * ((rand()%2) + 5);
  while(!over){
    update_data();
    if(mode == easy)  
      strcpy(m.name,"¦×¸}©Çª«");
    else if(mode == medium)
      strcpy(m.name,"¤@¯ë©Çª«");
    else
      strcpy(m.name,"ÅÜºA©Çª«");
    clear();
    if(!HAS_PERM(PERM_SYSOP))
    {
      show_file(buf,1,10,ONLY_COLOR);  /* ¼Ä¤H¹Ï¤ù */
      move(1,0); 
      prints("µ¥¯Å : %d",m.LV);
    }
    else
    {
      move(1,0); 
      prints(
"µ¥¯Å : %d\n¥Í©R : %d\n§ğ¨¾ : %d / %d\nÅ]ªk : %d / %d\n¸gÅç : %d\n¼úª÷ : %d"
,m.LV,m.maxhp,m.Att,m.Def,m.MA,m.MD,m.EXP,m.money);
    } 
    move(11,0);
    prints("[36m%s[m",msg_seperator);  /* ¤À¹j½u */
    move(13,0);
    prints("[36m%s[m",msg_seperator);  /* ¤À¹j½u */
    over = 0;
    while(over == 0 && rpguser.hp > 0 && m.hp >0)  /* ¾Ô°«...ª½¨ì over */
    {
      /* ¤£Åıuserª¾¹D©Çª«¦³¦h¤Ö¦å¡A©Ò¥H¥Î¨­Åéª¬ºAªí¥Ü */
      j = (m.hp/(m.maxhp/100) >= 100 ? 0 :
           m.hp/(m.maxhp/100) >= 75  ? 1 :
           m.hp/(m.maxhp/100) >= 50 ? 2 :
           m.hp/(m.maxhp/100) >= 25 ? 3 : 4);   
      move(0,0);
      clrtoeol();
      if(!HAS_PERM(PERM_SYSOP))
        prints("[1m[33;46m  ¡i ¾Ô °« ¤¤ ¡j                       \
[44m¡º ¼Ä¤H¡G %8s [40m°·±dª¬ºA¡G %s [m",m.name,health[j]);
      else
        prints("[1m[33;46m  ¡i ¾Ô °« ¤¤ ¡j          \
[44m¡º ¼Ä¤H¡G %8s [40m¥Í©RÂI¼Æ¡G%d/%d [m",m.name,m.hp,m.maxhp);
            move(14,0);
      prints("¡i¦Û¤vªºª¬ºA¡j
 ¡³ùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùŞùùùùùùùùùùùùùùùùùùùùùùùù¡³
 ùø¡iÅé  ¤O¡j %-4d / %4d  ¡iªk  ¤O¡j %-4d / %4d  ùø ¡i¸Ë³ÆªZ¾¹¡j%-8s   ùø
 ùø¡i§ğÀ»¤O¡j ¤@¯ë %4d  Å]ªk %4d                 ùø ¡i¸g Åç ­È¡j%-10d ùø
 ùø¡i¨¾¿m¤O¡j ¤@¯ë %4d  Å]ªk %4d                 ùø ¡i¨­¤W²{ª÷¡j%-10d ùø
 ¡³ùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùäùùùùùùùùùùùùùùùùùùùùùùùù¡³
      ",rpguser.hp,MHP,rpguser.mp,MMP,"ªÅµL¤@ª«",
        Att,MA,cuser.exp,Def,MD,cuser.silvermoney);
      sprintf(buf,"[1m[46;33m\
 (1)[37m¤@¯ë§ğÀ» [33m(2)[37m¨Ï¥ÎÅ]ªk [33m(3)[37m¼W±j¨¾¿m\
 [33m(4)[37m¨Ï¥Î¹D¨ã [33m(5)[37m§ë­° [33m(6)[37m°k¶] [33m(7)[37m¨Ï¥ÎªÌ¸ê®Æ [m");
      move(20,0);
      outs(buf);
      getdata(22,0,"­n°µ¤°»ò©O¡H",ans,3,DOECHO,0);

      switch(ans[0])
      {
        default:
          attack = 0;
        break;

        case '!':
          if(HAS_PERM(PERM_SYSOP))
            m.hp -= 10000;  
          break;   
        case '4':
          pressanykey("¬I¤u¤¤");
          attack = 0;
          break;

        /* ¤@¯ë§ğÀ» */
        case '1':
          j = (rpguser.kar*10)/((rand()%rpguser.kar*2)+2);
          if(j > 50)
          {
            sprintf(buf," [1;31m§A¥Î¾ğªKÂW°_¤@ªû«K«K¡A¥Î¤O©¹ [33m%s [31m¯{¹L¥h¡I¡I[m ",m.name);
            j = 5;
          }
          else if(j > 35)
          {
            sprintf(buf," [1;31m§A§V¤Oªº¦b¨­¤W·b·b....·b¥X¤@­Ó¶Â¤Y¤l¡A¥Î¤O©¹ [33m%s [31m¥á¹L¥h¡I¡I[m ",m.name);
            j = 3;
          }
          else if(j > 20)  /* ¹B®ğ¦n¨Ï¥X¤jµ´©Û */
          {
            sprintf(buf," [1;31m§A»Eºë·|¯«¡A§V¤O¿n¤F¤@¤f·ğ©¹ [33m%s [31m¦R¥h¡I¡I[m ",m.name);
            j = 2;
          }
          else if(j <= 20)
          {
            sprintf(buf," [1;37m§A¬½¬½ªº©¹ [33m%s [37mªº [31m%s [37m¥´¤U¥h¡I¡I[m "
            ,m.name,body[rand()%5]);
            /* ¶Ã¼Æ¶]§ğÀ»³¡¦ì */
            j = 1;
          }
          move(12,0);
          outs(buf);
          k = (rand()%((Att-m.Def)>=5 ? Att - m.Def : 5))*j;
          sprintf(buf," %s ¥¢¦å %d ÂI¡C",m.name,k);
          m.hp -= k;
          pressanykey(buf);
          attack = (m.hp > 0 ? 1 : 0);  /* ¼Ä¤H±¾¤F¤£·|¦A¤ÏÀ» */
          break;

        /* ¨Ï¥ÎÅ]ªk */
          case '2':
          sprintf(buf,"[1m[33;46m\
 (1)[37m§ğÀ»Å]ªk [33m(2)[37mªvÀøÅ]ªk [33m(3)[37m¨g¼ÉÅ]ªk[33m\
 (4)[37m¥Û¤ÆÅ]ªk [33m(5)[37m¶Ç°eÅ]ªk [33m(0)[37m¦^¤W¼h¿ï³æ    [m");
          move(20,0);
          prints("%79s"," ");
          move(20,0);
          outs(buf);
          getdata(22,0,"­n°µ¤°»ò©O¡H",ans,3,DOECHO,0);
          switch(ans[0])
          {
            default:
              attack = 0;
              break;

            case '1':
              if(rpguser.mp >= 20)
              {
                rpguser.mp -= 20;
                move(12,0);
                prints(" [1;32m§A¹ïµÛ [33m%s [36mØpØp¡ã[32mªº¥s­Ó¤£°±...[33m%s Å¥ªº³£§Ö·w¥h¤F...[m",m.name,m.name);
                k = rand()%((MA-m.MD)>=30 ? MA - m.MD : 30);
                sprintf(buf," %s ¥¢¦å %d ÂI¡C",m.name,k);
                m.hp -= k;
                attack = (m.hp > 0 ? 1 : 0);  /* ¼Ä¤H±¾¤F¤£·|¦A¤ÏÀ» */
              }
              else
              {
                sprintf(buf,"ªk¤O¤£°÷¡I");
                attack = 0;
              }
              pressanykey(buf);
              break;

            case '2':
              if(rpguser.mp >= 30 && rpguser.hp < MHP)
              {
                rpguser.mp -= 30;
                rpguser.hp += MHP/5;
                if(rpguser.hp > MHP)
                  rpguser.hp = MHP;
                move(12,0);
                outs(" [1;32m§A³ä³ä©À¹D¡G¡u[33mªü©ÔÄÀ­{¥É¬Ó¤j«Ò½ĞªvÀø§Úªº¶Ë¤f¡Aªüªù¡I[32m¡v  [m");
                sprintf(buf,"Åé¤O«ì´_¦Ü %d ÂI",rpguser.hp);
                attack = 1;
              }
              else
              {
                if(rpguser.hp >= MHP)
                  sprintf(buf,"§A²{¦bºë¤O¥R¨K¡A¤£»İ­nªvÀø");
                if(rpguser.mp < 30)
                  sprintf(buf,"ªk¤O¤£°÷¡I");
                attack = 0;
              }
              pressanykey(buf);
              break;

            case '3':
              if(rpguser.mp >= 30)
              {
                rpguser.mp -= 30;
                move(12,0);
                outs(" [1;32m±q§A¤â´x¤¤ÅÜ¥X´X²É[36m«Â¦Ó¿û[32m¡A§A¤G¸Ü¤£»¡°¨¤W§]¤U¥h...[m");
                if(Att >= (c_att(cuser.userid)*3/2)) /* ¤W­­ */
                {
                   sprintf(buf,"§Aªº§ğÀ»¤O¤w¸g´£¤É¨ì·¥­­¤F");
                   attack = 0;  /* ¨S¤É¨ì¼Ä¤H¤£§ğÀ» */
                }
                else
                {
                  Att += rand()%((rpguser.kar*2+rpguser.con*3+rpguser.dex*5)/10);
                  sprintf(buf,"§Aªº§ğÀ»¯à¤O´£¤É¨ì %d ÂI¡C",Att);
                  attack = 1;  /* ´£¤É«á¼Ä¤H¥ı¥´¤@¤U */
                }
              }
              else
              {
                sprintf(buf,"ªk¤O¤£°÷¡I");
                attack = 0;
              }
              pressanykey(buf);
              break;

            case '4':
              if(rpguser.mp >= 10)
              {
                rpguser.mp -= 10;
                move(12,0);
                prints(" §A¤f¤¤³ä³ä©À©G¡AÅÜ¥X¤@ÅøºÆ¨gÀş¶¡½¦¡A©¹ %s ¨­¤W¼â¹L¥h",m.name);
                if((rand()%MD) - m.MD > 0)
                {
                  sprintf(buf,"%s ³Q¼â¨ì¤F¡I©w¦b¨ºÃä¤@°Ê¤]¤£¯à°Ê...",m.name);
                  l += 3;
                }
                else
                {
                  sprintf(buf,"%s ®³¥X¤@­Ó¤ô±í¡A§âÀş¶¡½¦±µ¦í¤F...",m.name);
                  attack = 1;
                }
              }
              else
              {
                sprintf(buf,"ªk¤O¤£°÷¡I");
                attack = 0;
              }
              pressanykey(buf);
              break;

            case '5':
              if(rpguser.mp >= 50)
              {
                rpguser.mp -= 50;
                move(12,0);
                prints(" [1;32m§A²´¬İ¥´¤£¹L [33m%s [32m¤F¡A¤j³Û¤@Án¡G[31m´cÆF°h´²¡I¡I  [m",m.name);
                pressanykey(" §Aªº¸}©³¤U¬ğµM¾h°_¤@°}©Ç­·¡A§â§A§j¨«¤F...");
                rpg_guild();
                unlockutmpmode();
                return 0;
              }
              else
                pressanykey("ªk¤O¤£°÷¡I");
              attack = 0;
              break;
          }
          break;

      /* ´£¤É¨¾¿m¤O */
        case '3':
          move(12,0);
          outs(" [1;33m§A¤j³Û¡G¡u¸U¯àªº¤Ñ¯«¡A½Ğ½ç¤©§Ú¯«©_ªº¤O¶q....¡v[m");
          if(Def >= (c_def(cuser.userid)*3/2)) /* ¤W­­ */
          {
            sprintf(buf,"§Aªº¨¾¿m¤O¤w¸g´£¤É¨ì·¥­­¤F");
            attack = 0;  /* ¨S¤É¨ì¼Ä¤H¤£§ğÀ» */
          }
          else
          {
            Def += (rand()%(rpguser.kar*2+rpguser.con*3+rpguser.dex*5))/10;
            sprintf(buf,"§Aªº¨¾¿m¯à¤O´£¤É¨ì %d ÂI¡C",Def);
            attack = 1;  /* ´£¤É«á¼Ä¤H¥ı¥´¤@¤U */
          }
          pressanykey(buf);
          break;

        /* §ë­° */
        case '5':
          j=(rand()%rpguser.kar) - rand()%m.LV;  /* ¸I¹B®ğ§a :p */
          move(12,0);
          prints("[1;35m§A¸÷¦b %s «e­±»¡¡G¡u§Ú¦³²´¤£ÃÑ®õ¤s¡A«ô°UÄÇ¤F§Ú¤@©R§a¡I¡v[m",m.name);
          money = m.LV*100/((rand()%rpguser.kar/5)+1);
          if(j > 0 && cuser.silvermoney >= money)
          {
            sprintf(buf,"%s ¨M©w©ñ§A¤@°¨¡A¦¬¤F§A %d ¤¸ªº«OÅ@¶O¡C",m.name,money);
            demoney(money);
            over = 1;
            attack = 0;
          }
          else
          {
            sprintf(buf,"%s Ä±±oÁÙ¬O§â§A±ş¤F¤ñ¸û§Ö¡I",m.name);
            attack = 1;
          }
          pressanykey(buf);
          break;

        /* ¹Á¸Õ°k¶] */
        case '6':
          j=(rand()%rpguser.kar) - m.LV;  /* ¸I¹B®ğ§a :p */
          move(12,0);
          outs(" [1;32m§A¹Á¸Õ°k¶]....[m");
          if(j > 0)
          {
            money = m.LV*50/((rand()%rpguser.kar)+1);
            sprintf(buf,"°k¶]¦¨¥\\¡A¦ı¬OÅå·W¤¤±¼¤F %d ¤¸¡C",money);
            demoney(money);
            over = 1;
            attack = 0;
          }
          else
          {
            attack = 1;  /* °k¶]¥¢±Ñ¼Ä¤H¸É¤@®± */
            sprintf(buf,"°k¶]¥¢±Ñ¡I");
          }
          pressanykey(buf);
          break;

        case '7':
          rpg_uinfo();
          attack = 0;
          break;
      }

      if(l > 0)
      {
        attack = 0;
        sprintf(buf,"%s ³Q©w¦í¤F¡AÁÙ­n %d ¦^¦X¤~¯à°Ê§@....",m.name,l--);
        pressanykey(buf);
      }

      /* ¼Ä¤Hªº§ğÀ» */
      if(attack == 1)
      {
        move(12,0);
        prints("%-255s"," ");
        move(12,0);
        prints(" [1;33m%s [31m§ğÀ»¡I¡I[m",m.name);
        k = rand()%((m.Att-Def) >=5 ? m.Att - Def :5);  /* ¼Ä¤H¨S¯S§ğ */
        sprintf(buf," §A ¥¢¦å %d ÂI¡C",k);
        rpguser.hp -= k;
        if(rpguser.hp < 0) rpguser.hp = 0; // ­tªºÂ_½u«á·|¥[¦^¨Ó?
        rpg_rec(cuser.userid, rpguser);
        substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
        /* Á×§KUserÂ_½u¨S¬ö¿ı¨ì */
        pressanykey(buf);
        move(12,0);
        prints("                                                                               ");   
      }

      /* ¼Ä¤H±¾¤F */
      if(m.hp <= 0)
      {
        money = m.money + rand()%(20*m.LV);
        exp = m.EXP + rand()%(10*m.LV);
        sprintf(buf,"²×©óÄ¹¤F¡I¡I±o¨ì %d ÂI¸gÅç­È¡A¥H¤Î %d ¤¸",exp,money);
        pressanykey(buf);
        inmoney(money);
        inexp(exp);
        over = 1;  /* °j°éµ²§ô */
      }

      /* ¼Ä¤H°k¶] */
      if((m.hp*100)/m.maxhp < 3 && rand()%10 < 3 && m.hp > 0)
      {
        money = (rand()%m.money)+20;  /* ¼Ä¤H°k¶]ÁÈ¨ìªº¿ú¸òexp¤ñ¸û¤Ö */
        exp = (rand()%m.EXP)+10;
        sprintf(buf,"%s ¥´¤£¹L§A°k¨«¤F¡A±o¨ì %d ¸gÅç­È %d ¤¸",m.name,exp,money);
        pressanykey(buf);
        inmoney(money);
        inexp(exp);
        over = 1;  /* °j°éµ²§ô */
      }
      /* °}¤`Åo */
      if(rpguser.hp <= 0)
      {
        money = (rand()%(m.LV*100))+100;
        exp = (rand()%(m.LV*m.LV*10)) + 100;
        sprintf(buf,"¤£©¯°}¤`¤F¡I¡I·l¥¢%dÂI¸gÅç­È,%d¤¸",exp,money);
        pressanykey(buf);
        demoney(money);
        deexp(exp);
        rpguser.hp = 0;   /* hp¤£·|ÅÜ­tªº */
        over = 1;       /* °j°éµ²§ô */
      }
    }
    substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
    rpg_rec(cuser.userid, rpguser);
    return 0;
  }
  return 0;
}


void
rpg_train()
{
  char buf[3];
  int mode;

  if(check_money(10*rpguser.level,SILVER)) return;
  demoney(10*rpguser.level);
  stand_title("°V½m³õ");
  show_file(BBSHOME"/game/rpg/Welcome_train", 2, 15, ONLY_COLOR);
  getdata(18,0,"³ß¦n¿ï¾Ü 1.¦Û­h¶É¦V 2.§V¤O¤£¾Ó 3.»´ÃP¦Û¦p ",buf,2,DOECHO,0);
  if(buf[0] == '1') mode = hard;
  else if(buf[0] == '2') mode = medium;
  else if(buf[0] == '3') mode = easy;
  else return;
  do{
    rpg_kmob(mode);
    buf[0] = answer("¬O§_Ä~Äò¡H (y/N)");
  }while(buf[0] == 'y');
  return;
}

/* ­×§ï¸ê®Æ */

int
rpg_uquery(char *userid)
{
  char buf[80];
  rpgrec rpg;
  int u;

  if(u = getuser(userid))
    memcpy(&rpg , &rpgtmp, sizeof(rpgrec));
  else return RC_NONE;

  getdata(23,0,"¬O§_­n­×§ï¸ê®Æ(Y/N)¡H  [N]",buf,3,DOECHO,"N");
  if(buf[0] == 'y')
  {
    clear();
    sprintf(buf,"%i",rpg.race);
    getdata(0,0,"Â¾·~¡G (1)Äé¤ô±j¤H (2)Åª¤å§Ö¤â (3)±`¾nµ{¦¡\n"
                "       (4)²á¤Ñ²r±N (5)¤ô²y«a­x (6)¹CÀ¸«g¬P ",
    buf,3,LCECHO,buf);
      if(buf[0] < '7' && buf[0] > '0')
        rpg.race= atoi(buf);
    sprintf(buf,"%i",rpg.level);
    getdata(2,0,"¿é¤Jµ¥¯Å¡G",buf,4,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.level = atol(buf);
    sprintf(buf,"%i",rpg.hp);
    getdata(3,0,"Åé¤O¡G",buf,8,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.hp = atol(buf);
    sprintf(buf,"%i",rpg.mp);
    getdata(3,40,"ªk¤O¡G",buf,8,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.mp = atol(buf);
    sprintf(buf,"%i",rpg.str);
    getdata(5,0,"¤O¶q¡G",buf,4,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.str = atol(buf);
  sprintf(buf,"%i",rpg.wis);
  getdata(5,40,"´¼¼z¡G",buf,4,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.wis = atol(buf);
  sprintf(buf,"%i",rpg.dex);
  getdata(6,0,"±Ó±¶¡G",buf,4,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.dex = atol(buf);
  sprintf(buf,"%i",rpg.con);
  getdata(6,40,"Åé½è¡G",buf,4,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.con = atol(buf);
  sprintf(buf,"%i",rpg.kar);
  getdata(7,0,"¹B®ğ¡G",buf,4,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.kar = atol(buf);
  sprintf(buf,"%i",rpg.weapon);
  getdata(9,0,"ªZ¾¹¥N½X¡G",buf,4,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.weapon = atol(buf);
  sprintf(buf,"%i",rpg.armor);
  getdata(10,0,"¨¾¨ã¥N½X¡G",buf,4,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.armor=atol(buf);
  sprintf(buf,"%i",rpg.age);
  getdata(11,0,"¦~ÄÖ¡G",buf,9,DOECHO,buf);
      if(atol(buf) >=0)
        rpg.age=atol(buf);
  getdata(20, 0, msg_sure_yn, buf, 3, LCECHO,"y");
    if (buf[0] != 'y')
    {
      pressanykey("©ñ±ó­×§ï");
    }
    else
    {
      log_usies("SetRPG", rpg.userid);
      rpg_rec(userid, rpg);
      pressanykey("­×§ï§¹¦¨");
    }
  }
  return 0;
}

/* ½s¿èUser¸ê®Æ */
rpg_edit()
{
  int id;
  char genbuf[200];
  stand_title("­×§ï¸ê®Æ");
  usercomplete(msg_uid,genbuf);
  if(*genbuf)
  {
    move(2,0);
    if (id = getuser(genbuf))
    {
      rpg_udisplay(genbuf);
      rpg_uquery(genbuf);
    }
    else
    {
      pressanykey(err_uid);
    }
  }
  return 0;
}

/* PK */

int
rpg_pk(fd,enemyid)
  int fd;
  char enemyid[IDLEN+1];
{
  rpgrec enemy; 
  userec euser; 
  int MHP = rpguser.con*30,MMP= rpguser.wis*10;  /* ¦Û¤vªº³Ì¤jmp,hp */
  int Att = c_att(cuser.userid);  /* §ğÀ»¤O */
  int Def = c_def(cuser.userid);  /* ¨¾¿m¤O */
  int MA = c_ma(cuser.userid);    /* Å]ªk§ğÀ» */
  int MD = c_md(cuser.userid);    /* Å]ªk¨¾¿m */
  int Emhp,Emmp,Eatt=c_att(enemyid) ,Edef= c_def(enemyid)
      ,Ema=c_ma(enemyid) ,Emd=c_md(enemyid) ; 
  int money,exp;  /* ¥´§¹«á·|¼W¥[or´î¤Öªº$$¸òexp */
  char buf[256]= "\0"; /* ansµ¹user input¥Î,bufµ¹¿Ã¹õoutput¥Î */
  int j=0,k=0,l=0,over=0,attack=0; /* overµ²§ô¾Ô°«,attack¼Ä¤H§ğÀ» */
  int ans;
  user_info *my = currutmp;

  getuser(enemyid);
  enemy = rpgtmp;
  euser = xuser;  
  Emhp = enemy.con*30; Emmp = enemy.wis*10;     
  add_io(fd, 0);

  setutmpmode(RPK); 

  for(;;)
  {
    if(over == 1) 
    {
      add_io(0, 0);
      close(fd);
      break;
    }
    else
    {
      update_data();
      clear();
      if(!HAS_PERM(PERM_SYSOP))
      {
        sethomefile(buf, enemyid, "picture");
        show_file(buf,1,10,ONLY_COLOR);  /* ¼Ä¤H¹Ï¤ù */
      }
      else
      {
        move(1,0); 
        prints(
"µ¥¯Å : %d\n¥Í©R : %d\n§ğ¨¾ : %d / %d\nÅ]ªk : %d / %d\n¸gÅç : %d\n»È¹ô : %d"
,enemy.level,Emhp,Eatt,Edef,Ema,Emd,euser.exp,euser.silvermoney);
      } 
      move(11,0);
      prints("[36m%s[m",msg_seperator);  /* ¤À¹j½u */
      move(13,0);
      prints("[36m%s[m",msg_seperator);  /* ¤À¹j½u */
      over = 0;
      while(over == 0)  /* ¾Ô°«...ª½¨ì over */
      {
        /* ¤£Åıuserª¾¹D©Çª«¦³¦h¤Ö¦å¡A©Ò¥H¥Î¨­Åéª¬ºAªí¥Ü */
        j = ((enemy.hp*100)/Emhp >= 100 ? 0 :
            (enemy.hp*100)/Emhp >= 75  ? 1 :
            (enemy.hp*100)/Emhp >= 50 ? 2 :
            (enemy.hp*100)/Emhp >= 25 ? 3 : 4);   
        move(0,0);
        clrtoeol();
        if(!HAS_PERM(PERM_SYSOP))
          prints("[1m[33;46m  ¡i ¾Ô °« ¤¤ ¡j                       \
[44m¡º ¼Ä¤H¡G %8s [40m°·±dª¬ºA¡G %s [m",euser.userid,health[j]);
        else
          prints("[1m[33;46m  ¡i ¾Ô °« ¤¤ ¡j          \
[44m¡º ¼Ä¤H¡G %8s [40m¥Í©RÂI¼Æ¡G%d/%d [m",euser.userid,enemy.hp,Emhp);
        move(14,0);
        prints("¡i¦Û¤vªºª¬ºA¡j
 ¡³ùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùŞùùùùùùùùùùùùùùùùùùùùùùùù¡³
 ùø¡iÅé  ¤O¡j %-4d / %4d  ¡iªk  ¤O¡j %-4d / %4d  ùø ¡i¸Ë³ÆªZ¾¹¡j%-8s   ùø
 ùø¡i§ğÀ»¤O¡j ¤@¯ë %4d  Å]ªk %4d                 ùø ¡i¸g Åç ­È¡j%-10d ùø
 ùø¡i¨¾¿m¤O¡j ¤@¯ë %4d  Å]ªk %4d                 ùø ¡i¨­¤W²{ª÷¡j%-10d ùø
 ¡³ùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùäùùùùùùùùùùùùùùùùùùùùùùùù¡³
        ",rpguser.hp,MHP,rpguser.mp,MMP,"ªÅµL¤@ª«",
          Att,MA,cuser.exp,Def,MD,cuser.silvermoney);
        sprintf(buf,"[1m[46;33m (1)[37m¤@¯ë§ğÀ»");
        move(20,0);
        outs(buf);
        move(23,0);
        outs(my->turn ? "½ü¨ì§Aªº§ğÀ»!" : "¹ï¤è§ğÀ»¤¤...");

        if(my->turn)
        {
          move(22,0);
          outs("­n°µ¤°»ò©O¡H");
          ans=igetch();
          switch(ans)
          {
            default:
              attack = 0;
              pressanykey("¬I¤u¤¤");
              break;
            /* ¤@¯ë§ğÀ» */
            case '1':
              sprintf(buf," [1;37m§A¬½¬½ªº©¹ [33m%s [37mªº [31m%s [37m¥´¤U¥h¡I¡I[m "
              ,euser.userid,body[rand()%5]);
              /* ¶Ã¼Æ¶]§ğÀ»³¡¦ì */
              move(12,0);
              outs(buf);
              k = (rand()%((Att-Edef)>=5 ? Att - Edef : 5))*j;
              sprintf(buf," %s ¥¢¦å %d ÂI¡C",euser.userid,k);
              enemy.hp -= k;              
              pressanykey(buf);
              my->turn = -1;
              send(fd, &my, sizeof(user_info), 0);
              my->turn = 0;
              attack = (enemy.hp > 0 ? 1 : 0);  /* ¼Ä¤H±¾¤F¤£·|¦A¤ÏÀ» */
              break;
            case 'q':
              over = 1;  /* °j°éµ²§ô */
              break;
          }
        }
        do{
          if(ans == I_OTHERDATA)
          {
            recv(fd, &my, sizeof(user_info), 0);
            if(my->turn == -1) my->turn = 1;
          }
        }while(!my->turn);
      }
    }
  }
  return 0;
}
