#include <iostream>
#include <cstring>
#include <ctime>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include<sstream>

#define Test 0//本地运行
#if Test
#include "C:\Users\Wangyan\Desktop\bot\mahjong\jsoncpp\json.h"
#include "C:\Users\Wangyan\Desktop\bot\mahjong\Chinese-Standard-Mahjong-master\fan-calculator-usage\Mahjong-GB-CPP\MahjongGB\MahjongGB.h"
#else 
#include "MahjongGB/MahjongGB.h"
#include "jsoncpp/json.h"
#endif

////////////////////测试ok
using namespace std;
const double C0 = 1.44;
const double limit_time = CLOCKS_PER_SEC * 0.07;
const double all_limtime = CLOCKS_PER_SEC * 0.7;
double start_time;
vector<string> request, response;
Json::Value inputJSON, outputJSON;  //输入 //输出

vector<string> string_hand;    //暗牌  肯定有一个vector 算番器需要
vector<int> int_hand;           //mcts 使用  //
int array_hand[50] = { 0 };         //估值用

typedef pair<string, int> pair1;
typedef pair<string, pair1> pair2;
vector< pair2 > pack[4];//各方的明牌

int array_played_card[50] = { 0 };//打出的牌   手牌 明牌在其中
int all_card = 60;
vector<int> int_played_card; //剩余的牌   除去自己牌和明牌

int hua_card[4] = { 0 };//每个人的花牌数量
int turnID, myPlayerID, quan;  //输入指令的个数  自己的id  场风

//下面是一些预定义的常量，但是我也不知道准不准。。。。。。
double singleZI = 2.0;//单个的字牌扣分
double pairZI = 0.75;//一对字牌的得分
double KE = 2.0;//刻的得分
double SHUN = 4.0;//顺子的得分

void ZIvalue(int i, double& value) {//这个函数是对编号是i的字牌进行估值操作，在Value函数使用。
	if (array_hand[i] == 0)
		return;
	if (array_hand[i] == 1) {//手里字牌只有一张，这样的牌不好，应该尽快出手，估值下降
		value -= singleZI;
		return;
	}
	else if (array_hand[i] == 2) {//手里有两张字牌
		switch (array_played_card[i]) {
		case 0:value += pairZI;//没有出过这张字牌，价值比较大
		case 1:value += 0.5 * pairZI;//出过一张了，价值减半？
		case 2:value -= 1.5 * singleZI;//不可能再形成箭刻，用处很小
		}

	}
	else {//已经形成了刻，获得2番，估值增加
		value += 3 * KE;//箭刻的价值比单独的数字刻高，不过系数3是我估计的
	}
}

int numSHUN(int i) {//统计以i为中心的顺子的个数
	if (array_hand[i - 1] == 0 || array_hand[i] == 0 || array_hand[i + 1] == 0)
		return 0;
	if ((array_hand[i - 1] == 1 || array_hand[i] == 1 || array_hand[i + 1] == 1) && (array_hand[i - 1] >= 1 || array_hand[i] >= 1 || array_hand[i + 1] >= 1))
		return 1;
	if ((array_hand[i - 1] == 2 || array_hand[i] == 2 || array_hand[i + 1] == 2) && (array_hand[i - 1] >= 2 || array_hand[i] >= 2 || array_hand[i + 1] >= 2))
		return 2;
	return 3;
}

