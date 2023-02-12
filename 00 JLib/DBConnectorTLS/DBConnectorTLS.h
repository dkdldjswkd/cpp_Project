// ���� 8:15 2023-02-11
#pragma once
#include "DBConnector.h"

class DBConnectorTLS {
public:
	DBConnectorTLS(const char* dbAddr, const char* loginID, const char* password, const char* schema, const int port, unsigned short loggingTime = INFINITE);
	~DBConnectorTLS();

private:
	const DWORD tlsIndex;
	const char* dbAddr;
	const char* loginID;
	const char* password;
	const char* schema;
	const int port;
	const unsigned short loggingTime; // ���� �� �ش� �ð� �ʰ� �� �α�

public:
	MYSQL_RES* Query(const char* queryFormat, ...) {
		DBConnector* p_connector = (DBConnector*)TlsGetValue(tlsIndex);
		if (nullptr == p_connector) {

		}

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
};