#include <iostream>
#include <Windows.h>
#include <string>
#include <exception>
#include "mysql.h"
#include "errmsg.h"
#pragma comment (lib, "libmysql.lib")

int main(){
	MYSQL conn;
	MYSQL* connection = NULL;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	int query_stat;

	// 초기화
	mysql_init(&conn);

	// DB 연결

	connection = mysql_real_connect(&conn, "127.0.0.1", "root", "procademy", "test", 3306, (char*)NULL, 0);
	if (connection == NULL)
	{
		fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
		return 1;
	}



	// Select 쿼리문
	char* query = "SELECT * FROM account";	// From 다음 DB에 존재하는 테이블 명으로 수정하세요
	query_stat = mysql_query(connection, query);
	if (query_stat != 0)
	{
		printf("Mysql query error : %s", mysql_error(&conn));
		return 1;
	}

	// 결과출력
	sql_result = mysql_store_result(connection);		// 결과 전체를 미리 가져옴
//	sql_result=mysql_use_result(connection);		// fetch_row 호출시 1개씩 가져옴

	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
	{
		printf("%2s %2s %s\n", sql_row[0], sql_row[1], sql_row[2]);
	}
	mysql_free_result(sql_result);

	// DB 연결닫기
	mysql_close(connection);




	//	int a = mysql_insert_id(connection);


	//	query_stat = mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_ON);
	//	mysql_next_result(connection);				// 멀티쿼리 사용시 다음 결과 얻기
	//	sql_result=mysql_store_result(connection);	// next_result 후 결과 얻음
}