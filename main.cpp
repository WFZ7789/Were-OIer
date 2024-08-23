#include<bits/stdc++.h>

#include <windows.h>
using namespace std;
#define int long long

#define qdo 262//q前缀为低音，1后缀为高音，s前缀为半音阶
#define qre 294
#define qmi 330
#define qfa 349
#define qso 392
#define qla 440
#define qsi 494
#define do 523
#define re 578
#define mi 659
#define fa 698
#define so 784
#define la 880
#define si 988
#define do1 1046
#define re1 1175
#define mi1 1318
#define fa1 1480
#define so1 1568
#define la1 1760
#define si1 1976
#define sqdo 277
#define sqre 311
#define sqfa 370
#define sqso 415
#define sqla 466
#define sdo 554
#define sre 622
#define sfa 740
#define sso 831
#define sla 932
#define sdo1 1046
#define sre1 1245
#define sfa1 1480
#define sso1 1661
#define sla1 1865
string s[15]={"平民","狼人","魔术师","猎人","女巫","白狼王","预言家"};
string word[15]={"如果我说我是好人你信吗？ 如果我说我不是狼人你们也不会信对吧？ 如果我死，无所谓投就投吧","从我看，我们应该集中精力找出真正的狼人，而不是争吵起来。我们要紧密合作，互相分享信息，才能找到线索。","我的牌是狼人，但是被魔术师换了...","啊啊啊女巫，女巫，保护我们啊","魔术师，魔术师！我要换牌!","(看手机)(查百度)(照着念)我是一位值得信赖的朋友，我是好人，请大家相信我","I人冒泡"};
string id_word[105]={"大家千万不要投我是猎人，我死带错人就惨了。","朋友们，我是女巫，是平民的最强战力，把我都投出去了，你们咋办？","坏了，这把是平民毫无体验感，快杀我，我要重开！","我是全场唯一真预言家,","女巫们，保我！我是真的预言家，穿我衣服骗解药，说的是谁自己心里清楚！","别看我只是一个民~"} ;
string www[105]={"我是铁好人","哈哈哈，我说我是坏人你信吗？","我老实巴交，是个萌新，看着你们这些高手只能膜拜","慌...","大佬们照顾一下，我第一次玩","幸好我不是狼，不然就开局寄","大家都是好人不必要自相残害对吧"}; 
int id_has_said[15];
string player[15];
string me;
int r; 
unordered_map<string,int> id;
int condition[105];
int killedbywolf[105];
int kbwnum;
int died[105],dnum; 
int vote[105]; 
vector<int> voten[105]; 
int mustkill1,mustkill2,mustsave1,mustsave2;
bool is_say=0;
int when_say,say_who,who_say;
string say_what;
int saved[105],snum;
int rest=10; 
int ansss;
bool ten=0;
int votewrong[105];
string strin[105];
int last_strin;
int reward;
string rewards[105];
int no_die; 
int nxtid=-1; 
int mony;
unordered_map<string,int> giftt;
int smart[15];
int trust[15]; 
void music()
{
	while(1)
	{
		_sleep(500);
		Beep(mi,300);
		Beep(si,300);
		Beep(do1,300);
		Beep(si,300);
		Beep(mi,350);
		_sleep(1000);
		Beep(mi1,400);
		_sleep(500);
		Beep(mi,300);
		Beep(si,300);
		Beep(do1,300);
		Beep(si,300);
		Beep(re1,300);
		Beep(do1,300);
		Beep(si,500);
	    std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
	}
	
}
void mmm()
{
	memset(condition,0,sizeof(condition));
	memset(killedbywolf,0,sizeof(killedbywolf));
	memset(died,0,sizeof(died));
	for(int i=1;i<=105;i++)
	{
		voten[i].clear();
	}
	dnum=0;
	mustkill1=0,mustkill2=0,mustsave1=0,mustsave2=0;
	snum=0;
	ansss=0;
}
void gift()
{
	string s;
	while(s!="退出")
	{
		cout<<"请输入礼包密码，想退出，请输入“退出”"<<endl;
		cin>>s;
		if(s=="bilibili"&&giftt[s]==0)
		{
			cout<<"恭喜您获得成就：B站之神：输入密码从礼包获得"<<endl;
			rewards[++reward]="B站之神";
			giftt[s]=1;
		}
		else if(s=="lrsws"&&giftt[s]==0)
		{
			cout<<"恭喜您获得一次抽取下一次身份的机会："<<endl;
			cout<<"平民 R 25%"<<endl;
			cout<<"猎人 RR 20%"<<endl;
			cout<<"预言家 S 20%"<<endl;
			cout<<"魔术师 SR 17%"<<endl;
			cout<< "狼人 SS 10%"<<endl;
			cout<<"白狼王 SSS 5%"<<endl;
			cout<<"女巫 SSS+ 3%"<<endl; 
			_sleep(4000);
			system("cls");
			cout<<3<<endl;
			_sleep(1000);
			system("cls");
			cout<<2<<endl;
			_sleep(1000);
			system("cls");
			cout<<1<<endl;
			_sleep(1000);
			system("cls");
			int nxtidd=rand()%100;
			nxtidd++;
			if(nxtidd<=25)
			{
				cout<<"您下一次的身份是：平民"<<endl;
				nxtid= 0;
			}
			else if(nxtidd>25&&nxtidd<=45)
			{
				cout<<"您下一次的身份是：猎人"<<endl; 
				nxtid= 3;
			}
			else if(nxtidd>45&&nxtidd<=65)
			{
				cout<<"您下一次的身份是：预言家"<<endl; 
				nxtid= 6;
			}
			else if(nxtidd>65&&nxtidd<=82)
			{
				cout<<"您下一次的身份是：魔术师"<<endl; 
				nxtid= 2;
			}
			else if(nxtidd>82&&nxtidd<=92)
			{
				cout<<"您下一次的身份是：狼人"<<endl; 
				nxtid= 1;
			}
			else if(nxtidd>92&&nxtidd<=97)
			{
				cout<<"您下一次的身份是：白狼王"<<endl; 
				nxtid=5;
			}
			else if(nxtidd>97&&nxtidd<=100)
			{
				cout<<"您下一次的身份是：女巫"<<endl; 
				nxtid= 4;
			}
			giftt[s]=1;
		}
		else if(s=="12345"&&giftt[s]==0)
		{
			cout<<"狼人杀币+40";
			mony+=40; 
			giftt[s]=1;
		}
		else if(s=="64514"&&giftt[s]==0)
		{
			cout<<"狼人杀币+114";
			mony+=114; 
			giftt[s]=1;
		}
		else if(s=="Iwantmoney"&&giftt[s]==0)
		{
			cout<<"狼人杀币+100";
			mony+=100; 
			giftt[s]=1;
		}
		else if(s=="money"&&giftt[s]==0)
		{
			cout<<"狼人杀币+60";
			mony+=60; 
			giftt[s]=1;
		}
		else if(s=="langrensha"&&giftt[s]==0)
		{
			cout<<"狼人杀币+70";
			mony+=70; 
			giftt[s]=1;
		}
		else if(s=="Base_ring_tree"&&giftt[s]==0)
		{
			cout<<"狼人杀币+1000"<<endl;
			cout<<"恭喜您获得成就：作者的馈赠Lv.1"<<endl;
			rewards[++reward]="作者的馈赠Lv.1";
			mony+=1000; 
			giftt[s]=1;
		}
		else if(s=="jihuanshu"&&giftt[s]==0)
		{
			cout<<"狼人杀币+1000"<<endl;
			cout<<"恭喜您获得成就：作者的馈赠Lv.2"<<endl;
			rewards[++reward]="作者的馈赠Lv.2";
			mony+=1000; 
			giftt[s]=1;
		}
		else if(s=="florr"&&giftt[s]==0)
		{
			cout<<"狼人杀币+88"<<endl;
			cout<<"恭喜您获得成就：中华好花花"<<endl;
			rewards[++reward]="中华好花花";
			mony+=88; 
			giftt[s]=1;
		}
		else if(s=="digdig"&&giftt[s]==0)
		{
			cout<<"狼人杀币+99"<<endl;
			cout<<"恭喜您获得成就：diger"<<endl;
			rewards[++reward]="diger";
			mony+=99; 
			giftt[s]=1;
		}
		else if(s=="sb"&&giftt[s]==0)
		{
			cout<<"狼人杀币-99"<<endl;
			cout<<"恭喜您获得成就：文明你我他"<<endl;
			rewards[++reward]="文明你我他";
			mony-=99; 
			giftt[s]=1;
		}
		else if(s=="狼人杀"&&giftt[s]==0)
		{
			cout<<"狼人杀币+999"<<endl;
			mony+=999; 
			giftt[s]=1;
		}
		else{
			cout<<"无效密码"<<endl;
		}
		_sleep(3000);
		system("cls");
	}
}
void store()
{
	int cnt=0;
	while(cnt!=8)
	{
		cout<<"~~~~~欢迎光临~~~~~"<<endl;
		cout<<"您有"<<mony<<"块狼人杀币"<<endl;
		cout<<"输入编号，购买下一次的身份" <<endl;
		cout<<"1.女巫：500  2.白狼王：400  3.狼人：350  4.魔术师：300  5.预言家：250 6.猎人：100"<<endl;
		cout<<"输入7，盲盒抽取，盲盒300一个"<<endl;
		cout<<"输入8退出"<<endl ;
		cin>>cnt;
		if(cnt==1)
		{
			if(mony>=500)
			{
				cout<<"支付成功！"<<endl;
				nxtid= 4;
				mony-=500;
			}
			else{
				cout<<"余额不足"<<endl; 
			}
		}
		if(cnt==2)
		{
			if(mony>=400)
			{
				cout<<"支付成功！"<<endl;
				nxtid= 5;
				mony-=400;
			}
			else{
				cout<<"余额不足"<<endl; 
			}
		}
		if(cnt==3)
		{
			if(mony>=350)
			{
				cout<<"支付成功！"<<endl;
				nxtid= 1;
				mony-=350;
			}
			else{
				cout<<"余额不足"<<endl; 
			}
		}
		if(cnt==4)
		{
			if(mony>=300)
			{
				cout<<"支付成功！"<<endl;
				nxtid= 2;
				mony-=300;
			}
			else{
				cout<<"余额不足"<<endl; 
			}
		}
		if(cnt==5)
		{
			if(mony>=250)
			{
				cout<<"支付成功！"<<endl;
				nxtid= 6;
				mony-=250;
			}
			else{
				cout<<"余额不足"<<endl; 
			}
		}
		if(cnt==6)
		{
			if(mony>=100)
			{
				cout<<"支付成功！"<<endl;
				nxtid= 3;
				mony-=100;
			}
			else{
				cout<<"余额不足"<<endl; 
			}
		}
		if(cnt==7)
		{
			if(mony>=300)
			{
				cout<<"恭喜您获得一次抽取下一次身份的机会："<<endl;
				cout<<"平民 R 25%"<<endl;
				cout<<"猎人 RR 20%"<<endl;
				cout<<"预言家 S 20%"<<endl;
				cout<<"魔术师 SR 17%"<<endl;
				cout<< "狼人 SS 10%"<<endl;
				cout<<"白狼王 SSS 5%"<<endl;
				cout<<"女巫 SSS+ 3%"<<endl; 
				_sleep(4000);
				system("cls");
				cout<<3<<endl;
				_sleep(1000);
				system("cls");
				cout<<2<<endl;
				_sleep(1000);
				system("cls");
				cout<<1<<endl;
				_sleep(1000);
				system("cls");
				int nxtidd=rand()%100;
				nxtidd++;
				if(nxtidd<=25)
				{
					cout<<"您下一次的身份是：平民"<<endl;
					_sleep(1000);
					nxtid= 0;
				}
				else if(nxtidd>25&&nxtidd<=45)
				{
					cout<<"您下一次的身份是：猎人"<<endl; 
					_sleep(1000);
					nxtid= 3;
				}
				else if(nxtidd>45&&nxtidd<=65)
				{
					cout<<"您下一次的身份是：预言家"<<endl; 
					_sleep(1000);
					nxtid= 6;
				}
				else if(nxtidd>65&&nxtidd<=82)
				{
					cout<<"您下一次的身份是：魔术师"<<endl; 
					_sleep(1000);
					nxtid= 2;
				}
				else if(nxtidd>82&&nxtidd<=92)
				{
					cout<<"您下一次的身份是：狼人"<<endl;
					_sleep(1000); 
					nxtid= 1;
				}
				else if(nxtidd>92&&nxtidd<=97)
				{
					cout<<"您下一次的身份是：白狼王"<<endl; 
					_sleep(1000);
					nxtid=5;
				}
				else if(nxtidd>97&&nxtidd<=100)
				{
					cout<<"您下一次的身份是：女巫"<<endl; 
					_sleep(1000);
					nxtid= 4;
				}
			}
			else{
				cout<<"余额不足"<<endl;
			}
		}
		_sleep(100);
		system("cls");
	}
	
	
}
void wolfman_player()
{
	 for(int i=1;i<=9;i++)
	 {
	 	if(player[i]=="狼人"&&condition[i]==0)
	 	{
	 		cout<<"您环顾四周，发现"<<i<<"号玩家也是狼人"; 
	 		break;
		 }
     }
     _sleep(3000);
     system("cls");
	int man=0;
	while(man>9||man<1||condition[man]==1)
	{
		cout<<"您今晚要刀谁:"<<endl;
		cout<<"已死玩家：";
		for(int i=1;i<=10;i++)
		{
			if(condition[i]==1)
			{
				cout<<i<<"号 ";
			}
		 } 
		 cout<<endl;
		cin>>man;
		system("cls");
	}
	system("cls");
	killedbywolf[man]=1;
	died[++dnum]=man;
	id[player[man]]--;
	condition[man]=1;
 } 
 void wolfking_player()
{
	 
	int man=0;
	while(man>9||man<1||condition[man]==1)
	{
		cout<<"您今晚要刀谁:"<<endl;
		cout<<"已死玩家：";
		for(int i=1;i<=10;i++)
		{
			if(condition[i]==1)
			{
				cout<<i<<"号 ";
			}
		 } 
		 cout<<endl;
		cin>>man;
		system("cls");
	}
	system("cls");
	killedbywolf[man]=1;
	died[++dnum]=man;
	id[player[man]]--;
	condition[man]=1;
 } 
 void wolfman_computer()
 {
 	if(mustkill1!=0&&condition[mustkill1]==0)
 	{
 		int man=mustkill1;
	 	killedbywolf[man]=1;
	 	died[++dnum]=man;
	 	id[player[man]]--;
	 	condition[man]=1;
	 	return ;
	 }
	 if(mustkill2!=0&&condition[mustkill2]==0)
 	{
 		int man=mustkill2;
	 	killedbywolf[man]=1;
	 	died[++dnum]=man;
	 	id[player[man]]--;
	 	condition[man]=1;
	 	return ;
	 }
	 int rrar=rand()%3;
 	int man=rand()%10;
 	man++;
 	if(rrar!=0)
 	{
 		while(condition[man]==1)
	 	{
	 		man=rand()%10+1;
		 }
	 }
 	else{
 		int minn=-100000;
 		for(int i=1;i<=10;i++)
 		{
 			if(trust[i]>minn)
 			{
 				minn=trust[i];
 				man=i;
			 }
		 }
	 }
 	killedbywolf[man]=1;
 	died[++dnum]=man;
 	id[player[man]]--;
 	condition[man]=1;
 }
 void magician_player()
 {
 	int x=0,y=0;
 	while(x<1||x>10||y<1||y>10||condition[y]==1||condition[x]==1)
 	{ 
 		cout<<"您今晚要交换哪两个人的牌（如果您不想做任何操作请输入两个相同的数字）:"<<endl;
 	 	cin>>x>>y;
 	 	system("cls");
 	} 
 	swap(player[x],player[y]);
 	system("cls");
 }
 void magician_computer()
 {
 	 
 	int x=rand()%10;
	int y=rand()%10;
	while(condition[x]==1||condition[y]==1)
 	{
 		x=rand()%10+1;
 		y=rand()%10+1;
	 }
	x++;
	y++;
	swap(player[x],player[y]);
 }
 void witch_player()
 {
 	int f=-1;
 	int x=0;
 	for(int i=1;i<=9;i++)
	 {
	 	if(player[i]=="女巫"&&condition[i]==0)
	 	{
	 		cout<<"您环顾四周，发现"<<i<<"号玩家也是女巫"; 
	 		break;
		 }
     }
     _sleep(3000);
     system("cls");
 	while(x<1||x>10||(f!=0&&f!=1))
 	{
 		cout<<"您今晚要救哪个被狼人杀了的人？或者你要杀谁？（救人输入1，杀人输入0）"<<endl;
 		cout<<"这些人是今夜被狼人杀害的：" ;
 		for(int i=1;i<=9;i++)
 		{
 			if(killedbywolf[i]==1)
 			{
 				cout<<i<<"号 ";
			 }
		 }
		 cout<<endl;
		 cout<<"已死玩家(不一定被狼人所杀)：";
		for(int i=1;i<=10;i++)
		{
			if(condition[i]==1)
			{
				cout<<i<<"号 ";
			}
		 } 
		 cout<<endl;
 		cin>>f>>x;
 		if(killedbywolf[x]==0&&f==1||(f==0&&condition[x]==1))
 		{
 			x=0;
			cout<<"TA不是被狼人杀死的/不能鞭尸";
			_sleep(2000); 
			system("cls");
		 }
 		system("cls");
	 }
	 if(f==1)
	 {
	 	id[player[x]]++;
	 	condition[x]=0;
	 	saved[++snum]=x;
	 }
	 else{
	 		died[++dnum]=x;
			id[player[x]]--;
			condition[x]=1;
	 }
 }
