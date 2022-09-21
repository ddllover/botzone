#include<iostream>//使用kingmove queenmove 估计局面
#include<cstring>
#include<cstdio>
#include<algorithm>
using namespace std;
#define black 1;//黑为1
#define white -1;//白为-1
int interface[8][8]={0};//8*8的棋盘，详见棋盘设定
int nnext[8][2]={{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};//所有的走法
int Tmpchess1[8][8]={0},Tmpchess2[8][8]={0};
int currcolor;//棋盘（8*8）的设定：0为可走点  2为障碍物
int depth=1;
struct Map{   
    int mqueen,mking;
    int hqueen,hking;
}map[8][8]={0};//标记每个点的评估
struct way{   
    int x;
    int y;
    int step;
}value[70]={0};//计算每个点的step
struct Amazons{
    int mx0;
    int my0;
    int mx1;
    int my1;
    int mx2;
    int my2;
}bot[5]={0};//记录自己要走法 和 敌方的位置
int seekmore(int alpha,int beta,int tier);
bool judge();
//void tryzone(int tier);
//void tryone(int i,int j,int color,int tier);//模拟落子
//void trytwo(int i,int j,int tier);//模拟箭点
void queen(int i,int j);//计算queenmove
void king(int i,int j);//计算kingmoeve
int realvalue();//评估局面
bool inmap(int x,int y);//在地图里面
bool canmove(int i,int j);//判断结束
bool step(int x0,int y0,int x1,int y1,int x2,int y2,int color);//判断结束
int main(){
    int x0,y0,x1,y1,x2,y2;  //白棋 黑棋定位
    interface[2][0]=interface[5][0]=interface[0][2]=interface[7][2]=black;
    interface[2][7]=interface[5][7]=interface[0][5]=interface[7][5]=white;
    int times; cin>>times;  currcolor=white; //简单交互
    if(times>8) depth=2;
    if(times>15) depth=3;
    if(times>25) depth=4;
    if(times>35) depth=6;
    for(int i=0;i<times;i++){
        cin>>x0>>y0>>x1>>y1>>x2>>y2;
        if(x0==-1)  {currcolor=black;}
        else  step(x0,y0,x1,y1,x2,y2,-currcolor);
        if(i<times-1){ cin>>x0>>y0>>x1>>y1>>x2>>y2;
            if(x0>=0) step(x0,y0,x1,y1,x2,y2,currcolor);}
    }       
            //bot[0].mv=INT32_MIN;
            seekmore(INT32_MIN,INT32_MAX,0);
            cout<<bot[0].mx0<<" "<<bot[0].my0<<" "<<bot[0].mx1<<" "<<bot[0].my1<<" "<<bot[0].mx2<<" "<<bot[0].my2<<endl;
    return 0;
}
bool canmove(int i,int j){
    int temp1=0,temp2=0;//temp1和temp2是为了判断是否为可动棋子
    for(int k=0;k<8;k++){//temp2是障碍数
        int ti=i,tj=j;
        ti+=nnext[k][0];
        tj+=nnext[k][1];
        if(ti>=0&&ti<8&&tj<8&&tj>=0){
            temp1++;
            if(interface[ti][tj]!=0) temp2++;
        }
    }
    return(temp1>temp2);
}
bool step(int x0,int y0,int x1,int y1,int x2,int y2,int color){//让对方下棋
    bool temp=false;
    if(inmap(x0,y0)&&inmap(x1,y1)&&inmap(x2,y2)){
        if(interface[x0][y0]==color){
            if(interface[x1][y1]==0){
                if(interface[x2][y2]==0||interface[x2][y2]==color){
                    interface[x0][y0]=0;
                    interface[x1][y1]=color;
                    interface[x2][y2]=2;
                    temp=true;
                }
            }
        }
    }
    return(temp);
}
bool inmap(int x,int y){//在地图里面
    bool temp=false;
    if(x>=0&&x<8&&y>=0&&y<8)
    temp=true;
    return(temp);
}
void queen(int i,int j){
    int ti,tj;//找到queenmove的值
	int head=0,tail=0;
	value[tail].x=i;  value[tail].y=j;	value[tail].step=0;  //队列初始化 
	tail++;
	while(head<tail){
		for(int k=0;k<8;k++)//一次head可以标记一步可达的点
		{   ti=value[head].x;
            tj=value[head].y;
            while(1){
                ti=ti+nnext[k][0];
                tj=tj+nnext[k][1];
                if(ti>=0&&ti<8&&tj>=0&&tj<8&&Tmpchess1[ti][tj]==-1) continue;
                if(ti>=0&&ti<8&&tj>=0&&tj<8&&Tmpchess1[ti][tj]==0){
                    Tmpchess1[ti][tj]=-1;
                    value[tail].x=ti;
                    value[tail].y=tj;
                    value[tail].step=value[head].step+1;
                    tail++;
                }
                else break;
            }
		}
		head++;//head++就是转一次弯
	}
	for(int k=1;k<tail;k++){
	if(interface[i][j]==currcolor && map[value[k].x][value[k].y].mqueen>value[k].step)
		map[value[k].x][value[k].y].mqueen=value[k].step;
	if(interface[i][j]==-currcolor&& map[value[k].x][value[k].y].hqueen>value[k].step)
		map[value[k].x][value[k].y].hqueen=value[k].step;
    }
}
void king(int i,int j){
 int ti,tj;
	int head=0,tail=0;
	value[tail].x=i;  value[tail].y=j;	value[tail].step=0;  //队列初始化 
	tail++;
	while(head<tail){
		for(int k=0;k<8;k++)
		{
			ti=value[head].x+nnext[k][0];//每个方向一次只能行进一步
			tj=value[head].y+nnext[k][1];
			if(ti<0||tj>7||tj<0||ti>7||Tmpchess2[ti][tj]!=0)
				continue;
			else{
				Tmpchess2[ti][tj]=1;
				value[tail].x=ti;
				value[tail].y=tj;
				value[tail].step=value[head].step+1;
				tail++;
			}
		}
		head++;
	}
	for(int k=1;k<tail;k++){
	if(interface[i][j]==currcolor && map[value[k].x][value[k].y].mking>value[k].step)
		map[value[k].x][value[k].y].mking=value[k].step;
	if(interface[i][j]==-currcolor&& map[value[k].x][value[k].y].hking>value[k].step)
		map[value[k].x][value[k].y].hking=value[k].step;
    }
}
int realvalue(){
    for(int i=0;i<8;i++){
                for(int j=0;j<8;j++){
                    map[i][j].hking=map[i][j].hking=30;//先对评估初始化好提高好比较
                    map[i][j].hqueen=map[i][j].mqueen=100;
                }
            }
    int sumking=0,sumqueen=0;
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(interface[i][j]==currcolor||interface[i][j]==-currcolor){
                            for(int x=0;x<8;x++){
                                for(int y=0;y<8;y++){
                                    if(interface[x][y]==0)
                                    Tmpchess1[x][y]=Tmpchess2[x][y]=0;
                                    else Tmpchess1[x][y]=Tmpchess2[x][y]=2;
                                }
                            }
                            queen(i,j);
                            king(i,j);
                        }
        }
    }
    for(int i=0;i<8;i++)			
			for(int j=0;j<8;j++)
				if(interface[i][j]==0){
					if(map[i][j].mking<map[i][j].hking) sumking+=5;
					if(map[i][j].mking>map[i][j].hking) sumking-=5;
                    if(map[i][j].hking==map[i][j].mking&&map[i][j].hking!=30) sumking+=2;
					if(map[i][j].mqueen<map[i][j].hqueen) sumqueen+=9;
					if(map[i][j].mqueen>map[i][j].hqueen) sumqueen-=9;
                    if(map[i][j].mqueen==map[i][j].hqueen&&map[i][j].hqueen!=100) sumqueen+=3;
                    /*sumking+=map[i][j].mking-map[i][j].hking;
                    sumqueen+=2*map[i][j].mqueen-2*map[i][j].mking;*/
                }
    if(depth<=2) sumking*=0.9;
    if(depth>=2) sumqueen*=0.9;
    return(sumking+sumqueen);
}//只会计算自己的局面值  对对方来说局面值为负
int seekmore(int alpha,int beta,int tier){
    if(tier==depth) return(realvalue());
    if(!judge()) return(realvalue());
    int color;
    if(tier==0||tier%2==0){color=currcolor;}
    else color=-currcolor;
        for(int i=0;i<8;i++)//一层循环寻找的自己的可动子
        for(int j=0;j<8;j++)
            if(interface[i][j]==color)
                if(canmove(i,j)){
                    int ti,tj;
                    interface[i][j]=0;//起子
                    for(int k=0;k<8;k++){//寻找所有落点并尝试
                        ti=i,tj=j;
                        while(1){
                            ti+=nnext[k][0];
                            tj+=nnext[k][1];
                            if(ti>=0&&ti<8&&tj<8&&tj>=0&&interface[ti][tj]==0){
                                interface[ti][tj]=color;//落子
                                int ki,kj;
                                for(int l=0;l<8;l++){//寻找所有障碍并尝试
                                    ki=ti;kj=tj;
                                    while(1){
                                        ki+=nnext[l][0];
                                        kj+=nnext[l][1];
                                        if(ki>=0&&ki<8&&kj<8&&kj>=0&&interface[ki][kj]==0){
                                            interface[ki][kj]=2;//障碍物
                                            int temp=seekmore(alpha,beta,tier+1);//深搜
                                            interface[ki][kj]=0;
                                            //极大极小加剪枝
                                            if(currcolor==color){
                                                if(temp>alpha){
                                                    alpha=temp;
                                                    if(tier==0){
                                                    bot[0].mx0=i,bot[0].my0=j;
                                                    bot[0].mx1=ti,bot[0].my1=tj;
                                                    bot[0].mx2=ki,bot[0].my2=kj;
                                                    }
                                                }
                                                if(alpha>=beta){
                                                     interface[ti][tj]=0;
                                                     interface[i][j]=color;
                                                    return alpha;
                                                }
                                            }
                                            else{
                                                if(temp<beta){
                                                    beta=temp;
                                                }
                                                if(alpha>=beta){
                                                    interface[ti][tj]=0;
                                                    interface[i][j]=color;
                                                    return beta;
                                                }
                                            }
                                        }
                                        else break;
                                    }
                                }
                                interface[ti][tj]=0;
                            }
                            else break;
                        }
                    }
                    interface[i][j]=color;
                }
    if(currcolor==color) return alpha;
    return beta;
}
bool judge(){
    int ccount=0;
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(interface[i][j]==currcolor)
            if(canmove(i,j)) 
            ccount++;
        }
    }
    return(ccount);
}
