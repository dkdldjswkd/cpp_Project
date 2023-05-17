#include "DBConnector.h"
#include <stdio.h>
#include <Windows.h>
#include <timeapi.h>
#include <strsafe.h>
#include <thread>
#pragma comment(lib, "Winmm.lib")

DBConnector::DBConnector(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime) : loggingTime(loggingTime) {
	ConnectDB(dbAddr, port, loginID, password, schema, loggingTime);
}

DBConnector::~DBConnector() {
	mysql_close(connection);
}

void DBConnector::Log(const char* query, unsigned long queryTime){
	// fopen
	FILE* fp;
	char fileName[LOG_FILE_LEN];
	snprintf(fileName, LOG_FILE_LEN, "DB_LOG_%s_%u.txt", __DATE__, GetCurrentThreadId()); // 멀티 스레드환경(DBConnectorTLS)에서 같은 file을 바라보지 않게 하기위함
	fopen_s(&fp, fileName, "at");

	if (fp != NULL) {
		fprintf(fp, "[Query : %s] [QueryTime : %u ms]\n", query, queryTime);
		fclose(fp);
	}
}

void DBConnector::ConnectDB(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime){
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
	StringCchVPrintfA(query, MAX_QUERY, queryFormat, var_list);
	va_end(var_list);

	auto start = timeGetTime();
	if (0 != mysql_query(connection, (char*)query)) {
		throw DBException(query, mysql_errno(connection), mysql_error(connection));
	}
	auto durTime = timeGetTime() - start;
	if (loggingTime < durTime) {
		Log(query, durTime);
	}

	return mysql_store_result(connection);
}

MYSQL_RES* DBConnector::Query(const char* queryFormat, va_list args) {
	char query[MAX_QUERY];
	StringCchVPrintfA(query, MAX_QUERY, queryFormat, args);

	auto start = timeGetTime();
	if (0 != mysql_query(connection, (char*)query)) {
		throw DBException(query, mysql_errno(connection), mysql_error(connection));
	}
	auto durTime = timeGetTime() - start;
	if (loggingTime < durTime) {
		Log(queryFormat, durTime);
	}

	return mysql_store_result(connection);
}