double Value() {
	double value = 0.0;
	int ting_num = 0;//听牌数，指有多少种牌能和手里的牌吃碰，即ting_arr中不为零的个数
	int ting_total = 0;//听牌在牌堆中还有多少张，是ting_arr的所有数的和
	int ting_arr[50] = { 0 };//每种牌的听牌数量

	for (int i = 0; i < 19; i += 9) {//饼条万的的双头牌
		for (int j = 1; j < 7; j++) {
			if (array_hand[i + j] == 1 && array_hand[i + j + 1] == 1 && array_hand[i + j - 1] == 0 && array_hand[i + j + 2] == 0) {
				ting_arr[i + j - 1] = 4 - array_played_card[i + j - 1];
				ting_arr[i + j + 2] = 4 - array_played_card[i + j + 2];

			}
		}
		//对于12和89，因为他们分别只听3和7，所以单独计算
		if (array_hand[i + 0] == 1 && array_hand[i + 1] == 1 && array_hand[i + 2] == 0)
			ting_arr[i + 2] = 4 - array_played_card[i + 2];
		if (array_hand[i + 7] == 1 && array_hand[i + 8] == 1 && array_hand[i + 6] == 0)
			ting_arr[i + 6] = 4 - array_played_card[i + 6];
	}

	//下面处理嵌张的听牌数
	for (int i = 0; i < 19; i += 9) {
		for (int j = 0; j < 7; j++) {
			if (array_hand[i + j] == 1 && array_hand[i + j + 2] == 1 && array_hand[i + j + 1] == 0) {
				ting_arr[i + j + 1] = 4 - array_played_card[i + j + 1];
			}
		}
	}

	//对于字牌，如果有机会形成箭刻、圈风刻和门风刻，我们就保留，否则就降低估值
	for (int i = 31; i <= 33; i++) {//中发白，箭刻
		ZIvalue(i, value);
	}
	ZIvalue(27 + myPlayerID, value);//门风
	ZIvalue(27 + quan, value);//圈风

	//下面是对顺子的加分
	for (int i = 0; i < 19; i += 9) {
		for (int j = 1; j < 8; j++) {
			int tmp = numSHUN(i + j);
			value += tmp * SHUN;
			if (tmp != 0)
				j += 2;
		}
	}

	//对于刻，
	for (int i = 0; i < 31; i++) {//数字刻
		if (i != 27 + myPlayerID && i != 27 + quan && array_hand[i] >= 3) {//这里去掉了圈风刻门风刻
			value += KE;
			if (i == 0 || i == 8 || i == 9 || i == 17 || i == 18 || i == 26 || i > 26)
				value += 0.5 * KE;//试着给幺九刻加分？
		}
	}
	for (int i = 0; i < 42; i++) {
		if (ting_arr[i] > 0)
			ting_num++;//统计一下听多少种牌
		ting_total += ting_arr[i];
	}
	value += ting_num;
	//printf("ting_num = %d, ting_total = %d\n", ting_num, ting_total);
	value += ting_total * 0.1;//如果能听的牌张数很多，听牌数或许更容易减少

	return value;
}

class OP {//建立 B9 0~8 T9 9~17 W9 18~26 F4 27~ 30 J3 31~33 H8 34~41
public:
	string str[50];
	OP() :str{ "B1","B2","B3","B4","B5","B6","B7","B8","B9","T1","T2","T3","T4","T5","T6","T7","T8","T9","W1","W2","W3","W4","W5","W6","W7","W8","W9"
	,"F1","F2","F3","F4","J1","J2","J3","H1","H2","H3","H4","H5","H6","H7","H8" } {}
	int operator()(const string a) {
		if (a[0] == 'B') {
			return(a[1] - '1');
		}
		else if (a[0] == 'T') {
			return(a[1] - '1' + 9);
		}
		else if (a[0] == 'W') {
			return(a[1] - '1' + 18);
		}
		else if (a[0] == 'F') {
			return(a[1] - '1' + 27);
		}
		else if (a[0] == 'J') {
			return(a[1] - '1' + 31);
		}
		else if (a[0] == 'H') {
			return(a[1] - '1' + 34);
		}
		return 0;
	}
}op;

namespace judge { ////6种鸣牌操作  总共7种操作
	int HU(int card1, bool isZIMO = false, bool isGANG = false) {
		bool isJUEZHANG = false, isLAST = false;
		if (card1 != 42 && array_played_card[card1] - array_hand[card1] == 4) isJUEZHANG = true;
		if (all_card == 144) isLAST = true;
		string_hand.clear();
		for(int i=0;i<42;i++){
			for(int j=0;j<array_hand[i];i++){
				string_hand.push_back(op.str[i]);
			}
		}
		if (card1 == 42) string_hand.erase(find(string_hand.begin(), string_hand.end(), request[turnID].substr(2, 2)));
		string temp;

		if (card1 != 42) {
			temp = op.str[card1];
		}
		else temp = request[turnID].substr(2, 2);

		vector<pair<int, string> > ans;
		try {
			ans = MahjongFanCalculator(pack[myPlayerID], string_hand, temp, hua_card[myPlayerID], isZIMO, isJUEZHANG, isGANG, isLAST, myPlayerID, quan);
			/*for (auto i : re) {
				cout << i.first << " " << i.second << endl;
			}*/
		}
		catch (const string& error) {
			//cout << error<<"0 or fan error"<<endl<<"-----"<<endl;
			return 0;
		}
		int fan = 0;
		for (auto i : ans) {
			fan += i.first;
		}
		return fan;
	}
	bool PENG(int card1) {
		if (array_hand[card1] == 2) return 1;//2   为碰
		return 0;
	}

