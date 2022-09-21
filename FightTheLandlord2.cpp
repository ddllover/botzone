#include <iostream>
#include<fstream>
#include<ctime>
#include <set>
#include<vector>
#include<map>
#include <string>
#include <cassert>
#include <cstring> // 注意memset是cstring里的
#include <algorithm>
#include "jsoncpp/json.h" // 在平台上，C++编译时默认包含此库

using std::pair;
using std::set;
using std::multiset;
using std::map;
using std::sort;
using std::string;
using std::unique;
using std::vector;
using std::fstream;
using std::ios;

constexpr int PLAYER_COUNT = 3;
constexpr int INF = 100000000;

enum class Stage
{
	BIDDING, // 叫分阶段
	PLAYING	 // 打牌阶段
};

enum class Character
{
	LANDLORD,
	FARMER1,
	FARMER2
};

enum class CardComboType
{
	PASS,
	SINGLE,
	SERIAL,
	BOMB,
	ROCKET
};
string CardComboStrings[] = {
	"PASS",
	"SINGLE",
	"SERIAL",
	"BOMB",
	"ROCKET"
};

// 用0~53这54个整数表示唯一的一张牌
using Card = short;
constexpr Card card_joker = 52;
constexpr Card card_JOKER = 53;
using CardID = long long;

// 除了用0~53这54个整数表示唯一的牌，
// 这里还用另一种序号表示牌的大小（不管花色），以便比较，称作等级（Level）
// 对应关系如下：
// 3 4 5 6 7 8 9 10	J Q K   A	2	小王 大王
// 0 1 2 3 4 5 6 7	8 9 10	11	12	13	14
using Level = short;
constexpr Level MAX_LEVEL = 15;
constexpr Level MAX_STRAIGHT_LEVEL = 11;
constexpr Level level_joker = 13;
constexpr Level level_JOKER = 14;

/**
* 将Card变成Level
*/
constexpr Level card2level(Card card)
{
	return card / 4 + card / 53;
}

template <typename CARD_ITERATOR>
CardID cards2id(CARD_ITERATOR begin, CARD_ITERATOR end) {
	vector<Card> cards(begin, end);
	sort(cards.begin(), cards.end());
	CardID id = 0;
	for (Card c : cards) {
		id += ((CardID)1 << c);
	}
	return id;
}

string cardName[MAX_LEVEL] = {
	"3","4","5","6","7","8","9","10","J","Q","K","A","2","joker","JOKER"
};

//手牌
struct Hand {
	CardID id = 0;
	int cardCount = 0;
	int cardKind = 0;
	int cardsTable[5][MAX_LEVEL];
	Hand(CardID _id) {
		id = _id;
		memset(cardsTable, 0, sizeof(cardsTable));
		for (int i = 0; i <= card_JOKER; ++i) {
			if ((id >> i) & 1) {
				cardsTable[0][card2level(i)]++;
				if (cardsTable[0][card2level(i)] == 1)cardKind++;
				cardCount++;
			}
		}
		for (int i = 0; i < MAX_LEVEL; ++i) {
			for (int num = 4; num >= 1; --num) {
				if (cardsTable[0][i] >= num) {
					if (i <= MAX_STRAIGHT_LEVEL && i > 0) {
						cardsTable[num][i] = cardsTable[num][i - 1] + 1;
					}
					else {
						cardsTable[num][i] = 1;
					}
				}
				else {
					cardsTable[num][i] = 0;
				}
			}
		}
	}


	void show() {
		for (int i = 0; i <= card_JOKER; ++i) {
			if (id & ((CardID)1 << i)) {
				std::cout << cardName[card2level(i)] << " ";
			}
		}
		std::cout << "cardKind: " << cardKind;
		//std::cout << std::endl;
	}
};
bool operator<(Hand left, Hand right) {
	return left.id < right.id;
}

//牌型
struct CardCombo {
	CardID id = 0;
	int cardCount = 0;
	int cardPack[MAX_LEVEL];//
	CardComboType comboType;
	Level level = -1;
	int mainNum = -1;
	int length = -1;
	int subNum = -1;
	int subK = -1;

