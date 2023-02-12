#include "DBConnector.h"
#include <stdio.h>
#include <Windows.h>
#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

DBConnector::DBConnector(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime) : loggingTime(loggingTime) {
	// 초기화
	mysql_init(&conn);

	// C Connector 에서  Mysql 8.0 연결시 SSL 연결 모드 꺼야함
	unsigned int optssl = SSL_MODE_DISABLED;
	mysql_options(&conn, MYSQL_OPT_SSL_MODE, (const void*)&optssl);

	// DB 연결
	connection = mysql_real_connect(&conn, dbAddr, loginID, password, schema, port, (char*)NULL, 0);

	// 연결 실패
	if (connection == NULL) {
		throw DBException(mysql_errno(connection), mysql_error(connection));
	}
}

DBConnector::~DBConnector() {
	mysql_close(connection);
}

MYSQL_RES* DBConnector::Query(const char* queryFormat, ...) {
	char query[MAX_QUERY];

	va_list var_list;
	va_start(var_list, queryFormat);
	vsnprintf((char*)query, MAX_QUERY - 1, queryFormat, var_list);
	va_end(var_list);
	query[MAX_QUERY - 1] = 0;

	auto start = timeGetTime();
	if (0 != mysql_query(connection, (char*)query)) {
		// 쿼리 실패
		throw DBException(mysql_errno(connection), mysql_error(connection));
	}
	auto queryTime = timeGetTime() - start;

	if (loggingTime < queryTime) {
		// 로깅, 쿼리 평균 속도 로깅 생각해보자
	}

	return mysql_store_result(connection);
}