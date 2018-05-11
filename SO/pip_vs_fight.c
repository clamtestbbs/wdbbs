/*--------------------------------------------------------------------------*/
/* pip_vs_fight.c ¤pÂû¹ï¾Ôµ{¦¡				                    */
/* §@ªÌ:chyiuan   ·PÁÂSiEptheroªº§Ş³N«ü¾É				    */
/*--------------------------------------------------------------------------*/

int 
pip_set_currutmp()
{
	currutmp->pip.hp=d.hp;
	currutmp->pip.mp=d.mp;
	currutmp->pip.maxhp=d.maxhp;
	currutmp->pip.maxmp=d.maxmp;
	currutmp->pip.attack=d.attack;
	currutmp->pip.resist=d.resist;
	currutmp->pip.mresist=d.mresist;
	currutmp->pip.speed=d.speed;
	currutmp->pip.money=d.money;
}

int
pip_vf_fight(fd,first)
int fd;
int first;
{
	int ch,datac,dinjure,oldtired,oldhp;
	int oldhexp,oldmexp,oldmoney,oldhskill,oldmskill,oldbrave;
	int gameover=0,moneychange=0;	
	int oldpager;			/*¥n¾÷ª¬ºA*/
	int i;
	int notyou=0;			/*chyiuan:¥H§K°T®§³Q§Ë¿ù*/
	user_info *opponent;
	char data[200],buf1[256],buf2[256],mymsg[8][150];
	
	setutmpmode(CHICKENTALK);
	clear();
	
	opponent=currutmp->destuip;
	oldpager=currutmp->pager;	/*chyiuan:¦s¦bpageª¬ºA ¨Ã¥BÅÜ¦¨¨¾¤ô*/
	currutmp->pager=3;
	currutmp->pip.pipmode=0;	/*1:¿é¤F 2:Ä¹¤F 3:¤£ª±¤F */
	currutmp->pip.leaving=1;
	pip_set_currutmp();		/*§â¤pÂûªºdata  down load for³Q©I¥sªÌ*/
	currutmp->pip.nodone=first;	/*¨M©w½Ö¥ı§ğÀ»*/
	currutmp->pip.msgcount=0;	/*¾Ô°«°T®§Âk¹s*/
	currutmp->pip.chatcount=0;	/*²á¤Ñ°T®§Âk¹s*/
	currutmp->pip.msg[0]='\0';
	strcpy(currutmp->pip.name,d.name);
	
	/*¦s¤UÂÂ¤pÂûdata*/
	oldmexp=d.mexp;
	oldhexp=d.hexp;
	oldmoney=d.money;
	oldbrave=d.brave;
	oldhskill=d.hskill;
	oldmskill=d.mskill;
	
	add_io(fd, 0);
	/*¹ï¤è¥¼·Ç³Æ§´·í  ¥ıµ¥¤@¤U  ¬°¤F¨¾¤î·í¾÷ */
	while( gameover==0 && opponent->pip.leaving==0)
	{
		move(b_lines,0);
		prints("[1;46m ¹ï¤èÁÙ¦b·Ç³Æ¤¤                                                        [0m");
		ch=egetch();
	}
	for(i=0;i<8;i++)
			mymsg[i][0]='\0';
	for(i=0;i<10;i++)
			currutmp->pip.chat[i][0]='\0';
	/*¶}©lªº°T®§*/
	sprintf(mymsg[0],"[1;37m%s ©M %s ªº¾Ô°«¶}©l¤F..[0m",
	opponent->pip.name,currutmp->pip.name);
	strcpy(currutmp->pip.msg,mymsg[0]);
	currutmp->pip.msgcount=0;
	/*msgcount©Mcharcountªººâªk¤£¦P*/

	while (!(currutmp->pip.leaving==0 && opponent->pip.leaving==0))
	{
		clear();
		/*¬°¤F¤@¨Ç¨ä¥Lªº­ì¦]  ¹³Áı­¹µ¥¬O©I¥sÂÂªº  ©Ò¥Hreload*/
		pip_set_currutmp();
		
		if(opponent->pip.nodone!=1)
			strcpy(mymsg[currutmp->pip.msgcount%8],currutmp->pip.msg);
		move(0,0);
		prints("[1;34mùùùâ[44;37m ¦Û¤v¸ê®Æ [0;1;34mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		prints("[1m   [33m©m  ¦W:[37m%-20s                                              [31m  [m\n",
			d.name);
		sprintf(buf1,"%d/%d",d.hp,d.maxhp);
		sprintf(buf2,"%d/%d",d.mp,d.maxmp);
		prints("[1m   [33mÅé  ¤O:[37m%-12s[33mªk  ¤O:[37m%-12s[33m¯h  ³Ò:[37m%-12d                  [m\n",
			buf1,buf2,d.tired);
		prints("[1m   [33m§ğ  À»:[37m%-12d[33m¨¾  ¿m:[37m%-12d[33m³t  «×:[37m%-12d[33m§Ü  Å]:[37m%-9d  [m\n",
			d.attack,d.resist,d.speed,d.mresist);
		prints("[1m   [33m¾Ô°«§Ş:[37m%-12d[33mÅ]ªk§Ş:[37m%-12d[33mÅ]µû»ù:[37m%-12d[33mªZµû»ù:[37m%-9d  [m\n",
			d.hskill,d.mskill,d.mexp,d.hexp);
		prints("[1m   [33m­¹  ª«:[37m%-12d[33m¸É  ¤Y:[37m%-12d[33m¹s  ­¹:[37m%-12d[33mÆF  ªÛ:[37m%-9d  [m\n",
			d.food,d.bighp,d.cookie,d.medicine);
		prints("[1m   [33m¤H  çx:[37m%-12d[33m³·  ½¬:[37m%-12d[33mª÷  ¿ú:[37m%-15d               [m\n",
			d.ginseng,d.snowgrass,d.money);
		move(7,0);
		prints("[1;34mùùùâ[44;37m ¾Ô°«°T®§ [0;1;34mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		for(i=0;i<8;i++)
		{
			move(8+i,1);
			
			if(currutmp->pip.msgcount<8)
			{
				prints(mymsg[i]);
				/*¾A¥Îpip.msgcount¦b8¦æ¤º*/
			}
			else
			{
				prints(mymsg[(currutmp->pip.msgcount-8+i)%8]);
				/*pip.msgcount=8:ªí¥Ü¤w¸g¦³9­Ó ©Ò¥H±q0->7*/
			}
		}
		move(16,0);
		prints("[1;34mùùùâ[44;37m ½Í¸Ü°T®§ [0;1;34mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		for(i=0;i<2;i++)
		{
			move(17+i,0);
			if(currutmp->pip.chatcount<3)
			{
				prints(currutmp->pip.chat[i]);
				/*¾A¥Îpip.chatcount¦b2¦æ¤º*/
			}
			else
			{
				prints("%s",currutmp->pip.chat[(currutmp->pip.chatcount-2+i)%10]);
				/*pip.chatcount=3:ªí¥Ü¤w¸g¦³2­Ó ©Ò¥H±q0->1*/
			}
		}
		move(19,0);
		prints("[1;34mùùùâ[1;37;44m ¹ï¤â¸ê®Æ [0;1;34mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		prints("[1m   [33m©m  ¦W:[37m%-20s                                                [m\n",
			opponent->pip.name);
		sprintf(buf1,"%d/%d",opponent->pip.hp,opponent->pip.maxhp);
		sprintf(buf2,"%d/%d",opponent->pip.mp,opponent->pip.maxmp);
		prints("[1m   [33mÅé  ¤O:[37m%-12s[33mªk  ¤O:[37m%-12s[33mª÷  ¿ú:[37m%-15d               [m\n",
			buf1,buf2,opponent->pip.money);
		prints("[1;34mùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		if(opponent->pip.nodone==1)
		{
			notyou=1;
			prints("[1;37;44m  ¹ï¤è¥X©Û¤¤¡A½Ğµy«İ¤@·|.....                                [T/^T]CHAT/¦^ÅU  [m");
		}
		else	
		{
			notyou=0;		
			prints("[1;44;37m  ¾Ô°«©R¥O  [46m [1]´¶³q [2]¥ş¤O [3]Å]ªk [4]¨¾¿m [5]¸É¥R [6]°k©R [T/^T]CHAT/¦^ÅU  [m");
		}
		ch = egetch();
		if (ch == I_OTHERDATA)
		{
			datac = recv(fd, data, sizeof(data), 0);
			if (datac <= 0)
				break;
		}
		else if(ch =='T' || ch =='t')
		{
			int len;
			char msg[120];
			char buf[80];
			len=getdata(b_lines,0, "·Q»¡:", buf, 60, 1, 0);
			if(len && buf[0]!=' ')
			{
				sprintf(msg,"[1;46;33m¡¹%s[37;45m %s [0m",cuser.userid,buf);
				strcpy(opponent->pip.chat[currutmp->pip.chatcount%10],msg);				
				strcpy(currutmp->pip.chat[currutmp->pip.chatcount%10],msg);
				opponent->pip.chatcount++;
				currutmp->pip.chatcount++;
			}
			
		}
		else if(ch==Ctrl('T'))
		{
			clrchyiuan(7,19);
			move(7,0);
			prints("[1;31mùùùâ[41;37m ¦^ÅU½Í¸Ü [0;1;31mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
			for(i=0;i<10;i++)
			{
				move(8+i,0);
				if(currutmp->pip.chatcount<10)
				{
					prints(currutmp->pip.chat[i]);
					/*¾A¥Îpip.msgcount¦b¤C¦æ¤º*/
				}
				else
				{
					prints("%s",currutmp->pip.chat[(currutmp->pip.chatcount-10+i)%10]);
					/*pip.chatcount=10:ªí¥Ü¤w¸g¦³11­Ó ©Ò¥H±q0->9*/
				}
			}
			move(18,0);
			prints("[1;31mùùùâ[41;37m ¨ì¦¹¬°¤î [0;1;31mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m");
			pressanykey("¦^ÅU¤§«eªº½Í¸Ü ¥u¦³10³q");		
		}
		else if(currutmp->pip.nodone==1 && opponent->pip.leaving==1 && notyou==0)
		{
			d.nodone=1;
			switch(ch)
			{
				char buf[256];
				case '1': 
					if(rand()%9==0)
					{
						pressanykey("³ºµM¨S¥´¤¤..:~~~"); 
						sprintf(buf,"[1;33m%s [37m¹ï [33m%s[37m ¬I®i´¶³q§ğÀ»¡A¦ı¬O¨S¦³¥´¤¤...",
							d.name,opponent->pip.name);						
					}
					else
					{ 
						if(opponent->pip.resistmore==0)
							dinjure=(d.hskill/100+d.hexp/100+d.attack/9-opponent->pip.resist/12+rand()%20+2-opponent->pip.speed/30+d.speed/30);
						else
							dinjure=(d.hskill/100+d.hexp/100+d.attack/9-opponent->pip.resist/8+rand()%20+2-opponent->pip.speed/30+d.speed/30);            
						if(dinjure<=9)	dinjure=9;
						opponent->pip.hp-=dinjure;
						d.hexp+=rand()%2+2;
						d.hskill+=rand()%2+1;  
						sprintf(buf,"´¶³q§ğÀ»,¹ï¤èÅé¤O´î§C%d",dinjure);
						pressanykey(buf);
						sprintf(buf,"[1;33m%s [37m¬I®i¤F´¶³q§ğÀ»,[33m%s [37mªºÅé¤O´î§C [31m%d [37mÂI[0m"
							,d.name,opponent->pip.name,dinjure);						
					}
					opponent->pip.resistmore=0;
					opponent->pip.msgcount++;
					currutmp->pip.msgcount++;
					strcpy(opponent->pip.msg,buf);
					strcpy(mymsg[currutmp->pip.msgcount%8],buf);	
					currutmp->pip.nodone=2;	/*°µ§¹*/
					opponent->pip.nodone=1;
					break;
     
				case '2':
					show_fight_pic(2);
					if(rand()%11==0)
					{ 
						pressanykey("³ºµM¨S¥´¤¤..:~~~");
						sprintf(buf,"[1;33m%s [37m¹ï [33m%s[37m ¬I®i¥ş¤O§ğÀ»¡A¦ı¬O¨S¦³¥´¤¤...",
							d.name,opponent->pip.name);
					}     
					else 
					{ 
						if(opponent->pip.resistmore==0)      
							dinjure=(d.hskill/100+d.hexp/100+d.attack/5-opponent->pip.resist/12+rand()%30+6-opponent->pip.speed/50+d.speed/30);
						else
							dinjure=(d.hskill/100+d.hexp/100+d.attack/5-opponent->pip.resist/8+rand()%30+6-opponent->pip.speed/40+d.speed/30);                  
						if(dinjure<=20) dinjure=20;  
						if(d.hp>5)
						{ 
							opponent->pip.hp-=dinjure;
							d.hp-=5;
							d.hexp+=rand()%3+3;
							d.hskill+=rand()%2+2; 
							sprintf(buf,"¥ş¤O§ğÀ»,¹ï¤èÅé¤O´î§C%d",dinjure);
							pressanykey(buf);
							sprintf(buf,"[1;33m%s [37m¬I®i¤F¥ş¤O§ğÀ»,[33m%s [37mªºÅé¤O´î§C [31m%d [37mÂI[0m"
							,d.name,opponent->pip.name,dinjure);
						}
						else
						{ 
							d.nodone=1;
							pressanykey("§AªºHP¤p©ó5°Õ..¤£¦æ°Õ...");
						}
					}
					opponent->pip.resistmore=0;
					opponent->pip.msgcount++;
					currutmp->pip.msgcount++;
					strcpy(opponent->pip.msg,buf);
					strcpy(mymsg[currutmp->pip.msgcount%8],buf);	
					currutmp->pip.nodone=2;	/*°µ§¹*/
					opponent->pip.nodone=1;
					break;
     
				case '3':
					clrchyiuan(8,19);
					oldtired=d.tired;
					oldhp=d.hp;     
					d.magicmode=0;
					dinjure=pip_magic_menu(8,19); 
					if(dinjure<0)	dinjure=5;
					if(d.nodone==0)
					{
						if(d.magicmode==1)
						{
							oldtired=oldtired-d.tired;
							oldhp=d.hp-oldhp;
							sprintf(buf,"ªvÀø«á,Åé¤O´£°ª%d ¯h³Ò­°§C%d",oldhp,oldtired);
							pressanykey(buf);
							sprintf(buf,"[1;33m%s [37m¨Ï¥ÎÅ]ªkªvÀø¤§«á,Åé¤O´£°ª [36m%d [37mÂI¡A¯h³Ò­°§C [36m%d [37mÂI[0m",d.name,oldhp,oldtired);
						}
						else
						{
							if(rand()%15==0)
							{
								pressanykey("³ºµM¨S¥´¤¤..:~~~");  
								sprintf(buf,"[1;33m%s [37m¹ï [33m%s[37m ¬I®iÅ]ªk§ğÀ»¡A¦ı¬O¨S¦³¥´¤¤...",
									d.name,opponent->pip.name);
							}
							else
							{  
								if(d.mexp<=100)
								{
									if(rand()%4>0)
										dinjure=dinjure*60/100;
									else
										dinjure=dinjure*80/100;
								}
								else if(d.mexp<=250 && d.mexp>100)
								{
									if(rand()%4>0)
										dinjure=dinjure*70/100;
									else
										dinjure=dinjure*85/100;           
								}
								else if(d.mexp<=500 && d.mexp>250)
								{
									if(rand()%4>0)
										dinjure=dinjure*85/100;
									else
										dinjure=dinjure*95/100;           
								}
								else if(d.mexp>500)
								{
									if(rand()%10>0)
										dinjure=dinjure*90/100;
									else
										dinjure=dinjure*99/100;           
								}
								dinjure=dinjure/2;
								opponent->pip.hp-=dinjure; 
								d.mskill+=rand()%2+2;  
								sprintf(buf,"Å]ªk§ğÀ»,¹ï¤èÅé¤O´î§C%d(¹ï¾Ô¥´¹ï§é)",dinjure);
								pressanykey(buf);
								sprintf(buf,"[1;33m%s [37m¬I®i¤FÅ]ªk§ğÀ»,[33m%s [37mªºÅé¤O´î§C [31m%d [37mÂI[0m"
								,d.name,opponent->pip.name,dinjure);
							}
						}
						
						opponent->pip.msgcount++;
						currutmp->pip.msgcount++;
						strcpy(opponent->pip.msg,buf);
						strcpy(mymsg[currutmp->pip.msgcount%8],buf);
						/*«ì´_Åé¤O¬O¥Îd.hp©Md.maxhp¥h ©Ò¥H±o§ó·s*/
						currutmp->pip.hp=d.hp;
						currutmp->pip.mp=d.mp;
						currutmp->pip.nodone=2;	/*°µ§¹*/
						opponent->pip.nodone=1;
					}
					break;  
				
				case '4':
					currutmp->pip.resistmore=1;
					pressanykey("¤pÂû¥[±j¨¾¿m°Õ....");
					sprintf(buf,"[1;33m%s [37m¥[±j¨¾¿m¡A·Ç³Æ¥ş¤O©è¾× [33m%s [37mªº¤U¤@©Û[0m",
						d.name,opponent->pip.name);
					opponent->pip.msgcount++;
					currutmp->pip.msgcount++;
					strcpy(opponent->pip.msg,buf);
					strcpy(mymsg[currutmp->pip.msgcount%8],buf);
					currutmp->pip.nodone=2;	/*°µ§¹*/
					opponent->pip.nodone=1;
					break;
				case '5':
					pip_basic_feed();
					if(d.nodone!=1)
					{
						sprintf(buf,"[1;33m%s [37m¸É¥R¤F¨­¤Wªº¯à¶q¡AÅé¤O©Îªk¤O¦³ÅãµÛªº´£¤É[0m",d.name);
						opponent->pip.msgcount++;
						currutmp->pip.msgcount++;
						strcpy(opponent->pip.msg,buf);
						strcpy(mymsg[currutmp->pip.msgcount%8],buf);						
						/*«ì´_Åé¤O¬O¥Îd.hp©Md.maxhp¥h ©Ò¥H±o§ó·s*/
						currutmp->pip.hp=d.hp;
						currutmp->pip.mp=d.mp;
						currutmp->pip.nodone=2;	/*°µ§¹*/
						opponent->pip.nodone=1;
					}
					break;
				case '6':
					opponent->pip.msgcount++;
					currutmp->pip.msgcount++;
					if(rand()%20>=18 || (rand()%20>13 && d.speed <= opponent->pip.speed))
					{
						pressanykey("·Q°k¶]¡A«o¥¢±Ñ¤F...");
						sprintf(buf,"[1;33m%s [37m·Q¥ı°k¶]¦A»¡...¦ı«o¥¢±Ñ¤F...[0m",d.name);
						strcpy(opponent->pip.msg,buf);
						strcpy(mymsg[currutmp->pip.msgcount%8],buf);
					}
					else
					{
						sprintf(buf,"[1;33m%s [37m¦ÛÄ±¥´¤£¹L¹ï¤è¡A©Ò¥H¨M©w¥ı°k¶]¦A»¡...[0m",d.name);
						strcpy(opponent->pip.msg,buf);
						strcpy(mymsg[currutmp->pip.msgcount%8],buf);
						currutmp->pip.pipmode=3;
						moneychange=(rand()%200+300);
						currutmp->pip.money-=moneychange;
						opponent->pip.money+=moneychange;
						d.brave-=(rand()%3+2);      
						if(d.hskill<0)
							d.hskill=0;
						if(d.brave<0)
							d.brave=0;
						clear(); 
						showtitle("¹q¤l¾i¤pÂû", BoardName); 
						move(10,0);
						prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m");
						move(11,0);
						prints("            [1;31m¢x  [37m¹ê¤O¤£±jªº¤pÂû [33m%-10s                 [31m¢x[m",d.name);
						move(12,0);
						prints("            [1;31m¢x  [37m¦b»P¹ï¤â [32m%-10s [37m¾Ô°««á¸¨¶]°Õ          [31m¢x[m",opponent->pip.name);
						move(13,0);
						sprintf(buf1,"%d/%d",d.hexp-oldhexp,d.mexp-oldmexp);  
						prints("            [1;31m¢x  [37mµû»ù¼W¥[¤F [36m%-5s [37mÂI  §Ş³N¼W¥[¤F [36m%-2d/%-2d [37mÂI  [31m¢x[m",buf1,d.hskill-oldhskill,d.mskill-oldmskill);
						move(14,0);
						sprintf(buf2,"%d [37m¤¸",moneychange);
						prints("            [1;31m¢x  [37m«i´±­°§C¤F [36m%-5d [37mÂI  ª÷¿ú´î¤Ö¤F [36m%-13s  [31m¢x[m",oldbrave-d.brave,buf2);
						move(15,0);
						prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m");             
						pressanykey("¤T¤Q¤»­p ¨«¬°¤Wµ¦...");
					}
					currutmp->pip.nodone=2;	/*°µ§¹*/
					opponent->pip.nodone=1;
					break;
       	
				
			}
		}
		if(currutmp->pip.hp<0)
		{
			currutmp->pip.pipmode=1;
			opponent->pip.pipmode=2;
		}
		if(currutmp->pip.pipmode==2 || opponent->pip.pipmode==1 || opponent->pip.pipmode==3)
		{
			clear();
			showtitle("¹q¤l¾i¤pÂû", BoardName);
			move(10,0);
			prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m");
			move(11,0);
			prints("            [1;31m¢x  [37m­^«iªº¤pÂû [33m%-10s                     [31m¢x[m",d.name);
			move(12,0);
			prints("            [1;31m¢x  [37m¥´±Ñ¤F¹ï¤è¤pÂû [32m%-10s                 [31m¢x[m",opponent->pip.name);
			move(13,0);
			sprintf(buf1,"%d/%d",d.hexp-oldhexp,d.mexp-oldmexp);  
			prints("            [1;31m¢x  [37mµû»ù´£¤É¤F %-5s ÂI  §Ş³N¼W¥[¤F %-2d/%-2d ÂI  [31m¢x[m",buf1,d.hskill-oldhskill,d.mskill-oldmskill);
			move(14,0);
			sprintf(buf2,"%d ¤¸",currutmp->pip.money-oldmoney);
			prints("            [1;31m¢x  [37m«i´±´£¤É¤F %-5d ÂI  ª÷¿ú¼W¥[¤F %-9s [31m¢x[m",d.brave-oldbrave,buf2);
			move(15,0);
			prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m");     
			if(opponent->pip.hp<=0)     
				pressanykey("¹ï¤è¦º±¼Åo..©Ò¥H§AÄ¹Åo..");   
			else if(opponent->pip.hp>0)
				pressanykey("¹ï¤è¸¨¶]Åo..©Ò¥Hºâ§AÄ¹Åo.....");  
			d.money=currutmp->pip.money;
			pip_write_file();
		}
		if(gameover!=1 && (opponent->pip.pipmode==2 || currutmp->pip.pipmode==1))
		{
			moneychange=currutmp->pip.money*(1+rand()%2)/5;
			currutmp->pip.maxhp=currutmp->pip.maxhp*2/3;
			currutmp->pip.hp=currutmp->pip.maxhp/2;
			currutmp->pip.money-=moneychange;
			opponent->pip.money+=moneychange;
			d.mexp-=rand()%20+20;
			d.hexp-=rand()%20+20;
			d.hskill-=rand()%20+20;
			d.mskill-=rand()%20+20;
			pip_write_file();
			clear();
			showtitle("¹q¤l¾i¤pÂû", BoardName);       
			move(10,0);
			prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m");
			move(11,0);
			prints("            [1;31m¢x  [37m¥i¼¦ªº¤pÂû [33m%-10s                     [31m¢x[m",d.name);
			move(12,0);
			prints("            [1;31m¢x  [37m¦b»P [32m%-10s [37mªº¾Ô°«¤¤¡A                [31m¢x[m",opponent->pip.name);
			move(13,0);
			prints("            [1;31m¢x  [37m¤£©¯¦a¥´¿é¤F¡A°OªÌ²{³õ¯S§O³ø¾É.........   [31m¢x[m");
			move(14,0);
			prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m");        
			pressanykey("¤pÂû¥´¿é¤F....");
		}
		d.hp=currutmp->pip.hp;
		d.mp=currutmp->pip.mp;
		d.maxhp=currutmp->pip.maxhp;
		d.maxmp=currutmp->pip.maxmp;
		d.money=currutmp->pip.money;
		if(opponent->pip.pipmode!=0 || currutmp->pip.pipmode!=0)
		{
			currutmp->pip.leaving=0;
		}

	}
	pip_write_file();
	add_io(0, 0);
	close(fd);
	currutmp->destuip=0;
	currutmp->pager=oldpager;
	setutmpmode(CHICKEN);
}

int va_pip_vf_fight(va_list pvar)
{
  int sock, later;
  sock = va_arg(pvar, int);
  later = va_arg(pvar,int);
  return pip_vf_fight(sock, later);
}