	CardCombo(CardID _id) {
		id = _id;
		if (id == 0) {
			comboType = CardComboType::PASS;
			return;
		}
		memset(cardPack, 0, sizeof(cardPack));
		for (int i = 0; i <= card_JOKER; ++i) {
			if ((id >> i) & 1) {
				cardPack[card2level(i)]++;
				cardCount++;
			}
		}

		int maxnum = 0;
		int countOfNum[5] = { 0 };

		for (int i = 0; i < MAX_LEVEL; ++i) {
			countOfNum[cardPack[i]]++;
			maxnum = maxnum > cardPack[i] ? maxnum : cardPack[i];
		}
		for (int i = MAX_LEVEL - 1; i >= 0; --i) {
			if (cardPack[i] == maxnum) {
				level = i;
				break;
			}
		}
		mainNum = maxnum;
		length = countOfNum[maxnum];
		for (int num = maxnum - 1; num >= 0; --num) {
			if (countOfNum[num] != 0) {
				subNum = num;
				break;
			}
		}
		if (subNum == 0)subK = 0;
		else
			subK = countOfNum[subNum] / length;
		if (length == 1) {
			if (mainNum == 4 && subNum == 0) {
				comboType = CardComboType::BOMB;
			}
			else {
				comboType = CardComboType::SINGLE;
			}
		}
		else {
			if (cardPack[level_JOKER] && cardPack[level_joker]) {
				comboType = CardComboType::ROCKET;
			}
			else {
				comboType = CardComboType::SERIAL;
			}
		}
	}

	CardCombo() : id(0), comboType(CardComboType::PASS) {}

	void show(bool b = 0) {
		if (b == 0) {
			std::cout << "comboType: " << CardComboStrings[(int)comboType] << std::endl;
			std::cout << "length: " << length << std::endl;
			std::cout << "mainNum: " << mainNum << std::endl;
			std::cout << "subNum: " << subNum << std::endl;
			std::cout << "subK: " << subK << std::endl;
			std::cout << "level: " << level << std::endl;
		}
		for (int i = MAX_LEVEL - 1; i >= 0; --i) {
			if (cardPack[i] == mainNum) {
				for (int j = 0; j < mainNum; ++j) {
					std::cout << cardName[i] << " ";
				}
			}
		}
		for (int i = MAX_LEVEL - 1; i >= 0; --i) {
			if (cardPack[i] == subNum) {
				for (int j = 0; j < subNum; ++j) {
					std::cout << cardName[i] << " ";
				}
			}
		}
		//std::cout << std::endl;
	}
};

//辅助函数
  //找从牌
void f(int total, int need, int got, int* chosen, vector<vector<Level> >& vec) {
	for (int i = 0; i < total; ++i) {
		if (got == 0 ? 1 : (i > chosen[got - 1])) {
			chosen[got] = i;
			if (got == need - 1) {
				vector<Level> lst;
				for (int j = 0; j < need; ++j) {
					lst.push_back(chosen[j]);
				}
				vec.push_back(lst);
			}
			else {
				f(total, need, got + 1, chosen, vec);
			}
			chosen[got] = 0;
		}
	}
}
void getSubChoice(int total, int need, vector<vector<Level> >& vec) {
	int chosen[MAX_LEVEL] = { 0 };
	f(total, need, 0, chosen, vec);
}

constexpr int minSerialLen(int mainNum) {
	if (mainNum == 1)return 5;
	else if (mainNum == 2)return 3;
	else return 2;
}
constexpr pair<int, int> rangeOfSubK(int mainNum) {
	if (mainNum <= 2)return std::make_pair(0, 1);
	else if (mainNum == 3)return std::make_pair(0, 2);
	else return std::make_pair(2, 3);
}


/* 状态 */

// 我的牌有哪些
set<Card> myCards;

// 地主明示的牌有哪些
set<Card> landlordPublicCards;