void witch_computer()
{
	    if(mustsave1!=0)
	    {
	    	id[player[mustsave1]]++;
		 	condition[mustsave1]=0;
		 	saved[++snum]=mustsave1;
		 	mustsave1=0;
		}
		else  if(mustsave2!=0)
		{
			id[player[mustsave2]]++;
		 	condition[mustsave2]=0;
		 	saved[++snum]=mustsave2;
		 	mustsave2=0;
		}
		else{
         int f=rand()%2;
         int x=rand()%10;
         x++;
     	 if(f==1)
		 {
		 	while(killedbywolf[x]==0)
		 	x=rand()%10+1;
		 	id[player[x]]++;
		 	condition[x]=0;
		 	saved[++snum]=x;
		 }
		 else{
		 	while(condition[x]==1)
		 	x=rand()%10+1;
		 		died[++dnum]=x;
				id[player[x]]--;
				condition[x]=1;
		 }
	   }
}
void prophet_player()
{
	int x=0;
	while(x<=0||x>10||condition[x]==1) 
	{
		cout<<"您要查验谁的身份：";
		
		cin>>x;
		
		system("cls");
	}
	cout<<"TA是"<<player[x]<<endl;
	_sleep(2000);
	system("cls"); 
}
void prophet_computer(int n)
{
	int x=rand()%10+1;
	while(x==n||condition[x]==1)
	{
		x=rand()%10+1;
	}
	if(player[x]=="狼人"||player[x]=="白狼王")
	{
		is_say=rand()%10;
		when_say=rand()%3+1; 
		say_who=x;
		say_what=player[x]; 
		who_say=n;
	}
}
 bool checker()
 {
 	int sum=0,h=0,b=0;
 	for(int i=1;i<=10;i++)
 	{
 		if(condition[i]==0)
 		{
 			if(player[i]=="狼人"||player[i]=="白狼王")
	 		{
	 			b++;
			 }
			 else{
			 	h++;
			 }
		 }
 		sum++;
 		
	 }
	 return sum>1&&b!=0&&h!=0;
 }
 void night()
 {
 	   
 	   memset(vote,0,sizeof(vote));
	 	cout<<"天黑请闭眼..."<<endl; 
		_sleep(1000);
		 system("cls");
		 cout<<"魔术师请睁眼"; 
		 if(me=="魔术师")
		 {
		 	magician_player();
		 }
		 for(int i=1;i<=9;i++)
		 {
		 	if(player[i]=="魔术师"&&condition[i]==0)
		 	{
		 		magician_computer();
			 }
		 }
		 _sleep(2000);
		 system("cls");
		 cout<<"您现在的身份是："<<player[10];
		 me=player[10];
		 _sleep(2000);
		 system("cls");
		 cout<<"预言家请睁眼"; 
		 if(me=="预言家")
		 {
		 	prophet_player();
		 }
		 for(int i=1;i<=9;i++)
		 {
		 	if(player[i]=="预言家"&&condition[i]==0)
		 	{
		 		prophet_computer(i);
			 }
		 }
		 _sleep(2000);
		 system("cls");
		 cout<<"狼人请睁眼"<<endl; 
		 int ff=0;
		 for(int i=1;i<=9;i++)
		 {
		 	if(player[i]=="狼人"&&condition[i]==0)
		 	{
		 		ff=i;
		 		break;
			 }
	     }
		 int rar=rand()%2;
		 if(rar==0&&ff!=0&&me=="狼人")
		 {
		 	cout<<"叮~你有队友的来信"<<endl;
		 	int rrr=rand()%2;
		 	if(rrr==0)
		 	{
		 		int rr=-100000;
				int minn=-1000000;
				for(int i=1;i<=9;i++)
				{
					if(i!=ff&&condition[i]==0&&trust[i]>minn&&player[i]!="狼人")
					{
						minn=trust[i];
						rr=i;
					}
				}
				cout<<rr<<"号铁好人,刀他";
		    }
		    else{
		    	cout<<"加油，这把一定苟到最后！";
			}
		 }
		 if(me=="狼人")
		 {
		     wolfman_player();	  
		 } 
		 for(int i=1;i<=9;i++)
		 {
		 	if(player[i]=="狼人"&&condition[i]==0)
		 	{
		 		wolfman_computer();
			 }
			 
		 }
		 if(rar==0)
		 {
		 	_sleep(3000);
		 }
		 _sleep(2000);
		 system("cls"); 
		 cout<<"狼王请睁眼"<<endl; 
		 if(me=="白狼王")
		 {
		 	 wolfking_player();
		  } 
		  for(int i=1;i<=9;i++)
		 {
			 if(player[i]=="白狼王"&&condition[i]==0)
			 {
			 	wolfman_computer();
			 }
		 }
		  _sleep(2000);
		  system("cls");
		  cout<<"女巫请睁眼"<<endl;
		  ff=0;
			 for(int i=1;i<=9;i++)
			 {
			 	if(player[i]=="女巫"&&condition[i]==0)
			 	{
			 		ff=i;
			 		break;
				 }
		     }
		  rar=rand()%2;
		 if(rar==0&&ff!=0&&me=="女巫")
		 {
		 	cout<<"叮~你有队友的来信"<<endl;
		 	int rrr=rand()%2;
		 	int rr=-100000;
		 	if(rrr==0)
		 	{
		 		if(smart[ff]>6)
		 		{
			 			
						int minn=1000000;
						for(int i=1;i<=9;i++)
						{
							if(i!=ff&&condition[i]==0&&trust[i]<minn&&player[i]!="女巫")
							{
								minn=trust[i];
								rr=i;
							}
						}
				 }
		 		 else
		 		 {
		 		 	
		 		 	 rr=rand()%9+1;
		 		 	 while(condition[rr]==1||rr==ff)
		 		 	 rr=rand()%9+1;
				  }
				cout<<rr<<"号，我想一定是坏人,刀他!";
		    }
		    else{
		    	if(condition[10]==1)
		    	{
		    		cout<<"你今晚死了，你没法自救，我救你"; 
		    		mustsave1=10;
		    		int ffff=rand()%2;
		    		if(ffff==0)
		    		{
		    			cout<<"如果我也死了，请务必救我QAQ";
					}
				}
		    	else
		    	cout<<"加油，这把一定苟到最后！";
			}
		 }
		 if(rar==0)
		 _sleep(2000);
		  if(me=="女巫")
		 {
		 	 witch_player();
		 } 
		 for(int i=1;i<=9;i++)
		 {
		 	if(player[i]=="女巫"&&condition[i]==0)
		 	{
		 		witch_computer();
			 }
		 }
		 _sleep(2000);
		 system("cls");
		 return ;
 }
 void day()
 {
 	if(!checker())
 	{
 		if(id["狼人"]>0||id["白狼王"]>0)
		 {
		 	cout<<"狼人胜利！"; 
		 }
		 else{
		 	cout<<"平民胜利！"; 
		 }
		 return ;
	 }
 	int endging1=0;
 	cout<<"天亮了，象征平安的太阳升起来了..."<<endl;
 	rest=9;
 	for(int i=1;i<=10;i++)
	{
		if(condition[i]==1)
		{
			rest--;
		}
	 }
 	snum++;
 	while(--snum)
 	{
 		if(condition[saved[snum]]==0)
 		cout<<"昨夜，"<<saved[snum]<<"号玩家被救，不公布TA的身份"<<endl; 
 		if(saved[snum]==10) 
 		endging1=1;
	 }
	 int tmpp=0;
 	dnum++;
 	while(--dnum)
 	{
 		if(condition[died[dnum]]==1)
 		{
 			cout<<"昨夜，"<<died[dnum]<<"号玩家死了"<<endl;
 		    cout<<"TA是"<<player[died[dnum]]<<endl; 
 		    condition[died[dnum]]=1;
 		    tmpp++; 
		 }
 		
	 }
	 if(condition[10]==1)
	 {
	 	cout<<"而你，我的朋友，很遗憾，你死了..."<<endl;
	 	if(player[10]=="女巫")
	 	{
	 		if(endging1==1)
	 		{
	 			cout<<"达成结局1：天煞的猪队友"<<endl; 
	 			strin[++last_strin]="达成结局1：天煞的猪队友";
			 }
			 else{
			 	cout<<"达成结局2：无法自救"<<endl;
			 	strin[++last_strin]="达成结局2：无法自救";
			 }
		 }
		 else{
		 	cout<<"达成结局3：在劫难逃"<<endl;
		 	strin[++last_strin]="达成结局3：在劫难逃";
		 }
	 	if(me=="猎人"||me=="白狼王")
		 {
		 	int man=0;
			while(man>9||man<1||condition[man]==1)
			{
				cout<<"您要带走谁:"<<endl;
				cin>>man;
				system("cls");
			}
			cout<<"TA是"<<player[man]<<endl; 
			_sleep(1000);
			system("cls");
			died[++dnum]=man;
			id[player[man]]--;
			condition[man]=1;
			
			if(!checker)
			{
				if(id["狼人"]>0||id["白狼王"]>0)
				 {
				 	cout<<"狼人胜利！"<<endl; 
				 	if(me=="白狼王")
				 	{
				 		cout<<"达成结局4：孤注一掷（狼人版）"<<endl;
				 		strin[++last_strin]="达成结局4：孤注一掷（狼人版）";
					 }
					 else{
					 	cout<<"达成结局5：赌圣失败（好人版）"<<endl;
					 	strin[++last_strin]="达成结局5：赌圣失败（好人版）";
					 }
				 }
				 else{
				 	cout<<"平民胜利！"; 
				 	if(me=="白狼王")
				 	{
				 		cout<<"达成结局6：赌圣失败（狼人版）"<<endl;
				 		strin[++last_strin]="达成结局6：赌圣失败（狼人版）";
					 }
					 else{
					 	cout<<"达成结局7：孤注一掷（好人版）"<<endl;
					 	strin[++last_strin]="达成结局7：孤注一掷（好人版）";
					 }
				 }
			 } 
			 else 
			 {
			 	if(me=="猎人")
				 {
				 	cout<<"达成结局8：无异平民"<<endl;
				 	strin[++last_strin]="达成结局8：无异平民";
				  } 
				  else{
				  	cout<<"达成结局9：无异狼人"<<endl;
				  	strin[++last_strin]="达成结局9：无异狼人"; 
				  }
			  } 
		  } 
	 	return ; 
	  } 
	  unordered_map<string,int> wordrepeat;
	  int cntword=0;
	cout<<"请自由讨论并投票(投票时号码请用阿拉伯数字)..."<<endl;
	int mastvoten=0;
	while(cntword<min(rest,(long long)3))
	{
		
		int r=rand()%9+1,t=rand()%6;
		while(condition[r]==1||wordrepeat[word[t]]==1)
		r=rand()%9+1,t=rand()%6;
		int idsay=rand()%5;
		int bad=0;
		int ttt=-1;
		if(idsay==0&&id_has_said[r]==0)
		{
			t=rand()%6; 
			 
		 } 
		 else{
		 	if(player[r]=="狼人"||player[r]=="白狼王")
		 	{
		 		ttt=rand()%smart[r];
		 		if(ttt==0)
		 		{
		 			ttt=rand()%6;
		 			bad=1;
				 }
			 }
		 }
		 if(bad==1&&condition[r]==0)
		 {
		 	cout<<r<<"号："<<www[t]<<endl;
		 }
		 else if(condition[r]==0&&wordrepeat[id_word[t]]==0&&t!=4&&id_has_said[r]==0)
		 {
		 	id_has_said[r]=1;
		    cout<<r<<"号："<<id_word[t]<<endl;
		 }
		 else if(condition[r]==0&&wordrepeat[id_word[t]]==0&&t==4&&id_has_said[r]==0)
		 {
		 	id_has_said[r]=1;
		 	int rrrrr=0;
			 while(rrrrr<=0||rrrrr>=10||condition[rrrrr]==1||rrrrr==r)
			 {
			 	rrrrr=rand()%9+1;
			  } 
           cout<<r<<"号："<<id_word[t];
			  cout<<rrrrr<<"号是狼人"<<endl;
			  voten[rrrrr].push_back(r);
			  if(votewrong[r])
			  {
			  	votewrong[r]=0;
			  	int rrrrrr=0;
			  	while(rrrrrr<=0||rrrrrr>=10||condition[rrrrrr]==1||rrrrrr==rrrrr)
				 {
				 	rrrrrr=rand()%9+1;
				  } 
				  cout<<rrrrrr<<"号：你别叭叭了，都说错过一次了..."<<endl ;
				  vote[rrrrrr]=r;
			  }
			  int is_kill=rand()%3;
			 if(is_kill==0)
			 {
			 	int saywhat=rand()%2;
			 	if(saywhat==0)
				 {
				 	int rrrr=0;
				 	while(rrrr<=0||rrrr>=10||condition[rrrr]==1||rrrr==r)
				 	rrrr=rand()%9+1; 
				 	cout<<rrrr<<"号："<<"真哒？"<<endl;;
				    vote[rrrr]=rrrrr;
				    for(int kk=1;kk<=2/trust[r];kk++)
				    vote[rand()%9+1]=rrrrr;
				  } 
				  if(saywhat==1)
				  {
				  	int rrrr=0;
				 	while(rrrr<=0||rrrr>=10||condition[rrrr]==1||rrrr==r)
				 	rrrr=rand()%9+1; 
				 	cout<<rrrr<<"号："<<"一眼狼人好吧"<<endl;
				 	vote[rrrr]=r; 
				 	voten[r].push_back(rrrr);
				   } 
			  } 
			  else if(is_kill==1)
			  {
			  	int issave=rand()%3;
				if(issave==0)
				{
					if(mustsave1==0)
					{
						mustsave1=r;
					}
					else{
						mustsave2=r;
					}
				 } 
				 int iskill=rand()%2;
				if(iskill==0)
				{
					if(mustkill1==0)
					{
						mustkill1=r;
					}
					else{
						mustkill2=r;
					}
				 }
			  } 
		 }
		 if(condition[r]==0&&wordrepeat[id_word[t]]==0&&t==1&&id_has_said[r]==0)
		 {
		 	id_has_said[r]=1;
		 	cout<<r<<"号："<<word[t]<<endl;
		 	wordrepeat[word[t]]=1;
		 	cntword++;
		 	_sleep(100);
		 	int flag=rand()%2;
		 	if(flag==1&&tmpp>2)
		 	{
                int ra=0;
                while(ra<=0||ra>=10||condition[ra]==1)
	 			ra=rand()%(r-1);
	 			ra++;
	 			int ran=rand()%2;
	 			if(ran==0)
	 			{
	 				cout<<r-ra<<"号:"<<"不是哥们，就算你是女巫，你晚上也杀人啊"<<endl; 
	 				vote[r-ra]=r;
					vote[rand()%9+1]=r;
					vote[rand()%9+1]=r;
				 }
				 else{
				 	cout<<r-ra<<"号："<<"那你以后别乱杀人了..."<<endl;
				 	vote[r-ra]=r;
					vote[rand()%9+1]=r;
				 }
			 }
		 }
		 if(condition[r]==0&&wordrepeat[word[t]]==0&&idsay!=0)
		 {
		 	cout<<r<<"号："<<word[t]<<endl;
		 	wordrepeat[word[t]]=1;
		 	cntword++;
		 }
		 _sleep(1000);
		 if(cntword==when_say&&is_say!=0&&condition[who_say]==0)
		 {
		 	cout<<who_say<<"号："<<"我是预言家"<<say_who<<"号是"<<say_what<<endl;
		 	voten[say_who].push_back(who_say);
		 	is_say=0;
			 int is_kill=rand()%3;
			 if(is_kill==0)
			 {
			 	int saywhat=rand()%2;
			 	if(saywhat==0)
				 {
				 	int rrrr=0;
				 	while(rrrr<=0||rrrr>=10||condition[rrrr]==1||rrrr==who_say)
				 	rrrr=rand()%9+1; 
				 	cout<<rrrr<<"号："<<"真哒？"<<endl;;
				    vote[rrrr]=say_who;
				    for(int kk=1;kk<=2/trust[who_say];kk++)
				    vote[rand()%9+1]=say_who;
				  } 
				  if(saywhat==1)
				  {
				  	int rrrr=0;
				 	while(rrrr<=0||rrrr>=10||condition[rrrr]==1||rrrr==who_say)
				 	rrrr=rand()%9+1; 
				 	cout<<rrrr<<"号："<<"一眼狼人好吧"<<endl;
				 	vote[rrrr]=who_say; 
				   } 
			  } 
			  else if(is_kill==1)
			  {
			  	int issave=rand()%3;
				if(issave==0)
				{
					if(mustsave1==0)
					{
						mustsave1=who_say;
					}
					else{
						mustsave2=who_say;
					}
				 } 
				 int iskill=rand()%2;
				if(iskill==0)
				{
					if(mustkill1==0)
					{
						mustkill1=who_say;
					}
					else{
						mustkill2=who_say;
					}
				 }
			  }
		 }
	}
	int ran;
	while(ran<=0||ran>9||condition[ran]==1)
	ran= rand()%9+1;
	cout<<ran<<"号："<<"10号，你说两句呗"<<endl;
	string playerword;
	cin>>playerword;
	if(playerword.find("我")!=-1&&playerword.find("是")!=-1&&playerword.find("狼")!=-1&&playerword.find("号")==-1&&(playerword.find("刀")==-1||playerword.find("被刀")!=-1)&&(playerword.find("杀")==-1||playerword.find("被杀")!=-1)&&playerword.find("毒死")==-1&&playerword.find("不")==-1)
	{
		int ran;
		while(ran<=0||ran>9||condition[ran]==1)
		ran= rand()%9+1;
		cout<<ran<<"号："<<"逆天，但我投了"<<endl;
		if(vote[ran]==0) 
		vote[ran]=10;
	}
	else if((playerword.find("狼")!=-1||(playerword.find("坏")!=-1&&playerword.find("不")==-1))&&playerword.find("号")!=-1)
	{
		if(ten)
		{
			ten=0;
			int ran=0;
			while(ran<=0||ran>9||condition[ran]==1)
		    ran= rand()%9+1;
		    cout<<ran<<"号："<<"你可别叭叭了，都说错过一次了"<<endl;
			voten[10].push_back(ran);
			vote[ran]=10;
			while(ran<=0||ran>9||condition[ran]==1)
		    ran= rand()%9+1;
		    voten[10].push_back(ran);
		    vote[ran]=10;
		}
		else{
			int tmppp=playerword[playerword.find("号")-1]-'0';
			voten[tmppp].push_back(10);
			int ran;
			while(ran<=0||ran>9||condition[ran]==1||ran==tmppp)
			ran= rand()%9+1;
			if(condition[tmppp]==0)
			{
				int isvote=rand()%2;
				if(isvote&&vote[ran]==0 ) 
				vote[ran]=playerword[playerword.find("号")-1]-'0';
				while(ran<=0||ran>9||condition[ran]==1||ran==tmppp)
				ran= rand()%9+1;
				isvote=rand()%2;
				if(isvote&&vote[ran]==0) 
				vote[ran]=playerword[playerword.find("号")-1]-'0';
				while(ran<=0||ran>9||condition[ran]==1||ran==tmppp)
				ran= rand()%9+1;
				cout<<ran<<"号："<<"尊嘟假嘟？"<<endl;
				int rrr=rand()%3;
				for(int i=1;i<=2/trust[10];i++)
				{
					ran= rand()%9+1;
					isvote=rand()%2;
					if(isvote&&vote[ran]==0) 
					vote[ran]=10;
				}
				rrr=rand()%3;
				if(rrr==0)
				{
					cout<<tmppp<<"号："<<"啊对对对，我是狼，睁眼的时候我还看见你了呢!"<<endl; 
					ran= rand()%9+1;
					int isvote=rand()%2;
					if(isvote&&vote[ran]==0) 
					vote[ran]=10;
					ran= rand()%9+1;
					isvote=rand()%2;
					if(isvote&&vote[ran]==0) 
					vote[ran]=10;
				}
				else if(rrr==1)
				{
					
					if(player[tmppp]=="平民"||player[tmppp]=="猎人"||player[tmppp]=="魔术师"||player[tmppp]=="女巫"||player[tmpp]=="预言家")
					cout<<tmppp<<"号："<<"自爆了，我是"<<player[tmppp]<<"，好人们保我！"<<endl;
					int issave=rand()%2;
					if(issave==0)
					{
						if(mustsave1==0)
						{
							mustsave1=tmppp;
						}
						else{
							mustsave2=tmppp;
						}
					 } 
					 int iskill=rand()%3;
					if(iskill==0)
					{
						if(mustkill1==0)
						{
							mustkill1=tmppp;
						}
						else{
							mustkill2=tmppp;
						}
					 } 
				 } 
				 else if(rrr==2)
				 {
				 	cout<<tmppp<<"号："<<"笑死，这么着急投我一个好人，你不会是狼吧？"<<endl;
				 }
			}
			
			else
			cout<<ran<<"号："<<"TA不是都死了吗..."<<endl;
		}
		
		
		
	}
	else if(playerword.find("我")!=-1&&(playerword.find("我是")!=-1||playerword.find("真的是")!=-1||playerword.find("才是")!=-1)&&playerword.find("狼")==-1) 
	{
		int ran;
		while(ran<=0||ran>9||condition[ran]==1)
		ran= rand()%9+1;
		int isvote=rand()%2;
		if(isvote==1)
		{
			cout<<ran<<"号："<<"OK"<<endl;
		}
		else{
			cout<<ran<<"号："<<"胡说八道！一眼狼人"<<endl ;
			if(vote[ran]==0) 
			vote[ran]=10;
		}
	}
	else if(playerword.find("是")!=-1&&playerword.find("号")!=-1&&(playerword.find("吗")!=-1||playerword.find("么")!=-1||playerword.find("？")!=-1)&&playerword.find("活")==-1&&playerword.find("死")==-1)
	{
		int tmppp=playerword[playerword.find("号")-1]-'0';
		int randd=rand()%3;
		if(randd==0) 
		cout<<tmppp<<"号："<<"我是好人！"<<endl; 
		else if(randd==1)
		{
			cout<<tmppp<<"号："<<"我是好人，但你是不是就不一定了..."<<endl; 
			if(vote[tmppp]==0)
			{
				vote[tmppp]=10;
			}
		}
		else
		cout<<tmppp<<"号："<<"你蠢啊，你这样问，哪怕我真是坏人我也不可能说啊，10号一眼萌新"<<endl; 
	 } 
	 else if(playerword.find("6")!=-1&&playerword.find("号")==-1)
	 {
	 	int tmpppp=rand()%9+1;
	 	while(condition[tmpppp]==1)
	 	{
	 		tmpppp=rand()%9+1;
		 }
		 cout<<tmpppp<<"号："<<"你别6了，说正事"<<endl; 
	 }
	else{
		int speak=rand()%5;
		int ran=0;
		while(ran<=0||ran>9||condition[ran]==1)
		 ran= rand()%9+1;
		if(speak==0)
		{
			cout<<ran<<"号："<<"..."<<endl; 
		}
		if(speak==1)
		{
			cout<<ran<<"号："<<"未知全貌，不予置评"<<endl;
			if(vote[ran]==0) 
			vote[ran]=10;
		}
		if(speak==2)
		{
			cout<<ran<<"号："<<"额..."<<endl;
		}
		if(speak==3)
		{
			cout<<ran<<"号："<<"阿巴阿巴阿巴"<<endl;
		}
		if(speak==4)
		{
			cout<<ran<<"号："<<"这边建议多说点直白有用的话..."<<endl; 
		}
	}
	for(int i=1;i<=9;i++)
	{
		
		if(vote[i]==0&&condition[i]==0)
		{
			while(condition[vote[i]]==1||vote[i]==0||vote[i]==i)
			vote[i]=rand()%9+1;
		}
	 } 
	 int who=0;
	 while(who<1||who>9||condition[who]==1)
	 {
	 	cout<<"请问您要投给几号？"<<endl;
	 	cin>>who;
	 	system("cls");
	 }
	 vote[10]=who;
	 int voteans[15];
	 memset(voteans,0,sizeof(voteans));
	 int kicked,numberofvote=-1;
	 for(int i=1;i<=10;i++)
	{
		voteans[vote[i]]++;
		cout<<i<<"号";
		if(vote[i]==0)
		{
			cout<<"未投"<<endl;
		}
		else{
			cout<<"投给了"<<vote[i]<<"号"<<endl; 
		}
	 } 
	 for(int i=1;i<=10;i++)
	{
		if(voteans[i]>numberofvote)
		{
			numberofvote=voteans[i];
			kicked=i;
		}
		cout<<i<<"号："<<voteans[i]<<"票"<<endl;
		
	 } 
	 cout<<kicked<<"号玩家被放逐"<<endl;
	id[player[kicked]]--;
	condition[kicked]=1;
	cout<<"TA是"<<player[kicked]<<endl;
	if(player[kicked]!="狼人"&&player[kicked]!="白狼王")
	{
		for(int i=1;i<=10;i++)
		if(vote[i]==kicked)
		trust[i]--;
		for(auto e:voten[kicked])
		{
			votewrong[e]=1;
			if(e==10)
			{
				ten=1;
			}
		}
	} 
	else{
		for(int i=1;i<=10;i++)
		if(vote[i]==kicked)
		trust[i]+=2;
	}
	if((player[kicked]=="猎人"||player[kicked]=="白狼王")&&kicked!=10)
	{
		int rr=-100000;
		int minn=1000000;
		for(int i=1;i<=10;i++)
		{
			if(i!=kicked&&condition[i]==0&&trust[i]<minn)
			{
				minn=trust[i];
				rr=i;
			}
		}
		cout<<"TA带走了"<<rr<<"号玩家"<<endl; 
		id[player[rr]]--;
	    condition[rr]=1;
	    cout<<"TA带走的玩家是"<<player[rr]<<endl;
	    if(rr==10)
		{
			cout<<"我的朋友，很遗憾，你死了...";
			return ;
		 } 
	}
	 if(kicked==10)
	 {
	 	cout<<"很遗憾，您被放逐了..."<<endl;
	 	cout<<"达成结局10：惨遭抛弃"<<endl;
	  } 
	  _sleep(5000);
	  system("cls"); 
	  for(int i=1;i<=10;i++) 
	  {
	  	killedbywolf[i]=0;
	  	vote[i]=0;
	  }
	  vote[rand()%9+1]=ansss;
	  vote[rand()%9+1]=ansss;
	  dnum=0;
 }
