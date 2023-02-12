#include "DBConnector.h"
#include <stdio.h>
#include <Windows.h>
#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

DBConnector::DBConnector(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime) : loggingTime(loggingTime) {
	// �ʱ�ȭ
	mysql_init(&conn);

	// C Connector ����  Mysql 8.0 ����� SSL ���� ��� ������
	unsigned int optssl = SSL_MODE_DISABLED;
	mysql_options(&conn, MYSQL_OPT_SSL_MODE, (const void*)&optssl);

	// DB ����
	connection = mysql_real_connect(&conn, dbAddr, loginID, password, schema, port, (char*)NULL, 0);

	// ���� ����
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
		// ���� ����
		throw DBException(mysql_errno(connection), mysql_error(connection));
	}
	auto queryTime = timeGetTime() - start;

	if (loggingTime < queryTime) {
		// �α�, ���� ��� �ӵ� �α� �����غ���
	}

	return mysql_store_result(connection);
}