	bool BUGANG(int card1) {
		for (vector< pair2 >::iterator i = pack[myPlayerID].begin(); i < pack[myPlayerID].end(); i++) {
			if (i->first == "PENG" && i->second.first == op.str[card1]) {
				return 1;
			}
		}
		return 0;
	}

	bool GANG(int card1) {
		if (array_hand[card1] >= 3) return 1;//3  为杠 暗杠
		return 0;
	}

	bool CHI_1(int card1, int id) {//吃 碰 杠
		if (card1 <= 26 && (id + 1) % 4 == myPlayerID && card1 + 2 <= 42 && card1 / 9 == (card1 + 2) / 9 && array_hand[card1 + 1] >= 1 && array_hand[card1 + 2] >= 1) return 1;//1 吃 必须是上家打出的
		return 0;
	}

	bool CHI_2(int card1, int id) {
		if (card1 <= 26 && (id + 1) % 4 == myPlayerID && card1 - 1 >= 0 && (card1 - 1) / 9 == (card1 + 1) / 9 && card1 + 1 <= 42 && array_hand[card1 - 1] >= 1 && array_hand[card1 + 1] >= 1) return 1;
		return 0;
	}

	bool CHI_3(int card1, int id) {
		if (card1 <= 26 && (id + 1) % 4 == myPlayerID && card1 - 2 >= 0 && (card1 - 2) / 9 == card1 / 9 && array_hand[card1 - 2] >= 1 && array_hand[card1 - 1] >= 1) return 1;
		return 0;
	}
}

void play_card(int card1, int card2) {
	if (card1 != 42) {
		all_card++;
		array_hand[card1]++;
		int_played_card.erase(find(int_played_card.begin(), int_played_card.end(), card1));
		array_played_card[card1]++;
	}
	//int_hand.erase(find(int_hand.begin(), int_hand.end(), card2));
	array_hand[card2]--;

}

void back_card(int card1, int card2) {
	if (card1 != 42) {
		all_card--;
		array_hand[card1]--;
		int_played_card.push_back(card1);
		array_played_card[card1]--;
	}
	array_hand[card2]++;
}

class state {
public:
	state* son_state[43][42];// 起牌 出牌
	double vis, win;//vis节点经历的次数 ， win时basic_val的累和
	double basic_val; //节点的估值  节点的胜率0~1
	double uct_val;   //uct节点选择     再次访问时更新 
	state() {
		memset(son_state, 0, sizeof(son_state));
		vis = 1;
		win = 0;
		basic_val = Value() / 10; //估值函数  
		uct_val = 0;
	}
	state* get_son(int card1, int card2) {
		if (!son_state[card1][card2]) {
			play_card(card1, card2);
			son_state[card1][card2] = new state;
			back_card(card1, card2);
		}
		return son_state[card1][card2];
	}
	double val(state* son) {  //同时更新
		//return basic_val;
		if (!son) return -1;
		son->vis++;
		if (son->vis != 0)
			son->uct_val = son->win / son->vis + C0 * pow(log(vis) / son->vis, 0.5);
		else son->uct_val = 100;  //未访问的节点怎么办？
		return son->uct_val;
	}
	int best_son(int card1) {
		int max_val_card = -1;
		double max_val =-111111111111111111 ;
		for(int i=0;i<42;i++){
			for(int j=0;j<array_hand[i];i++){
				double temp = val(get_son(card1, i));
				if (temp > max_val) {
					max_val = temp;
					max_val_card = i;
				}
			}
		}
		return max_val_card;
	}
	~state() {
	}
};

