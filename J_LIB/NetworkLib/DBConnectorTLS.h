// ���� 4:32 2023-02-12
#pragma once
#include "DBConnector.h"
#include "LFStack.h"

class DBConnectorTLS {
public:
	DBConnectorTLS(const char* dbAddr, const char* loginID, const char* password, const char* schema, const int port, unsigned short loggingTime = INFINITE);
	~DBConnectorTLS();

private:
	const DWORD tlsIndex;
	LFStack<MYSQL*> handleStack;

private:
	const char* dbAddr;
	const char* loginID;
	const char* password;
	const char* schema;
	const int port;
	const unsigned short loggingTime; // ���� �� �ش� �ð� �ʰ� �� �α�

public:
	MYSQL_RES* Query(const char* queryFormat, ...);
};