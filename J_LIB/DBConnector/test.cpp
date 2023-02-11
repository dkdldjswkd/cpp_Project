#include <iostream>
#include <Windows.h>
#include "DBConnector.h"

int main() {
	DBConnector dbSession("127.0.0.1", "root", "password", "test", 3306);

	MYSQL_RES* sql_res = dbSession.Query("SELECT * FROM level");
	for (;;) {
		MYSQL_ROW sql_row = mysql_fetch_row(sql_res);
		if (NULL == sql_row) break;

		printf("%2s %2s %s\n", sql_row[0], sql_row[1], sql_row[2]);
	}
}