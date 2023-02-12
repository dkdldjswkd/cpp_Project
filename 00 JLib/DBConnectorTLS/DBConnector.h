// ���� 6:11 2023-02-11
// C Connector ����  Mysql 8.0 ����� �Ʒ� ����ó�� �н����� ����� ���� ������� ���� �ؾ���
// ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'password';

#pragma once
#include <stdio.h>
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

class DBConnector {
public:
	DBConnector(const char* dbAddr, const char* loginID, const char* password, const char* schema, int port, unsigned short loggingTime = INFINITE);
	~DBConnector();

private:
	MYSQL conn;
	MYSQL* connection;
	unsigned short loggingTime; // ���� �� �ش� �ð� �ʰ� �� �α�

public:
	MYSQL_RES* Query(const char* queryFormat, ...);
};