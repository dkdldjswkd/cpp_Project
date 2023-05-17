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
	snprintf(fileName, LOG_FILE_LEN, "DB_LOG_%s_%u.txt", __DATE__, GetCurrentThreadId()); // ��Ƽ ������ȯ��(DBConnectorTLS)���� ���� file�� �ٶ��� �ʰ� �ϱ�����
	fopen_s(&fp, fileName, "at");

	if (fp != NULL) {
		fprintf(fp, "[Query : %s] [QueryTime : %u ms]\n", query, queryTime);
		fclose(fp);
	}
}

void DBConnector::ConnectDB(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime){
	if (connection != nullptr)
		return;

	// �ʱ�ȭ
	mysql_init(&conn);

	// C Connector Mysql 8.0 ���� �� SSL ���� �����ʿ�
	int optssl = SSL_MODE_DISABLED;
	mysql_options(&conn, MYSQL_OPT_SSL_MODE, (const void*)&optssl);

	// �翬�� �ɼ�
	bool reconnect = true;
	mysql_options(&conn, MYSQL_OPT_RECONNECT, (const void*)&reconnect);

	// ����
	connection = mysql_real_connect(&conn, dbAddr, loginID, password, schema, port, (char*)NULL, 0);

	// ���� ����
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