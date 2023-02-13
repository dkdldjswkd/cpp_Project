#include "DBConnectorTLS.h"
#include <stdio.h>

DBConnectorTLS::DBConnectorTLS(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime)
	: tlsIndex(TlsAlloc()), dbAddr(dbAddr), port(port), loginID(loginID), password(password), schema(schema), loggingTime(loggingTime) {
}

DBConnectorTLS::~DBConnectorTLS() {
	for (int i = 0; i <= handleIndex; i++) {
		mysql_close(handleArray[i]);
	}
}

MYSQL_RES* DBConnectorTLS::Query(const char* queryFormat, ...) {
	// TLS 인덱스에서 Get
	DBConnector* p_connector = (DBConnector*)TlsGetValue(tlsIndex);
	if (nullptr == p_connector) {
		p_connector = new DBConnector(dbAddr, port, loginID, password, schema, loggingTime);
		// TLS/Array Set
		TlsSetValue(tlsIndex, (LPVOID)p_connector);
		handleArray[InterlockedIncrement((LONG*)&handleIndex)] = p_connector->connection;
	}

	// 해당 스레드 connector->Query() call
	char query[MAX_QUERY];
	va_list var_list;
	va_start(var_list, queryFormat);
	vsnprintf((char*)query, MAX_QUERY - 1, queryFormat, var_list);
	query[MAX_QUERY - 1] = 0;
	va_end(var_list);

	return p_connector->DoQuery(query);
}