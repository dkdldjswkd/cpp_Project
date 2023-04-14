// C Connector ����  Mysql 8.0 ����� �Ʒ� ����ó�� �н����� ����� ���� ������� ���� �ؾ���
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
	int loggingTime; // �ð� �ʰ� �� �α�

public:
	void ConnectDB(const char* dbAddr, int port, const char* loginID, 
		const char* password, const char* schema, unsigned short loggingTime = INFINITE);
	MYSQL_RES* Query(const char* queryFormat, ...);
	MYSQL_RES* Query(const char* queryFormat, va_list args);
};