double Mcts(state* son, int deep) {
    if(clock() < start_time + all_limtime) return son->basic_val;
	if (deep == 5) return son->basic_val;
	//if (son->basic_val == 1)  return(son->basic_val);

	int card1, card2;
	if (deep == 0) {//&&(request[turnID][0]=='2'||request[turnID].find("PLAY")!=std::string::npos)
		card1 = 42;
	}
	else if (all_card < 144) card1 = int_played_card[rand() % int_played_card.size()];
	else return 0;
	double itemp = judge::HU(card1)/4.0;
	if (itemp) return itemp;  //八番和直接带走
	card2 = son->best_son(card1);
	play_card(card1, card2);
	itemp = Mcts(son->son_state[card1][card2], deep + 1);
	back_card(card1, card2);
	son->win += itemp;
	son->vis++;
	return itemp;
}

namespace decide {
	void play_chi(int card1, int type) {
		pack[myPlayerID].push_back(pair2("CHI", pair1(op.str[card1 - type], 2 + type)));
		if (type != -1) {
			array_hand[card1 - type - 1]--;
		}
		if (type != 0) {
			array_hand[card1 - type]--;
		}
		if (type != 1) {
			array_hand[card1 - type + 1]--;
		}
	}
	void back_chi(int card1, int type) {
		pack[myPlayerID].pop_back();
		if (type != -1) {
			//int_hand.push_back(card1 - type - 1);
			array_hand[card1 - type - 1]++;
		}
		if (type != 0) {
			//int_hand.push_back(card1 - type);
			array_hand[card1 - type]++;
		}
		if (type != 1) {
			//int_hand.push_back(card1 - type + 1);
			array_hand[card1 - type + 1]++;
		}
	}
	void CHI(state* root, int card1, int type) {
		//pack 需要改一下而且可能有多种选择
		play_chi(card1, type);
		double start_time0 = clock();
		while (clock() < start_time0 + limit_time) {
			Mcts(root, 0);
		}
		back_chi(card1, type);
	}

	void play_peng(int card1) {
		pack[myPlayerID].push_back(pair2("PENG", pair1(op.str[card1], request[turnID][2])));
		/*for<int>::iterator i = int_hand.begin(); i != int_hand.end();) {
			if (*i == card1) i = int_hand.erase(i);
			else i++;
		}*/
		array_hand[card1] -= 2;
	}
	void back_peng(int card1) {
		pack[myPlayerID].pop_back();
		//for (int i = 0; i < 2; i++)
		//	int_hand.push_back(card1);
		array_hand[card1] += 2;
	}
	void PENG(state* root, int card1) {
		play_peng(card1);
		double start_time0 = clock();
		while (clock() < start_time0 + limit_time) {
			Mcts(root, 0);
		}
		back_peng(card1);
	}

	void paly_gang(int card1, int angang = 0) {
		pack[myPlayerID].push_back(pair2("GANG", pair1(op.str[card1], request[turnID][2])));
		/*for (vector<int>::iterator i = int_hand.begin(); i != int_hand.end();) {
			if (*i == card1) i = int_hand.erase(i);
			else i++;
		}*/
		array_hand[card1] -= (3 + angang);
	}
	void back_gang(int card1, int angang = 0) {
		//pack[myPlayerID].erase(find(pack[myPlayerID].begin(),pack[myPlayerID].end(),pair1(op.str[card1], request[turnID][2])));
		pack[myPlayerID].pop_back();
		//for (int i = 0; i < 3; i++)
		//	int_hand.push_back(card1);
		array_hand[card1] += (3 + angang);
	}
	void GANG(state* root, int card1, int angang = 0) {//杠 暗杠
		paly_gang(card1, angang);
		double start_time0 = clock();
		while (clock() < start_time0 + limit_time) {
			Mcts(root, 1);
		}
		back_gang(card1, angang);
	}

	void paly_bugang(int card1) {
		for (vector< pair2 >::iterator i = pack[myPlayerID].begin(); i < pack[myPlayerID].end(); i++) {
			if (i->first == "PENG" && i->second.first == op.str[card1]) {
				i->first = "GANG";
				break;
			}
		}
		//int_hand.erase(find(int_hand.begin(), int_hand.end(), card1));
		array_hand[card1]--;
	}
	void back_bugang(int card1) {
		for (vector< pair2 >::iterator i = pack[myPlayerID].begin(); i < pack[myPlayerID].end(); i++) {
			if (i->first == "GANG" && i->second.first == op.str[card1]) {
				i->first = "PENG";
				break;
			}
		}
		//int_hand.push_back(card1);
		array_hand[card1]++;
	}
	void BUGANG(state* root, int card1) {
		paly_bugang(card1);
		double start_time0 = clock();
		while (clock() < start_time0 + limit_time) {
			Mcts(root, 1);
		}
		back_bugang(card1);
	}

