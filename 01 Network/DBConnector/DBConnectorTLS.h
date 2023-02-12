// 오전 12:39 2023-02-13
#pragma once
#include "DBConnector.h"

#define MAX_CONNECTOR 100

class DBConnectorTLS {
public:
	DBConnectorTLS(const char* dbAddr, const int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime = INFINITE);
	~DBConnectorTLS();

private:
	const DWORD tlsIndex;
	MYSQL* handleArray[MAX_CONNECTOR];
	int handleIndex = -1;

private:
	const char* const dbAddr;
	const char* const loginID;
	const char* const password;
	const char* const schema;
	const int port;
	const unsigned short loggingTime; // 쿼리 중 해당 시간 초과 시 로깅

public:
	MYSQL_RES* Query(const char* queryFormat, ...);
};