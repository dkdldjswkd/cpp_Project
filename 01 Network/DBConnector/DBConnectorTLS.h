#pragma once
#include "DBConnector.h"

#define MAX_CONNECTOR 100

class DBConnectorTLS {
public:
	DBConnectorTLS(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime);
	~DBConnectorTLS();

private:
	// tls
	const int tlsIndex;
	int useIndex = -1;
	DBConnector* connectorArr[MAX_CONNECTOR];

	// DBConnector 생성자 전달 인자
	char dbAddr[20];
	int port;
	char loginID[100];
	char password[100];
	char schema[100];
	unsigned short loggingTime = INFINITE;

private:
	DBConnector* Get();

public:
	MYSQL_RES* Query(const char* queryFormat, ...);
};