// 大家从最开始到现在都出过什么
vector<vector<Card>> whatTheyPlayed[PLAYER_COUNT];

// 当前要出的牌需要大过谁
CardCombo lastValidCombo;

// 大家还剩多少牌
short cardRemaining[PLAYER_COUNT] = { 17, 17, 17 };

// 我是几号玩家
int myPosition;

// 地主位置
int landlordPosition = -1;

//我的身份
Character myCharacter;

// 地主叫分
int landlordBid = -1;

// 阶段
Stage stage = Stage::BIDDING;

// 自己的第一回合收到的叫分决策
vector<int> bidInput;

//可能的出牌
vector<CardCombo> possibleAction;

//手牌权重记录
map<Hand, int> weightRecord;

//未知的牌
int invisible[MAX_LEVEL] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,1,1 };
Level biggest[5] = { 0 };

//上一位出牌的玩家
int lastPlayer;

//是敌人？
bool isEnemy = false;

//在手牌中凑出特定combo并塞进vec
template <typename CARD_ITERATOR>
void collectCombo(Hand& hand, vector<CardCombo>& vec, CARD_ITERATOR begin, CARD_ITERATOR end) {
	CardID comboID = 0;

	for (Card i = 0; i <= card_JOKER; ++i) {
		if (*begin == card2level(i) && (hand.id & ((CardID)1 << i))) {
			comboID += ((CardID)1 << i);
			++begin;
			if (begin == end)break;
		}
	}
	CardCombo theCombo(comboID);
	if (theCombo.comboType != CardComboType::PASS) {
		vec.push_back(theCombo);
	}
}

//找出特定combo并塞进vec
void findCombo(Hand& hand, vector<CardCombo>& vec, Level _lv, int _length, int _mainNum, int _subNum = 0, int _subK = 0) {
	if (hand.cardsTable[_mainNum][_lv] >= _length) {
		multiset<Level> mainRequired;
		for (Level k = _lv; k >= _lv - _length + 1; --k) {
			for (int i = 0; i < _mainNum; ++i) {
				mainRequired.insert(k);
			}
		}
		if (_subK == 0) {
			collectCombo(hand, vec, mainRequired.begin(), mainRequired.end());
		}
		else {
			int need = _subK * _length;
			vector<Level> enough;
			for (Level k = 0; k < MAX_LEVEL; ++k) {
				if (k<_lv - _length + 1 || k>_lv) {
					if (hand.cardsTable[0][k] >= _subNum) {
						enough.push_back(k);
					}
				}
			}
			int total = enough.size();
			if (total < need)return;
			vector< vector<Level> >subChoice;
			getSubChoice(total, need, subChoice);
			for (int i = 0; i < subChoice.size(); ++i) {
				multiset<Level> required;
				required.insert(mainRequired.begin(), mainRequired.end());
				for (int j = 0; j < need; ++j) {
					for (int k = 0; k < _subNum; ++k)
						required.insert(enough[subChoice[i][j]]);
				}
				collectCombo(hand, vec, required.begin(), required.end());
			}
		}
	}
}

