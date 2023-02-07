#pragma once
#include <stdio.h>
#include "mysql/include/mysql.h"
#include "mysql/include/errmsg.h"
#pragma comment(lib, "mysql/lib/libmysql.lib")

#define MAX_QUERY 1024

class DBConnector {
public:
	DBConnector(const char* dbAddr, const char* loginID, const char* password, const char* schema, int port, unsigned short loggingTime = INFINITE);
	~DBConnector();

private:
	MYSQL conn;
	MYSQL* connection;
	unsigned short loggingTime; // 쿼리 중 해당 시간 초과 시 로깅

public:
	MYSQL_RES* Query(const char* queryFormat, ...);
};