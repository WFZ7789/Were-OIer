#include<bits/stdc++.h>
using namespace std;
#define int long long
string s[15]={"平民","狼人","魔术师","猎人","女巫","白狼王","预言家"};
string word[15]={"如果我说我是好人你信吗？ 如果我说我不是狼人你们也不会信对吧？ 如果我死，无所谓投就投吧","大家千万不要投我是猎人，我死带错人就惨了。","从我看，我们应该集中精力找出真正的狼人，而不是争吵起来。我们要紧密合作，互相分享信息，才能找到线索。","朋友们，我是女巫，是平民的最强战力，把我都投出去了，你们咋办？","我的牌是狼人，但是被魔术师换了...","坏了，这把是平民毫无体验感，快杀我，我要重开！","啊啊啊女巫，女巫，保护我们啊","魔术师，魔术师！我要换牌，谁爱当平民谁当","(看手机)(查百度)(照着念)我是一位值得信赖的朋友，我是好人，请大家相信我","I人冒泡","我是预言家"};
string player[15];
string me;
int r; 
map<string,int> id;
int condition[105];
int killedbywolf[105];
int kbwnum;
int died[105],dnum; 
int vote[105]; 
int mustkill1,mustkill2,mustsave1,mustsave2;
bool is_say=0;
int when_say,say_who,who_say;
string say_what;
int saved[105],snum;
int rest=10; 
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
	while(man>9||man<1)
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
	while(man>9||man<1)
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
 	int man=rand()%10;
 	man++;
 	killedbywolf[man]=1;
 	died[++dnum]=man;
 	id[player[man]]--;
 	condition[man]=1;
 }
 void magician_player()
 {
 	int x=0,y=0;
 	while(x<1||x>10||y<1||y>10)
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
 		for(int i=1;i<=10;i++)
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
 		if(killedbywolf[x]==0&&f==1)
 		{
 			x=0;
			cout<<"TA不是被狼人杀死的";
			_sleep(2000); 
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
		 		died[++dnum]=x;
				id[player[x]]--;
				condition[x]=1;
		 }
	   }
}
void prophet_player()
{
	int x=0;
	while(x<=0||x>10) 
	{
		cout<<"您要查验谁的身份：";
		cin>>x;
	}
	cout<<"TA是"<<player[x]<<endl;
	_sleep(2000);
	system("cls"); 
}
void prophet_computer(int n)
{
	int x=rand()%10+1;
	while(x==n)
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
 	int sum=0;
 	for(int i=0;i<6;i++)
 	{
	    if(i==5)
	    {
	    	if(id[s[1]]>0)
	    	{
	    		continue;
			}
		}
 		if(id[s[i]]>0)
 		{
 			sum++;
		 }
	 }
	 return sum>1;
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
		  cout<<"女巫请睁眼";
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
			 }
			 else{
			 	cout<<"达成结局2：自杀传奇"<<endl;
			 }
		 }
		 else{
		 	cout<<"达成结局3：在劫难逃"<<endl;
		 }
	 	if(me=="猎人"||me=="白狼王")
		 {
		 	int man=0;
			while(man>9||man<1)
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
					 }
					 else{
					 	cout<<"达成结局5：赌圣失败（好人版）"<<endl;
					 }
				 }
				 else{
				 	cout<<"平民胜利！"; 
				 	if(me=="白狼王")
				 	{
				 		cout<<"达成结局6：赌圣失败（狼人版）"<<endl;
					 }
					 else{
					 	cout<<"达成结局7：孤注一掷（好人版）"<<endl;
					 }
				 }
			 } 
			 else 
			 {
			 	if(me=="猎人")
				 {
				 	cout<<"达成结局8：无异平民"<<endl;
				  } 
				  else{
				  	cout<<"达成结局9：无异狼人"<<endl;
				  }
			  } 
		  } 
	 	return ; 
	  } 
	  unordered_map<string,int> wordrepeat;
	  int cntword=0;
	cout<<"请自由讨论并投票(投票时号码请用阿拉伯数字)..."<<endl;
	while(cntword<min(rest,(long long)3))
	{
		int r=rand()%9,t=rand()%11;
		 r++;
		 if(condition[r]==0&&wordrepeat[word[t]]==0&&t==10)
		 {
		 	
		 	int rrrrr=0;
			 while(rrrrr<=0||rrrrr>=10||condition[rrrrr]==1||rrrrr==r)
			 {
			 	rrrrr=rand()%9+1;
			  } 
           cout<<r<<"号："<<word[t];
			  cout<<rrrrr<<"号是狼人"<<endl;
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
				    vote[rand()%9+1]=rrrrr;
				    vote[rand()%9+1]=rrrrr;
				    vote[rand()%9+1]=rrrrr;
				    vote[rand()%9+1]=rrrrr;
				  } 
				  if(saywhat==1)
				  {
				  	int rrrr=0;
				 	while(rrrr<=0||rrrr>=10||condition[rrrr]==1||rrrr==r)
				 	rrrr=rand()%9+1; 
				 	cout<<rrrr<<"号："<<"一眼狼人好吧"<<endl;
				 	vote[rrrr]=r; 
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
		 if(condition[r]==0&&wordrepeat[word[t]]==0&&t==3)
		 {
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
		 if(condition[r]==0&&wordrepeat[word[t]]==0)
		 {
		 	cout<<r<<"号："<<word[t]<<endl;
		 	wordrepeat[word[t]]=1;
		 	cntword++;
		 }
		 _sleep(1000);
		 if(cntword==when_say&&is_say!=0)
		 {
		 	cout<<who_say<<"号："<<"我是预言家，"<<say_who<<"号是"<<say_what<<endl;
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
				    vote[rand()%9+1]=say_who;
				    vote[rand()%9+1]=say_who;
				    vote[rand()%9+1]=say_who;
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
	while(ran<0||ran>9||condition[ran]==1)
	ran= rand()%9+1;
	cout<<ran<<"号："<<"10号，你说两句呗"<<endl;
	string playerword;
	cin>>playerword;
	if(playerword.find("我")!=-1&&playerword.find("是")!=-1&&playerword.find("狼")!=-1&&playerword.find("号")==-1&&(playerword.find("刀")==-1||playerword.find("被刀")!=-1)&&(playerword.find("杀")==-1||playerword.find("被杀")!=-1)&&playerword.find("毒死")==-1&&playerword.find("不")==-1)
	{
		int ran;
		while(ran<0||ran>9||condition[ran]==1)
		ran= rand()%9+1;
		cout<<ran<<"号："<<"逆天，但我投了"<<endl;
		if(vote[ran]==0) 
		vote[ran]=10;
		int rr=rand()%10;
		if(rr==0)
		{
			ran= rand()%9+1;
		    cout<<ran<<"号："<<"跟票"<<endl;
		    if(vote[ran]==0) 
		    vote[ran]=10;
		 } 
	}
	else if((playerword.find("狼")!=-1||(playerword.find("坏")!=-1&&playerword.find("不")==-1))&&playerword.find("号")!=-1)
	{
		
		int tmppp=playerword[playerword.find("号")-1]-'0';
		int ran;
		while(ran<0||ran>9||condition[ran]==1||ran==tmppp)
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
			if(rrr==1)
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
			else if(rrr==2)
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
	else if(playerword.find("我")!=-1&&(playerword.find("我是")!=-1||playerword.find("真的是")!=-1||playerword.find("才是")!=-1)&&playerword.find("狼")==-1) 
	{
		int ran;
		while(ran<0||ran>9||condition[ran]==1)
		ran= rand()%9+1;
		int isvote=rand()%2;
		if(isvote==1)
		{
			cout<<ran<<"号："<<"算了，信一次吧..."<<endl;
		}
		else{
			cout<<ran<<"号："<<"不是，你这也太老套了吧，一眼狼人"<<endl ;
			if(vote[ran]==0) 
			vote[ran]=10;
		}
	}
	else if(playerword.find("是")!=-1&&playerword.find("号")!=-1&&(playerword.find("吗")!=-1||playerword.find("么")!=-1||playerword.find("？")!=-1))
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
			cout<<ran<<"号："<<"你可疑！"<<endl;
			if(vote[ran]==0) 
			vote[ran]=10;
		}
		if(speak==2)
		{
			cout<<ran<<"号："<<"我也这么觉得"<<endl;
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
			while(condition[vote[i]]==1)
			vote[i]=rand()%9+1;
		}
	 } 
	 int who=0;
	 while(who<1||who>9)
	 {
	 	cout<<"请问您要投给几号？"<<endl;
	 	cin>>who;
	 }
	 vote[10]=who;
	 int voteans[15];
	 memset(voteans,0,sizeof(voteans));
	 int kicked,numberofvote=-1;
	 for(int i=1;i<=10;i++)
	{
		voteans[vote[i]]++;
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
	if(player[kicked]=="猎人"||player[kicked]=="白狼王")
	{
		int rr=rand()%10;
		rr++;
		while(rr==kicked)
		rr=rand()%10+1;
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
	  _sleep(3000);
	  system("cls"); 
	  for(int i=1;i<=10;i++) 
	  {
	  	killedbywolf[i]=0;
	  	vote[i]=0;
	  }
	  dnum=0;
 }
signed main(int32_t argc = 0)
{
	if(argc != 0)	srand(argc);
	else srand(time(nullptr));
	cout<<"在16世纪时，人们都会相信有女巫、吸血鬼之类的存在。"<<endl;
	cout<<"如果你是被冠上“狼人”或者“女巫”的罪名，"<<endl;
	cout<<"都会被处以火刑，或者是被银器杀死。"<<endl;
	cout<<"因此村民就开始决定要找出藏在他们中的狼人..."<<endl;
	_sleep(5000);
	 system("cls");
	cout<<"狼人杀现在开始：包括您共有十位玩家，此游戏共有11种结局，祝您游玩愉快(您是十号玩家)"<<endl;
	 _sleep(3000);
	 system("cls"); 
	 r=rand()%7;
	 cout<<"您的身份是："<<s[r]<<endl;
	 _sleep(2000);
	 system("cls");
	 me=s[r];
	 id[me]++;
	 for(int i=1;i<=9;i++)
	 {
	 	r=rand()%7;
		 while((id[s[r]]==2&&s[r]=="平民")||(id[s[r]]==2&&(s[r]=="女巫"||s[r]=="狼人"))||(id[s[r]]==1&&(s[r]=="魔术师"||s[r]=="猎人"||s[r]=="白狼王"||s[r]=="预言家")))
	 	{
	 		r=rand()%7;
		 } 
		 player[i]=s[r];
		 id[s[r]]++;
		 
	 }
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
	     	return 0;
		 }
	 }
	 if(id["狼人"]>0||id["白狼王"]>0)
	 {
	 	cout<<"狼人胜利！"; 
	 }
	 else{
	 	cout<<"平民胜利！"; 
	 }
	 cout<<"达成结局11：苟到最后" ;
	
}