//找出大于特定combo的combo并塞进possibleAction
void findBiggerCombo(Hand& hand, CardComboType comboType, Level level, int length, int mainNum, int subNum = 0, int subK = 0) {
	if (comboType == CardComboType::PASS) {
		//ANY 
		//rocket
		if (hand.cardsTable[0][level_joker] && hand.cardsTable[0][level_JOKER]) {
			multiset<Level> required;
			required.insert(level_joker);
			required.insert(level_JOKER);
			collectCombo(hand, possibleAction, required.begin(), required.end());
		}
		//bomb
		for (Level lv = 0; lv < MAX_LEVEL; ++lv) {
			if (hand.cardsTable[0][lv] == 4) {
				multiset<Level> required;
				for (int i = 0; i < 4; ++i)required.insert(lv);
				collectCombo(hand, possibleAction, required.begin(), required.end());
			}
		}
		//others
		for (Level lv = 0; lv < MAX_LEVEL; ++lv) {
			for (int _mainNum = 1; _mainNum <= 4; ++_mainNum) {
				for (int _length = 1; _length <= 12; _length == 1 ? _length = minSerialLen(_mainNum) : ++_length) {
					for (int _subK = rangeOfSubK(_mainNum).first; _subK < rangeOfSubK(_mainNum).second; ++_subK) {
						if (_subK == 0)findCombo(hand, possibleAction, lv, _length, _mainNum);
						else {
							for (int _subNum = 1; _subNum <= 2; ++_subNum) {
								findCombo(hand, possibleAction, lv, _length, _mainNum, _subNum, _subK);
							}
						}


					}
				}
			}
		}
	}
	else if (comboType == CardComboType::ROCKET) {
		return;
	}
	else {
		//rocket
		if (hand.cardsTable[0][level_joker] && hand.cardsTable[0][level_JOKER]) {
			multiset<Level> required;
			required.insert(level_joker);
			required.insert(level_JOKER);
			collectCombo(hand, possibleAction, required.begin(), required.end());
		}
		if (comboType == CardComboType::BOMB) {
			//bigger bomb
			for (Level lv = level + 1; lv < MAX_LEVEL; ++lv) {
				if (hand.cardsTable[0][lv] == 4) {
					multiset<Level> required;
					for (int i = 0; i < 4; ++i)required.insert(lv);
					collectCombo(hand, possibleAction, required.begin(), required.end());
				}
			}
		}
		else {
			//bomb
			for (Level lv = 0; lv < MAX_LEVEL; ++lv) {
				if (hand.cardsTable[0][lv] == 4) {
					multiset<Level> required;
					for (int i = 0; i < 4; ++i)required.insert(lv);
					collectCombo(hand, possibleAction, required.begin(), required.end());
				}
			}

			//others
			for (Level lv = level + 1; lv < MAX_LEVEL; ++lv) {
				findCombo(hand, possibleAction, lv, length, mainNum, subNum, subK);
			}
		}
	}
}
void findBiggerCombo(Hand& hand, CardCombo cardcombo) {
	findBiggerCombo(hand, cardcombo.comboType, cardcombo.level, cardcombo.length, cardcombo.mainNum, cardcombo.subNum, cardcombo.subK);
}

constexpr int levelWeight(Level level) {
	return (level - 6) * 10 + (level >= 10) ? (level - 10) * 10 : 0;
}

//计算权重
int getWeight(CardCombo combo) {
	if (combo.comboType == CardComboType::ROCKET)return 450;
	if (combo.comboType == CardComboType::BOMB) {
		return 300 + levelWeight(combo.level) / 2;
	}
	if (combo.comboType == CardComboType::SINGLE) {
		if (combo.mainNum == 1 && combo.level > 10) {
			return (combo.mainNum - 1) * 50 + levelWeight(combo.level) + 30;
		}
		else {
			return (combo.mainNum - 1) * 50 + levelWeight(combo.level);
		}
	}
	return combo.mainNum * 50 + levelWeight(combo.level);
}

