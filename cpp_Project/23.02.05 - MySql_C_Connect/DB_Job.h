#pragma once
#include <string>
typedef unsigned short WORD;

// Player
#define MAX_PLAYER 100
#define MAX_SLOT  100
#define MAX_ITEM  100
#define MAX_QUEST 100
#define MAX_QUANTITY 100
#define MAX_LEVEL 10000
#define MAX_MONEY 10000000
#define INVALID_RETURN -1
struct Player {
	int level = 0;
	int money = 0;

	int complete_quest = 0;
	bool quest[MAX_QUEST] = { 0, };

	static struct Item {
		Item() : id(0), price(0), quantity(0) {}
		int id;
		int price;
		int quantity;
	};
	Item item[MAX_SLOT];
	int use_slot = 0;

public:
	// ���� ���� ret
	int levelUp() {
		int up = (rand() % 3) + 1;

		if (MAX_LEVEL <= level)
			return INVALID_RETURN;

		level += up;
		if (MAX_LEVEL <= level) {
			return up - (level - MAX_LEVEL);
		}
		return up;
	}

	// ���� �� ret
	int AddMoney() {
		int addMoney = (rand() % 10000) + 1;

		if (MAX_MONEY <= money)
			return INVALID_RETURN;

		money += addMoney;
		if (MAX_MONEY <= money) {
			return addMoney - (money - MAX_MONEY);
		}
		return addMoney;
	}

	// Ŭ���� �� ����Ʈ ��ȣ ret
	int questComplete() {
		if (MAX_QUEST <= complete_quest) {
			return INVALID_RETURN;
		}
		else {
			for (;;) {
				auto clear = rand() % MAX_QUEST;
				if (quest[clear])
					continue;

				quest[clear] = true;
				complete_quest++;
				return clear;
			}
		}
	}

	int BuyItem(int& itemID, int& slot) {
		// id == price �� ����
		auto id = rand() % MAX_ITEM;
		itemID = id;

		// �� �ִ��� Ȯ��
		if (money < id)
			return INVALID_RETURN;

		// �̹� �����ִ� ������
		for (int i = 0; i < use_slot; i++) {
			if (item[i].id == id && (item[i].quantity < MAX_QUANTITY)) {
				slot = i;
				money -= id;
				item[i].quantity++;
				return true;
			}
		}
		// �������� �ʴ� ������
		if (MAX_SLOT <= use_slot)
			return INVALID_RETURN;
		for (int i = 0; i < MAX_SLOT; i++) {
			if (item[i].quantity == 0) {
				item[i].id = id;
				item[i].quantity++;
				return true;
			}
		}
		return INVALID_RETURN;
	}
};
Player players[MAX_PLAYER];

//------------------------------
// DB ���� �޽����� ���
//------------------------------
struct st_DBQUERY_HEADER {
	virtual void Exec() = 0;
	WORD Type;
};

//------------------------------
//  �÷��̾� ������
//------------------------------
#define df_DBQUERY_MSG_LEVELUP 0
struct st_DBQUERY_MSG_LEVELUP : public st_DBQUERY_HEADER {
public:
	st_DBQUERY_MSG_LEVELUP() : AccountNo(AccountNo), Level(Level) {}

public:
	__int64	AccountNo;
	int	Level;

public:
	void Exec() { 
		//char buf[1024];
		//const char* query = "SELECT * FROM level";
		//query_stat = mysql_query(connection, query);
		//if (query_stat != 0) {
		//	printf("Mysql query error : %s", mysql_error(&conn));
		//	return;
		//}

		//sql_result = mysql_store_result(connection); // ��� ��ü�� �̸� ������
		//while ((sql_row = mysql_fetch_row(sql_result)) != NULL) {
		//	if (AccountNo == std::stoi(sql_row[0])) {
		//		mysql_free_result(sql_result);

		//		sprintf(buf, "update level set level = level+1 where account_no = %d", AccountNo);
		//		mysql_query(connection, buf);
		//		mysql_free_result(sql_result);
		//		return;
		//	}
		//}
		//sprintf(buf, "INSERT INTO level (account_no, level) VALUES (%d, 1);", AccountNo);
		//mysql_free_result(sql_result);
	}
};

//------------------------------
// �÷��̾� �� ����
//------------------------------
#define df_DBQUERY_MSG_MONEY_ADD 1
struct st_DBQUERY_MSG_MONEY_ADD : public st_DBQUERY_HEADER {
public:
	st_DBQUERY_MSG_MONEY_ADD() : iAccountNo(iAccountNo), iMoney(iMoney), iWhy(iWhy) {}

public:
	__int64	iAccountNo;
	int	iMoney;
	int	iWhy;		// ����

public:
	void Exec() {}
};

//------------------------------
// ����Ʈ �Ϸ�
//------------------------------
#define df_DBQUERY_MSG_QUEST_COMPLETE 2
struct st_DBQUERY_MSG_QUEST_COMPLETE : public st_DBQUERY_HEADER {
public:
	st_DBQUERY_MSG_QUEST_COMPLETE() : iAccountNo(iAccountNo), iQuestID(iQuestID){}

public:
	__int64	iAccountNo;
	int	iQuestID;

public:
	void Exec() {}
};

//------------------------------
// ������ ����
//------------------------------
#define df_DBQUERY_MSG_ITEM_BUY 3
struct st_DBQUERY_MSG_ITEM_BUY : public st_DBQUERY_HEADER {
public:
	st_DBQUERY_MSG_ITEM_BUY() : iAccountNo(iAccountNo), iItemID(iItemID) {}
	
public:
	__int64	iAccountNo;
	int	iItemID;
	int	iSlot;

public:
	void Exec() {}
};

//------------------------------
// �ŷ�
//------------------------------
#define df_DBQUERY_MSG_ITEM_TRADE 4
struct st_DBQUERY_MSG_ITEM_TRADE : public st_DBQUERY_HEADER {
public:
	st_DBQUERY_MSG_ITEM_TRADE() {}

public:
	void Exec() {}

public:
	__int64	FromAccountNo;
	__int64	FromItemSlot;

	__int64	ToAccountNo;
	__int64	ToItemSlot;

	int	tradeMoney;
	int	Quantity;
};