#include <iostream>
#include <Windows.h>
#include <string>
#include <exception>
#include "mysql.h"
#include "errmsg.h"
#pragma comment (lib, "libmysql.lib")

int main(){
	MYSQL conn;
	MYSQL* connection = NULL;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	int query_stat;

	// �ʱ�ȭ
	mysql_init(&conn);

	// DB ����

	connection = mysql_real_connect(&conn, "127.0.0.1", "root", "procademy", "test", 3306, (char*)NULL, 0);
	if (connection == NULL)
	{
		fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
		return 1;
	}



	// Select ������
	char* query = "SELECT * FROM account";	// From ���� DB�� �����ϴ� ���̺� ������ �����ϼ���
	query_stat = mysql_query(connection, query);
	if (query_stat != 0)
	{
		printf("Mysql query error : %s", mysql_error(&conn));
		return 1;
	}

	// ������
	sql_result = mysql_store_result(connection);		// ��� ��ü�� �̸� ������
//	sql_result=mysql_use_result(connection);		// fetch_row ȣ��� 1���� ������

	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
	{
		printf("%2s %2s %s\n", sql_row[0], sql_row[1], sql_row[2]);
	}
	mysql_free_result(sql_result);

	// DB ����ݱ�
	mysql_close(connection);




	//	int a = mysql_insert_id(connection);


	//	query_stat = mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_ON);
	//	mysql_next_result(connection);				// ��Ƽ���� ���� ���� ��� ���
	//	sql_result=mysql_store_result(connection);	// next_result �� ��� ����
}