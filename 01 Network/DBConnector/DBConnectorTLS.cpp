#include "DBConnectorTLS.h"
#include <stdio.h>
#include <iostream>

DBConnectorTLS::DBConnectorTLS(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime = INFINITE) : tlsIndex(TlsAlloc()) {
	// 생성자 전달 인자 초기화
	strncpy_s(this->dbAddr, sizeof(this->dbAddr), dbAddr, sizeof(this->dbAddr) - 1);
	this->port = port;
	strncpy_s(this->loginID, sizeof(this->loginID), loginID, sizeof(this->loginID) - 1);
	strncpy_s(this->password, sizeof(this->password), password, sizeof(this->password) - 1);
	strncpy_s(this->schema, sizeof(this->schema), schema, sizeof(this->schema) - 1);
	this->loggingTime = loggingTime;
}

DBConnectorTLS::~DBConnectorTLS() {
	for (int i = 0; i <= useIndex; i++) {
		delete connectorArr[i];
	}
}

DBConnector* DBConnectorTLS::Get() {
	DBConnector* p = (DBConnector*)TlsGetValue(tlsIndex);
	if (nullptr == p) {
		p = new DBConnector(dbAddr, port, loginID, password, schema, loggingTime);
		TlsSetValue(tlsIndex, (LPVOID)p);
		connectorArr[InterlockedIncrement((LONG*)&useIndex)] = p;
	}

	return p;
}

MYSQL_RES* DBConnectorTLS::Query(const char* queryFormat, ...) {
    va_list args;
    va_start(args, queryFormat);
    MYSQL_RES* result = Get()->Query(queryFormat, args);
    va_end(args);
    return result;
}