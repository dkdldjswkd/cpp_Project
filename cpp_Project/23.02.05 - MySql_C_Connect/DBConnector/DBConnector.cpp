#include "DBConnector.h"
#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "mysql/include/mysql.h"
#include "mysql/include/errmsg.h"
#pragma comment(lib, "mysql/lib/libmysql.lib")
#pragma comment(lib, "Winmm.lib")

DBConnector::DBConnector(const char* dbAddr, const char* loginID, const char* password, const char* schema, int port, unsigned short loggingTime){
	// �ʱ�ȭ
	mysql_init(&conn);

	// SSL ���� ��� ����
	unsigned int optssl = SSL_MODE_DISABLED;
	mysql_options(&conn, MYSQL_OPT_SSL_MODE, (const void*)&optssl);

	// DB ����
	connection = mysql_real_connect(&conn, dbAddr, loginID, password, schema, 3306, (char*)NULL, 0);

	// ���� ����
	if (connection == NULL) {
		throw(mysql_error(&conn));
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
	auto ret_val = mysql_query(connection, (char*)query);
	auto queryTime = timeGetTime() - start;

	if (loggingTime < queryTime) {
		// �α�, ���� ��� �ӵ� �α� �����غ���
	}

	if (0 != ret_val) {
		throw(mysql_error(&conn));
	}

	return mysql_store_result(connection);
}