signed main()
{
	std::thread musicThread(music);
	srand(time(0));
	cout<<"在16世纪时，人们都会相信有女巫、吸血鬼之类的存在。"<<endl;
	cout<<"如果你是被冠上“狼人”或者“女巫”的罪名，"<<endl;
	cout<<"都会被处以火刑，或者是被银器杀死。"<<endl;
	cout<<"因此村民就开始决定要找出藏在他们中的狼人..."<<endl;
	_sleep(5000);
	 system("cls");
	 while(1)
	 {
	 	system("cls");
	 	int op=0;
	 	while(op<=0||op>5)
	 	{
	 		cout<<"输入1，查看获得的结局"<<endl;
		 	cout<<"输入2，查看获得的成就"<<endl;
		 	cout<<"输入3，输入密码开启礼包"<<endl;
		 	cout<<"输入4，打开商店" <<endl;
		 	cout<<"输入5，开始游戏"<<endl;
		 	cin>>op;
		 	 system("cls");
		 }
	 	  system("cls");
	 	  if(op==1)
	 	  {
	 	  	  cout<<"您总共玩了"<<last_strin<<"次狼人杀"<<endl;
	 	  	  for(int i=1;i<=last_strin;i++)
	 	  	  {
	 	  	  	    cout<<strin[i]<<endl;
				 }
				 _sleep(last_strin*1000+300);
		   }
		   if(op==2)
		   {
		   	   cout<<"您一共有"<<reward<<"个成就"<<endl; 
		   	   for(int i=1;i<=reward;i++)
		   	   {
		   	   	    cout<<rewards[i]<<endl;
				  }
				  _sleep(reward*1000+300);
		   }
		   if(op==3)
		   {
		   	  gift();
		   }
		   if(op==4)
		   {
		   	    store();
		   }
	 	  if(op==5)
	 	  {
	 	  	memset(id_has_said,0,sizeof(id_has_said));
	 	  	memset(condition,0,sizeof(condition));
	 	  	memset(id_has_said,0,sizeof(id_has_said));
	 	  	for(int i=0;i<7;i++)
	 	  	{
	 	  		id[s[i]]=0;
			   }
	 	  	cout<<"狼人杀现在开始：包括您共有十位玩家，此游戏共有11种结局，祝您游玩愉快(您是十号玩家)"<<endl;
			 _sleep(3000);
			 system("cls"); 
			 
			 r=rand()%7;
			 if(nxtid!=-1)
			 {
			 	r=nxtid;
			 	nxtid=-1;
			 }
			 cout<<"您的身份是："<<s[r]<<endl;
			 _sleep(2000);
			 system("cls");
			 me=s[r];
			 id[me]++;
			 for(int i=1;i<=9;i++)
			 {
			 	smart[i]=rand()%10+1;
			 	r=rand()%7;
				 while((id[s[r]]==2&&s[r]=="平民")||(id[s[r]]==2&&(s[r]=="女巫"||s[r]=="狼人"))||(id[s[r]]==1&&(s[r]=="魔术师"||s[r]=="猎人"||s[r]=="白狼王"||s[r]=="预言家")))
			 	{
			 		r=rand()%7;
				 } 
				 player[i]=s[r];
				 id[s[r]]++;
				 trust[i]=10; 
			 }
			 trust[10]=10;
			 player[10]=me;
			 while(checker())
			 {
			     night();
			     if(checker()) 
			     day();
			     else{
			     	break;
				 }
			     if(condition[10]==1)
			     {
			     	break;
				 }
			 }
			 if(condition[10]==0)
			 {
			 	if(id["狼人"]>0||id["白狼王"]>0)
				 {
				 	cout<<"狼人胜利！"; 
				 }
				 else{
				 	cout<<"平民胜利！"; 
				 }
				 cout<<"达成结局11：苟到最后" ;
				 no_die++;
				 strin[++last_strin]="达成结局11：苟到最后"; 
				 cout<<"狼人杀币+50"<<endl;
			 }
			 
			 mony+=50;
			 _sleep(3000);
			 system("cls"); 
			  if(last_strin==1)
			 	{
			 		cout<<"恭喜您获得成就 新手：玩完一次游戏"<<endl;
					 rewards[++reward] ="新手";
				 }
			 	if(last_strin==3)
			 	{
			 		cout<<"恭喜您获得成就 老玩家LV.1：玩完三次游戏"<<endl;
					 rewards[++reward] ="老玩家LV.1";
				 }
				 if(last_strin==5)
			 	{
			 		cout<<"恭喜您获得成就 老玩家LV.2：玩完五次游戏"<<endl;
					 rewards[++reward] ="老玩家LV.2";
				 }
				 if(last_strin==5)
			 	{
			 		cout<<"恭喜您获得成就 老玩家LV.3：玩完十次游戏"<<endl;
					 rewards[++reward] ="老玩家LV.3";
				 }
				 if(no_die==1&&condition[10]==0)
			 	{
			 		cout<<"恭喜您获得成就 不死圣人LV.1：苟到最后1次"<<endl;
					 rewards[++reward] ="不死圣人LV.1";
				 }
				 if(no_die==3&&condition[10]==0)
			 	{
			 		cout<<"恭喜您获得成就 不死圣人LV.2：苟到最后3次"<<endl;
					 rewards[++reward] ="不死圣人LV.2";
				 }
				 if(no_die==5&&condition[10]==0)
			 	{
			 		cout<<"恭喜您获得成就 不死圣人LV.3：苟到最后5次"<<endl;
					 rewards[++reward] ="不死圣人LV.3";
				 }
			   }
			  
			_sleep(3000);
			 system("cls");
	 }
	musicThread.join();
}
