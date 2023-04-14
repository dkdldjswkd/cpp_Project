// C Connector 에서  Mysql 8.0 연결시 아래 예시처럼 패스워드 방식을 예전 방식으로 변경 해야함
// ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'password';

#pragma once
#include <exception>
#include "mysql/include/mysql.h"

#define MAX_QUERY 1024

struct DBException : public std::exception {
public:
	DBException(int errorNo, const char* errorStr) : 
		errorNo(errorNo), errorStr(errorStr) {
	}
	DBException(char* errorQuery, int errorNo, const char* errorStr) : 
		errorNo(errorNo), errorStr(errorStr) {
		strncpy_s(this->errorQuery, MAX_QUERY, errorQuery, MAX_QUERY - 1);
	}

public:
	char errorQuery[MAX_QUERY];
	int errorNo;
	const char* errorStr;
};

class DBConnector {
public:
	DBConnector(const char* dbAddr, int port, const char* loginID, 
		const char* password, const char* schema, unsigned short loggingTime = INFINITE);
	~DBConnector();

private:
	// mysql
	MYSQL conn;
	MYSQL* connection = nullptr;

	// opt
	int loggingTime; // 시간 초과 시 로깅

public:
	void ConnectDB(const char* dbAddr, int port, const char* loginID, 
		const char* password, const char* schema, unsigned short loggingTime = INFINITE);
	MYSQL_RES* Query(const char* queryFormat, ...);
	MYSQL_RES* Query(const char* queryFormat, va_list args);
};