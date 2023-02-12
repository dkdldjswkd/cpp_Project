// 오전 12:39 2023-02-13
// C Connector 에서  Mysql 8.0 연결시 아래 예시처럼 패스워드 방식을 예전 방식으로 변경 해야함
// ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'password';

#pragma once
#include <exception>
#include "mysql/include/mysql.h"

#define MAX_QUERY 1024

struct DBException : public std::exception {
public:
	DBException(const char* error_code, int error_no, const char* error_str) : error_no(error_no), error_str(error_str) {}
	DBException(int error_no, const char* error_str) : error_no(error_no), error_str(error_str) {}

public:
	int error_no;
	const char* error_str;
};

class DBConnectorTLS;
class DBConnector {
public:
	DBConnector(const char* dbAddr, int port, const char* loginID, const char* password, const char* schema, unsigned short loggingTime = INFINITE);
	~DBConnector();

private:
	friend DBConnectorTLS;

private:
	MYSQL conn;
	MYSQL* connection;
	const unsigned short loggingTime; // 쿼리 중 해당 시간 초과 시 로깅

public:
	MYSQL_RES* Query(const char* queryFormat, ...);
};