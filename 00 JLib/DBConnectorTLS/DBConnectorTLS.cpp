#include "DBConnectorTLS.h"
#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include "mysql/include/errmsg.h"
#pragma comment(lib, "mysql/lib/libmysql.lib")
#pragma comment(lib, "Winmm.lib")

DBConnectorTLS::DBConnectorTLS(const char* dbAddr, const char* loginID, const char* password, const char* schema, int port, unsigned short loggingTime)
	: tlsIndex(TlsAlloc()), dbAddr(dbAddr), loginID(loginID), password(password), schema(schema), port(port), loggingTime(loggingTime) {
}

DBConnectorTLS::~DBConnectorTLS() {
	mysql_close(connection);
}