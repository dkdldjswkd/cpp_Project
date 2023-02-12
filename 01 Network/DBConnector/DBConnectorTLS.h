// ���� 12:39 2023-02-13
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
	const unsigned short loggingTime; // ���� �� �ش� �ð� �ʰ� �� �α�

public:
	MYSQL_RES* Query(const char* queryFormat, ...);
};