	void PASS(state* root, int deep) {
		double start_time0 = clock();
		while (clock() < start_time0 + limit_time) {
			Mcts(root, deep);
		}
	}
}
void init();
int main()
{           //连糊不糊都不知道的一个样例
	start_time = clock();
	MahjongInit();
#if Test
	freopen("C:/Users/Wangyan/Desktop/bot/mahjong/mybot/sample2.txt", "r", stdin);
	//freopen("C:/Users/Wangyan/Desktop/bot/mahjong/mybot/sampleout2.txt", "w", stdout);//or ifstream cin("bot/mahjong/sample2.txt");
#endif
	srand(time(nullptr));//设置随机种子
	init();
	//出牌部分
	if (turnID >= 2) {
		if (request[turnID][0] == '2') {//代表摸牌了 所以要打出去  这里存在补杠 暗杠
			if (judge::HU(42, true) >= 8) response.push_back("HU");
			else {
				int card1 = op(request[turnID].substr(2, 2));
				state* root = new state[3];
				int _count = 0;
				bool Judge[3];
				Judge[0] = true;
				array_hand[card1]--;
				Judge[1] = judge::GANG(card1); array_hand[card1]++;
				Judge[2] = judge::BUGANG(card1);
				while (clock() < start_time + all_limtime) {
					_count = (_count + 1) % 3;
					switch (_count) {
					case 0:decide::PASS(&root[0], 0); break;
					case 1:if (Judge[1]) decide::GANG(&root[1], card1, 1); break;
					case 2:if (Judge[2]) decide::BUGANG(&root[2], card1); break;
					default: break;
					}
				}
				int card2 = 0; double maxval = -100; int marki = 0;
				for (int i = 0; i < 3; i++) {
					double temp;
					if (Judge[i]) {
						switch (i) {
						case 0:temp = root[i].val(&root[i]);//估值前面没进行变化
							if (temp > maxval) {
								maxval = temp;
								marki = i;
								card2 = root[marki].best_son(42);
							}
							break;
						case 1:decide::paly_gang(card1, 1);
							temp = root[i].val(&root[i]);//估值前面没进行变化
							if (temp > maxval) {
								maxval = temp;
								marki = i;
							}
							decide::back_gang(card1, 1);
							break;
						case 2:decide::paly_bugang(card1);
							temp = root[i].val(&root[i]);//估值前面没进行变化
							if (temp > maxval) {
								maxval = temp;
								marki = i;
							}
							decide::back_bugang(card1);
							break;
						}
					}
				}
				switch (marki) {
				case 0:response.push_back("PLAY " + op.str[card2]); break;
				case 1:response.push_back("GANG " + op.str[card1]); break;
				case 2:response.push_back("BUGANG " + op.str[card1]); break;
				}
			}
		}
		else {//这里要判断吃，碰，杠 怎么把他们与起牌弄到一起
			int card1 = -1;
			if (request[turnID][2] - '0' != myPlayerID) {
				if (request[turnID].find("PLAY") != std::string::npos || request[turnID].find("PENG") != std::string::npos) card1 = op(request[turnID].substr(9, 2));
				if (request[turnID].find("CHI") != std::string::npos) card1 = op(request[turnID].substr(11, 2));
				if (card1 != -1) {
					if (judge::HU(card1) >= 8) response.push_back("HU");////
					else if (request[turnID].substr(4, 6) == "BUGANG" && judge::HU(op(request[turnID].substr(11, 2)), false, true) >= 8) response.push_back("HU");
					else {
						state* root = new state[6];
						int _count = 0;
						bool Judge[6];
						Judge[1] = judge::GANG(card1);  Judge[2] = judge::PENG(card1);
						Judge[3] = judge::CHI_1(card1, request[turnID][2] - '0'); Judge[4] = judge::CHI_2(card1, request[turnID][2] - '0');
						Judge[5] = judge::CHI_3(card1, request[turnID][2] - '0'); Judge[0] = true;
						if (Judge[1] || Judge[2] || Judge[3] || Judge[4] || Judge[5]) {
							while (clock() < start_time + all_limtime) {
								_count = (_count + 1) % 6;
								switch (_count) {
								case 0:decide::PASS(&root[0], 1); break;
								case 1:if (Judge[1]) decide::GANG(&root[1], card1); break;
								case 2:if (Judge[2]) decide::PENG(&root[2], card1); break;
								case 3:if (Judge[3]) decide::CHI(&root[3], card1, -1); break;
								case 4:if (Judge[4]) decide::CHI(&root[4], card1, 0); break;
								case 5:if (Judge[5]) decide::CHI(&root[5], card1, 1); break;
								default:break;
								}
							}
						}
						int card2 = 0; double maxval = -100; int marki = 0;
						for (int i = 0; i < 6; i++) {
							double temp;
							if (Judge[i]) {
								switch (i) {
								case 0:  temp = root[i].val(&root[i]);
									if (temp > maxval) {
										maxval = temp;	marki = i;
									} break;
								case 1: decide::paly_gang(card1);
									temp = root[i].val(&root[i]);
									if (temp > maxval) {
										maxval = temp;	marki = i;
									}
									decide::back_gang(card1);
									break;
								case 2:	decide::play_peng(card1);
									temp = root[i].val(&root[i]);
									if (temp > maxval) {
										maxval = temp;	marki = i;
										card2 = root[i].best_son(42);
									}
									decide::back_peng(card1);
									break;
								case 3:decide::play_chi(card1, -1);
									temp = root[i].val(&root[i]);
									if (temp > maxval) {
										maxval = temp;	marki = i;
										card2 = root[i].best_son(42);
									}
									decide::back_chi(card1, -1);
									break;
								case 4:decide::play_chi(card1, 0);
									temp = root[i].val(&root[i]);
									if (temp > maxval) {
										maxval = temp;	marki = i;
										card2 = root[i].best_son(42);
									}
									decide::back_chi(card1, 0);
									break;
								case 5:decide::play_chi(card1, 1);
									temp = root[i].val(&root[i]);
									if (temp > maxval) {
										maxval = temp;	marki = i;
										card2 = root[i].best_son(42);
									}
									decide::back_chi(card1, 1);
									break;
								}
							}
						}
						switch (marki) {
						case 0:response.push_back("PASS"); break;
						case 1:response.push_back("GANG"); break;
						case 2:response.push_back("PENG " + op.str[card2]); break;
						case 3:response.push_back("CHI " + op.str[card1 + 1] + " " + op.str[card2]); break;
						case 4:response.push_back("CHI " + op.str[card1] + " " + op.str[card2]); break;
						case 5:response.push_back("CHI " + op.str[card1 - 1] + " " + op.str[card2]); break;
						}
					}
				}
				else response.push_back("PASS");
			}
			else  response.push_back("PASS");
		}
	}
	//输出交互
	outputJSON["response"] = response[turnID];
	cout << outputJSON << endl;
	//调试用的
#if Test
	sort(int_hand.begin(), int_hand.end());
	for (auto i : int_hand) {
		cout << op.str[i] << ' ';
	}
	cout << endl << clock() - start_time;
#endif
	return 0;
}
void init() {
	//把所有信息读入
	for (int i = 34; i <= 41; i++) {
		array_played_card[i] = 1;
	}
	cin >> inputJSON;
	turnID = inputJSON["responses"].size();
	for (int i = 0; i < turnID; i++) {
		request.push_back(inputJSON["requests"][i].asString());
		response.push_back(inputJSON["responses"][i].asString());
	}
	request.push_back(inputJSON["requests"][turnID].asString());
	//梳理信息
	if (turnID < 2) {
		response.push_back("PASS");
		return;
	}
	istringstream sin;
	string stemp;
	int itemp, last_id;
	sin.str(request[0]);
	sin >> itemp >> myPlayerID >> quan;
	sin.clear();
	sin.str(request[1]); sin >> itemp;
	for (int j = 0; j < 4; j++) {
		sin >> itemp;
		hua_card[j] = itemp;

	}
	for (int j = 0; j < 13; j++) {
		sin >> stemp;
		array_hand[op(stemp)]++;
		array_played_card[op(stemp)]++;
	}  //花牌的种类不用管
	for (int i = 2; i <= turnID; i++) {
		sin.clear();
		sin.str(request[i]);
		sin >> itemp;
		if (itemp == 2) {
			//起牌
			sin >> stemp;
			//int_hand.push_back(op(stemp));
			array_hand[op(stemp)]++;
			array_played_card[op(stemp)]++;
			sin.clear();
			all_card++;
			if (i == turnID) break;
			sin.str(response[i]);
			sin >> stemp;
			if (stemp == "GANG") {
				sin >> stemp;
				array_hand[op(stemp)] = 0;
				//array_played_card[op(stemp)] += 4;
			}
			else { //int_hand.erase(find(int_hand.begin(), int_hand.end(), op(stemp)));
				sin >> stemp;
				array_hand[op(stemp)]--;
				//array_played_card[op(stemp)]++;
			}
		}
		if (itemp == 3) {
			string sstemp;
			if (request[i - 1].find("PLAY") != std::string::npos || request[i - 1].find("PENG") != std::string::npos) sstemp = request[i - 1].substr(9, 2);
			if (request[i - 1].find("CHI") != std::string::npos) sstemp = request[i - 1].substr(11, 2);
			sin >> itemp >> stemp;
			if (stemp == "DRAW") { all_card++; }
			else if (stemp == "PLAY") {
				sin >> stemp;  //if (i + 1 >= turnID || request[i + 1].find("DRAW") != string::npos || request[i + 1][0] == '2')
				array_played_card[op(stemp)]++;
			}
			else if (stemp == "CHI") {
				sin >> stemp;//stemp中间的
				int iitemp;//吃的第几张
				//string sstemp = request[i - 1].substr(9, 2);  //吃的别人的
				if (sstemp == stemp) iitemp = 2;
				else if (sstemp < stemp) iitemp = 1;
				else iitemp = 3;
				pack[itemp].push_back(pair2("CHI", pair1(stemp, iitemp)));
				int k = op(stemp);
				if (itemp == myPlayerID) {  //自己牌早已算进played
					array_hand[k - 1]--; array_hand[k]--; array_hand[k + 1]--;
					array_hand[k + iitemp - 2]++;
				}
				else {
					array_played_card[k - 1]++; array_played_card[k]++; array_played_card[k + 1]++;
					array_played_card[k + iitemp - 2]--;
				}
				sin >> stemp;
				if (itemp == myPlayerID) array_hand[op(stemp)]--;
				else  array_played_card[op(stemp)]++;
			}
			else if (stemp == "PENG") {
				stemp = sstemp;
				pack[itemp].push_back(pair2("PENG", pair1(stemp, last_id)));
				if (itemp == myPlayerID)
					array_hand[op(stemp)] -= 2;
				else array_played_card[op(stemp)] += 2;
				sin >> stemp;
				if (itemp == myPlayerID) array_hand[op(stemp)]--;
				else array_played_card[op(stemp)]++;
			}
			else if (stemp == "BUGANG") { //也算杠吧
				sin >> stemp;
				for (vector< pair2 >::iterator i = pack[itemp].begin(); i < pack[itemp].end(); i++) {
					if (i->first == "PENG" && i->second.first == stemp) {
						i->first == "GANG";
						break;
					}
				}
				if (itemp == myPlayerID) array_hand[op(stemp)]--;
				else array_played_card[op(stemp)]++;
			}
			else if (stemp == "GANG" && request[i - 1].length() >= 10) {//否则为暗杠
				stemp = sstemp;
				pack[itemp].push_back(pair2("GANG", pair1(stemp, last_id)));
				if (itemp == myPlayerID) array_hand[op(stemp)] = 0;
				else array_played_card[op(stemp)] += 3;
			}
			else if (stemp == "BUHUA") hua_card[itemp]++;
			last_id = itemp;
		}
	}
	for (int i = 0; i < 42; i++) {
		int j;
		for (j = 0; j < array_hand[i]; j++) {
			int_hand.push_back(i);
		}
	}
#if Test
    for (int i = 0; i < 42; i++) {
		int j;
		for (i <= 34 ? j = 4 : j = 1; j > array_played_card[i]; j--) {
			int_played_card.push_back(i);
		}
    }
	for (auto i : int_hand) {
		cout << op.str[i] << ' ';
	}
#endif
	sin.clear();
}
