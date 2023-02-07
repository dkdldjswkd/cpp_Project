#include <iostream>
#include <Windows.h>
#include <thread>
#include "DB_Job.h"
#include "LFObjectPool.h"
#include "LFQueue.h"
#include "mysql/include/mysql.h"
#include "mysql/include/errmsg.h"
#pragma comment(lib, "mysql/lib/libmysql.lib")
#pragma warning(disable : 4996)
using namespace std;

// JOB
HANDLE jobEvent = INVALID_HANDLE_VALUE;
LFQueue <st_DBQUERY_HEADER*> jobQ;

// SQL
MYSQL conn;
MYSQL* connection = NULL;
MYSQL_ROW sql_row;

int func();

// DB 메시지 생성
void update() {
	while (1) {
		Sleep(100);

		auto type = rand() % 3; // 일단 세개만
		switch (type) {
		case df_DBQUERY_MSG_LEVELUP: {
			st_DBQUERY_MSG_LEVELUP* p_job = new st_DBQUERY_MSG_LEVELUP();
			p_job->Type = df_DBQUERY_MSG_LEVELUP;
			p_job->AccountNo = rand() % 100;
			p_job->Level = (rand() % 3) + 1;
			jobQ.Enqueue(p_job);
			break;
		}
		case df_DBQUERY_MSG_MONEY_ADD: {
			st_DBQUERY_MSG_MONEY_ADD* p_job = new st_DBQUERY_MSG_MONEY_ADD();
			p_job->Type = df_DBQUERY_MSG_MONEY_ADD;
			p_job->iAccountNo = rand() % 100;
			p_job->iMoney = (rand() % 10000) + 1;
			p_job->iWhy = rand() % 10;
			jobQ.Enqueue(p_job);
			break;
		}
		case df_DBQUERY_MSG_QUEST_COMPLETE: {
			st_DBQUERY_MSG_QUEST_COMPLETE* p_job = new st_DBQUERY_MSG_QUEST_COMPLETE();
			p_job->Type = df_DBQUERY_MSG_QUEST_COMPLETE;
			p_job->iAccountNo = rand() % 100;
			p_job->iQuestID = rand() % 100;
			jobQ.Enqueue(p_job);
			break;
		}
		//case df_DBQUERY_MSG_ITEM_BUY: {
		//	st_DBQUERY_MSG_ITEM_BUY* p_job = new st_DBQUERY_MSG_ITEM_BUY();
		//	p_job->Type = df_DBQUERY_MSG_ITEM_BUY;
		//	p_job->iAccountNo = rand() % 100;
		//	p_job->iItemID = rand() % 100;
		//	p_job->iPrice = (rand() % 10000) + 1;
		//	p_job->iSlot = rand() % 100;
		//	break;
		//}
		//case df_DBQUERY_MSG_ITEM_TRADE: {
		//	st_DBQUERY_MSG_ITEM_TRADE* p_job = new st_DBQUERY_MSG_ITEM_TRADE();
		//	p_job->Type = df_DBQUERY_MSG_ITEM_TRADE;
		//	p_job->FromAccountNo = rand() % 100;
		//	p_job->FromItemSlot = rand() % 100;
		//	p_job->ToAccountNo = (rand() % 100) % p_job->FromAccountNo;
		//	p_job->ToItemSlot = rand() % 100;
		//	p_job->tradeMoney = (rand() % 10000) + 1;
		//	p_job->Quantity = (rand() % 100) + 1;
		//	break;
		//}
		}
	}
	
}

// 큐잉, DB 저장
void DBWrite() {
	for (;;) {
		WaitForSingleObject(jobEvent, INFINITE);
		st_DBQUERY_HEADER* p_job;
		while (jobQ.Dequeue(&p_job)) {
			p_job->Exec();
		}
	}
}

void Init() {
	// 초기화
	mysql_init(&conn);

	// SSL 연결 모드 끄기
	unsigned int optssl = SSL_MODE_DISABLED;
	mysql_options(&conn, MYSQL_OPT_SSL_MODE, (const void*)&optssl);

	// DB 연결
	connection = mysql_real_connect(&conn, "127.0.0.1", "root", "password", "test", 3306, (char*)NULL, 0);
	if (connection == NULL) {
		fprintf(stderr, "Mysql connection error : %s \n", mysql_error(&conn));
		return;
	}
}

#define MAX_QUERY 1024
struct Query {
	char query[MAX_QUERY];
};
J_LIB::LFObjectPool<Query> queryPool;

MYSQL_RES* SendQuery(const char* queryFormat, ...) {
	Query * p_query = queryPool.Alloc();

	va_list var_list;
	va_start(var_list, queryFormat);
	vsnprintf((char*)p_query, MAX_QUERY - 1, queryFormat, var_list);
	va_end(var_list);

	int query_stat = mysql_query(connection, (char*)p_query);
	if (query_stat != 0) {
		printf("Mysql query error : %s", mysql_error(&conn));
		CRASH();
	}
	//return mysql_store_result(connection);

	// 결과출력
	return mysql_store_result(connection); // 결과 전체를 미리 가져옴
	// sql_result=mysql_use_result(connection); // fetch_row 호출시 1개씩 가져옴
}

int main() {
	jobEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	Init();

	//int account_no = 42;
	//MYSQL_RES* sql_result = SendQuery("update level set level = level+1 where account_no = %d", account_no);

	//MYSQL_ROW sql_row = mysql_fetch_row(sql_result);
	//if (sql_row != NULL) {
	//	// 등록되지 않음
	//	mysql_free_result(sql_result);
	//	SendQuery("INSERT INTO level (account_no, level) VALUES (%d, 1);", account_no);
	//}
	//else {
	//	// 이미 등록됨
	//	mysql_free_result(sql_result);
	//	SendQuery("update level set level = level+1 where account_no = %d", account_no);
	//}

	MYSQL_RES* sql_result = SendQuery("SELECT * FROM level");
	while ((sql_row = mysql_fetch_row(sql_result)) != NULL) {
		printf("%2s %2s %s\n", sql_row[0], sql_row[1], sql_row[2]);
	}
	mysql_free_result(sql_result);

	//// 결과출력
	//sql_result = mysql_store_result(connection); // 결과 전체를 미리 가져옴
	//// sql_result=mysql_use_result(connection); // fetch_row 호출시 1개씩 가져옴

	//while ((sql_row = mysql_fetch_row(sql_result)) != NULL) {
	//	printf("%2s %2s %s\n", sql_row[0], sql_row[1], sql_row[2]);
	//}
	//mysql_free_result(sql_result);

	//	// Select 쿼리문
	//const char* query = "SELECT * FROM level";	// From 다음 DB에 존재하는 테이블 명으로 수정하세요
	//int query_stat = mysql_query(connection, query);
	//if (query_stat != 0) {
	//	printf("Mysql query error : %s", mysql_error(&conn));
	//	return 1;
	//}

	//// 결과출력
	//MYSQL_RES* sql_result = mysql_store_result(connection); // 결과 전체를 미리 가져옴
	//// sql_result=mysql_use_result(connection); // fetch_row 호출시 1개씩 가져옴

	//while ((sql_row = mysql_fetch_row(sql_result)) != NULL) {
	//	printf("%2s %2s %s\n", sql_row[0], sql_row[1], sql_row[2]);
	//}
	//mysql_free_result(sql_result);

	// DB 연결닫기
	mysql_close(connection);
}