int getWeight(Hand hand, bool first = 0) {
	if (hand.id == 0) {
		if (first)return INF;
		else return 0;
	}
	auto iter = weightRecord.find(hand);
	if (iter != weightRecord.end()) {
		return iter->second;
	}
	else {
		vector<CardCombo> node;
		//rocket
		if (hand.cardsTable[0][level_joker] && hand.cardsTable[0][level_JOKER]) {
			multiset<Level> required;
			required.insert(level_joker);
			required.insert(level_JOKER);
			collectCombo(hand, node, required.begin(), required.end());
		}
		//bomb
		for (Level lv = 0; lv < MAX_LEVEL; ++lv) {
			if (hand.cardsTable[0][lv] == 4) {
				multiset<Level> required;
				for (int i = 0; i < 4; ++i)required.insert(lv);
				collectCombo(hand, node, required.begin(), required.end());
			}
		}
		//others
		Level lv;
		for (lv = MAX_LEVEL - 1; lv >= 0; --lv) {
			if (hand.cardsTable[0][lv] > 0)break;
		}
		for (int _mainNum = 1; _mainNum <= 4; ++_mainNum) {
			for (int _length = 1; _length <= 12; _length == 1 ? _length = minSerialLen(_mainNum) : ++_length) {
				for (int _subK = rangeOfSubK(_mainNum).first; _subK < rangeOfSubK(_mainNum).second; ++_subK) {
					if (_subK == 0)findCombo(hand, node, lv, _length, _mainNum);
					else {
						for (int _subNum = 1; _subNum <= 2; ++_subNum) {
							findCombo(hand, node, lv, _length, _mainNum, _subNum, _subK);
						}
					}
				}
			}
		}

		int max = -INF;
		for (auto i = node.begin(); i != node.end(); ++i) {
			int cur = getWeight(*i) + getWeight(Hand(hand.id - i->id));
			if (cur > max)max = cur;
		}
		int ret = max - 100;
		weightRecord[hand] = ret;
		return ret;
	}
}


