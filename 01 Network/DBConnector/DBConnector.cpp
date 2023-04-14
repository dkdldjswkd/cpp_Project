#include "DBConnector.h"
#include <stdio.h>
#include <Windows.h>
#include <timeapi.h>
#pragma comment(lib, "Winmm.lib")

// �߰� �����ؾ��� ���
// loggingTime �ʰ� ���� �α�
// ...
// 10�п� �ѹ��� avr, min, max, min query, max query �α�
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