#include "DBConnector.h"
#include <stdio.h>
#include <Windows.h>
#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

// 추가 구현해야할 기능
// loggingTime 초과 쿼리 로깅
// ...
// 10분에 한번씩 avr, min, max, min query, max query 로깅
// ...

DBConnector::DBConnector(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime) : loggingTime(loggingTime) {
	ConnectDB(dbAddr, port, loginID, password, schema, loggingTime);
}

DBConnector::~DBConnector() {
	mysql_close(connection);
}

void DBConnector::ConnectDB(const char* dbAddr, int port, const char* loginID, 
	const char* password, const char* schema, unsigned short loggingTime){
	if (connection != nullptr)
		return;

	// 초기화
	mysql_init(&conn);

	// C Connector Mysql 8.0 연결 시 SSL 연결 해제필요
	int optssl = SSL_MODE_DISABLED;
	mysql_options(&conn, MYSQL_OPT_SSL_MODE, (const void*)&optssl);

	// 재연결 옵션
	bool reconnect = true;
	mysql_options(&conn, MYSQL_OPT_RECONNECT, (const void*)&reconnect);

	// 연결
	connection = mysql_real_connect(&conn, dbAddr, loginID, password, schema, port, (char*)NULL, 0);

	// 연결 실패
	if (connection == NULL) {
		throw DBException(mysql_errno(connection), mysql_error(connection));
	}
}

MYSQL_RES* DBConnector::Query(const char* queryFormat, ...) {
	char query[MAX_QUERY];

	va_list var_list;
	va_start(var_list, queryFormat);
	vsnprintf((char*)query, MAX_QUERY - 1, queryFormat, var_list);
	query[MAX_QUERY - 1] = 0;
	va_end(var_list);

	if (0 != mysql_query(connection, (char*)query)) {
		throw DBException(query, mysql_errno(connection), mysql_error(connection));
	}

	return mysql_store_result(connection);
}

MYSQL_RES* DBConnector::Query(const char* queryFormat, va_list args) {
	char query[MAX_QUERY];
	vsnprintf((char*)query, MAX_QUERY - 1, queryFormat, args);
	query[MAX_QUERY - 1] = 0;

	if (0 != mysql_query(connection, (char*)query)) {
		throw DBException(query, mysql_errno(connection), mysql_error(connection));
	}

	return mysql_store_result(connection);
}