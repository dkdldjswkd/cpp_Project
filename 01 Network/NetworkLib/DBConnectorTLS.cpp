#include "DBConnectorTLS.h"

DBConnectorTLS::DBConnectorTLS(const char* dbAddr, const char* loginID, const char* password, const char* schema, int port, unsigned short loggingTime)
	: tlsIndex(TlsAlloc()), dbAddr(dbAddr), loginID(loginID), password(password), schema(schema), port(port), loggingTime(loggingTime) {
}

DBConnectorTLS::~DBConnectorTLS() {
	for (;;) {
		MYSQL* connection;
		if (!handleStack.Pop(&connection))
			break;

		mysql_close(connection);
	}
}

MYSQL_RES* DBConnectorTLS::Query(const char* queryFormat, ...) {
	// TLS �ε������� Get
	DBConnector* p_connector = (DBConnector*)TlsGetValue(tlsIndex);
	if (nullptr == p_connector) {
		p_connector = new DBConnector(dbAddr, loginID, password, schema, port, loggingTime);
		TlsSetValue(tlsIndex, (LPVOID)p_connector);
		handleStack.Push(p_connector->connection);
	}

	// �ش� ������ connector->Query() call
	va_list var_list;
	va_start(var_list, queryFormat);
	auto ret = p_connector->Query(queryFormat, var_list);
	va_end(var_list);
	return ret;
}