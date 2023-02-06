#include <iostream>
#include <Windows.h>
#include <thread>
#include "DB_Job.h"
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
MYSQL_RES* sql_result;
MYSQL_ROW sql_row;
int query_stat;

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

void f(int AccountNo) {

	char buf[1024];
	const char* query = "SELECT * FROM level";
	query_stat = mysql_query(connection, query);
	if (query_stat != 0) {
		printf("Mysql query error : %s", mysql_error(&conn));
		return;
	}

	sql_result = mysql_store_result(connection); // 결과 전체를 미리 가져옴
	while ((sql_row = mysql_fetch_row(sql_result)) != NULL) {
		if (AccountNo == std::stoi(sql_row[0])) {
			sprintf(buf, "update level set level = level+1 where account_no = %d", AccountNo);
			mysql_query(connection, buf);
			return;
		}
	}
	sprintf(buf, "INSERT INTO level (account_no, level) VALUES (%d, 1);", AccountNo);
	mysql_query(connection, buf);
}

int main() {
	jobEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	Init();
	f((rand() % 100) + 2);
}

int func() {
	// Select 쿼리문
	const char* query = "SELECT * FROM actor";	// From 다음 DB에 존재하는 테이블 명으로 수정하세요
	query_stat = mysql_query(connection, query);
	if (query_stat != 0) {
		printf("Mysql query error : %s", mysql_error(&conn));
		return 1;
	}

	// 결과출력
	sql_result = mysql_store_result(connection); // 결과 전체를 미리 가져옴
	// sql_result=mysql_use_result(connection); // fetch_row 호출시 1개씩 가져옴

	while ((sql_row = mysql_fetch_row(sql_result)) != NULL) {
		printf("%2s %2s %s\n", sql_row[0], sql_row[1], sql_row[2]);
	}
	mysql_free_result(sql_result);

	// DB 연결닫기
	mysql_close(connection);

	// int a = mysql_insert_id(connection);
	// query_stat = mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_ON);
	// mysql_next_result(connection); // 멀티쿼리 사용시 다음 결과 얻기
	// sql_result=mysql_store_result(connection); // next_result 후 결과 얻음
}