//主动
CardCombo myAction_1(Hand hand) {
	findBiggerCombo(hand, lastValidCombo);
	vector<CardCombo> goodCombo;
	int max = -INF;
	int dan_punishment = (myPosition == landlordPosition && (cardRemaining[(myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT] == 1 || cardRemaining[(myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT] == 1)) || (myPosition != landlordPosition && cardRemaining[landlordPosition] == 1) ? 1000 : 0;
	if (myPosition != landlordPosition && cardRemaining[(myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT] == 1 && (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT != landlordPosition)
	{
		for (auto i = possibleAction.begin(); i != possibleAction.end(); ++i) {
			if (i->mainNum == 1 && i->length == 1) {
				return *i;
			}
		}
	}
	else
	{
		for (auto i = possibleAction.begin(); i != possibleAction.end(); ++i) {
			Hand left(hand.id - i->id);
			if (left.id == 0)return *i;//能一次出完，直接出
			int weight = getWeight(*i) + getWeight(left, 1);
			if (weight > max) {
				goodCombo.clear();
				max = weight;
				if (i->comboType != CardComboType::ROCKET && i->mainNum != 4) {
					goodCombo.push_back(*i);
				}
			}
			else if (weight == max) {
				if (i->comboType != CardComboType::ROCKET && i->mainNum != 4) {
					goodCombo.push_back(*i);
				}
			}
		}
		auto best = goodCombo.begin();
		int maxscore = -INF;
		for (auto i = goodCombo.begin(); i != goodCombo.end(); ++i) {
			Hand left(hand.id - i->id);
			int score = i->mainNum * i->length * 100 + getWeight(left, 1) + (i->level >= 10 ? -1000*(i->mainNum-1) : 0);
			if (i->mainNum == 1) score -= dan_punishment;
			if (left.cardKind == 1 && i->level >= biggest[i->mainNum])score += 10000;
				if (score > maxscore) {
					maxscore = score;
					best = i;
				}
		}
		return *best;
	}
}

//被动
CardCombo myAction_2(Hand hand) {
	findBiggerCombo(hand, lastValidCombo);
	if (possibleAction.size() == 0)return CardCombo();
	int lvcWeight = getWeight(lastValidCombo);
	int punishment = isEnemy * (lvcWeight > 0 ? (cardRemaining[lastPlayer] <= 8 ? 3 : 1) * lvcWeight : 0) + 100;
	int reward = (!isEnemy) * ((lastValidCombo.level >= 7 ? 1000 : 0) + ((cardRemaining[lastPlayer] <= 3 && lastValidCombo.level >= 4) ? 1000 : 0));
	int enemyR = (myPosition == landlordPosition) ? std::min(cardRemaining[(myPosition + 1) % 3],cardRemaining[(myPosition + 2) % 3]) : (cardRemaining[landlordPosition]);
	if ((isEnemy && ((lastValidCombo.level >= 10 && lastValidCombo.mainNum*lastValidCombo.length>2)||enemyR<=6))) {

	}
	else {
		possibleAction.push_back(CardCombo());
	}
	int max = -INF;
	auto best = possibleAction.begin();
	for (auto i = possibleAction.begin(); i != possibleAction.end(); ++i) {
		Hand left(hand.id - i->id);
		int weight = getWeight(left, 1);
		if (left.cardKind == 1 && i->level >= biggest[i->mainNum])weight += 10000;
		if (isEnemy && cardRemaining[lastPlayer] <= 2)weight += getWeight(*i) * 100;
		if (i->comboType == CardComboType::PASS)weight += (reward - punishment);
		if (weight > max) {
			max = weight;
			best = i;
		}
	}
	return *best;
}



namespace BotzoneIO
{
	using namespace std;
	void read()
	{
		// 读入输入（平台上的输入是单行）
		string line;
		getline(cin, line);
		Json::Value input;
		Json::Reader reader;
		reader.parse(line, input);

		// 首先处理第一回合，得知自己是谁、有哪些牌
		{
			auto firstRequest = input["requests"][0u]; // 下标需要是 unsigned，可以通过在数字后面加u来做到
			auto own = firstRequest["own"];
			for (unsigned i = 0; i < own.size(); i++)
				myCards.insert(own[i].asInt());
			if (!firstRequest["bid"].isNull())
			{
				// 如果还可以叫分，则记录叫分
				auto bidHistory = firstRequest["bid"];
				myPosition = bidHistory.size();
				for (unsigned i = 0; i < bidHistory.size(); i++)
					bidInput.push_back(bidHistory[i].asInt());
			}
		}

		// history里第一项（上上家）和第二项（上家）分别是谁的决策


		int turn = input["requests"].size();
		for (int i = 0; i < turn; i++)
		{
			auto request = input["requests"][i];
			auto llpublic = request["publiccard"];
			if (!llpublic.isNull())
			{
				// 第一次得知公共牌、地主叫分和地主是谁
				landlordPosition = request["landlord"].asInt();
				landlordBid = request["finalbid"].asInt();
				myPosition = request["pos"].asInt();
				cardRemaining[landlordPosition] += llpublic.size();
				for (unsigned i = 0; i < llpublic.size(); i++)
				{
					landlordPublicCards.insert(llpublic[i].asInt());
					if (landlordPosition == myPosition)
						myCards.insert(llpublic[i].asInt());
				}
			}
			int whoInHistory[] = { (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT, (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT };
			auto history = request["history"]; // 每个历史中有上家和上上家出的牌
			if (history.isNull())
				continue;
			stage = Stage::PLAYING;

			// 逐次恢复局面到当前
			int howManyPass = 0;
			for (int p = 0; p < 2; p++)
			{
				int player = whoInHistory[p];	// 是谁出的牌
				auto playerAction = history[p]; // 出的哪些牌
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) // 循环枚举这个人出的所有牌
				{
					int card = playerAction[_].asInt(); // 这里是出的一张牌
					playedCards.push_back(card);
				}
				whatTheyPlayed[player].push_back(playedCards); // 记录这段历史
				cardRemaining[player] -= playerAction.size();

				if (playerAction.size() == 0)
					howManyPass++;
				else {
					lastValidCombo = CardCombo(cards2id(playedCards.begin(), playedCards.end()));
					lastPlayer = player;
				}
			}

			if (howManyPass == 2) {
				lastValidCombo = CardCombo();
				lastPlayer = myPosition;
			}

			if (i < turn - 1)
			{
				// 还要恢复自己曾经出过的牌
				auto playerAction = input["responses"][i]; // 出的哪些牌
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) // 循环枚举自己出的所有牌
				{
					int card = playerAction[_].asInt(); // 这里是自己出的一张牌
					myCards.erase(card);				// 从自己手牌中删掉
					playedCards.push_back(card);
				}
				whatTheyPlayed[myPosition].push_back(playedCards); // 记录这段历史
				cardRemaining[myPosition] -= playerAction.size();
			}
		}
		//确定身份和敌我关系

		if (myPosition == landlordPosition)
			myCharacter = Character::LANDLORD;
		else if ((myPosition + 1) % PLAYER_COUNT == landlordPosition)
			myCharacter = Character::FARMER2;
		else
			myCharacter = Character::FARMER1;

		if (myCharacter == Character::LANDLORD && lastPlayer != myPosition) {
			isEnemy = true;
		}
		else if (myCharacter != Character::LANDLORD && lastPlayer == landlordPosition) {
			isEnemy = true;
		}
		else {
			isEnemy = false;
		}

		//统计未知牌
		for (int p = 0; p <= 2; ++p) {
			for (auto i = whatTheyPlayed[p].begin(); i != whatTheyPlayed[p].end(); ++i) {
				for (auto j = i->begin(); j != i->end(); ++j) {
					invisible[card2level(*j)]--;
				}
			}
		}
		for (auto i = myCards.begin(); i != myCards.end(); ++i) {
			invisible[card2level(*i)]--;
		}
		for (int num = 1; num <= 4; ++num) {
			for (Level lv = level_JOKER; lv >= 0; --lv) {
				if (invisible[lv] >= num) {
					biggest[num] = lv;
					break;
				}
			}
		}
	}


	//输出叫分（0, 1, 2, 3 四种之一）


	void bid(int value)
	{
		Json::Value result;
		result["response"] = value;

		Json::FastWriter writer;
		cout << writer.write(result) << endl;
	}




	//输出打牌决策，begin是迭代器起点，end是迭代器终点
	//CARD_ITERATOR是Card（即short）类型的迭代器

	template <typename CARD_ITERATOR>
	void play(CARD_ITERATOR begin, CARD_ITERATOR end)
	{
		Json::Value result, response(Json::arrayValue);
		for (; begin != end; begin++)
			response.append(*begin);
		result["response"] = response;

		Json::FastWriter writer;
		cout << writer.write(result) << endl;
	}

	void play(CardID id) {
		Json::Value result, response(Json::arrayValue), debug(Json::arrayValue);
		for (int i = 0; i <= card_JOKER; ++i) {
			if ((id >> i) & 1) {
				response.append(i);
			}
		}
		result["response"] = response;
		debug.append(isEnemy);
		debug.append(myPosition);
		debug.append(landlordPosition);
		debug.append(lastPlayer);
		result["debug"] = debug;
		Json::FastWriter writer;
		cout << writer.write(result) << endl;

	}
}

int main()
{
	srand(time(nullptr));

	
	BotzoneIO::read();

	if (stage == Stage::BIDDING)
	{
		// 做出决策（你只需修改以下部分）

		auto maxBidIt = std::max_element(bidInput.begin(), bidInput.end());
		int maxBid = maxBidIt == bidInput.end() ? 0 : *maxBidIt;

		int bidValue;
		Hand myhand(cards2id(myCards.begin(), myCards.end()));
		int score = getWeight(myhand);
		for (Level lv = 11; lv <= level_JOKER; ++lv) {
			score += myhand.cardsTable[0][lv] * (lv - 10) * 25;
		}
		if (score >= -200) {
			if(score>=0)bidValue = 3;
			else bidValue = maxBid + 1;
		}
		else bidValue = 0;
		// 决策结束，输出结果（你只需修改以上部分）

		BotzoneIO::bid(bidValue);
	}
	else if (stage == Stage::PLAYING)
	{
		// 做出决策（你只需修改以下部分）
		Hand myhand(cards2id(myCards.begin(), myCards.end()));
		CardCombo action;
		if (lastValidCombo.id == 0)
			action = myAction_1(myhand);
		else
			action=myAction_2(myhand);


		// 决策结束，输出结果（你只需修改以上部分）

		BotzoneIO::play(action.id);
	}
	
}
