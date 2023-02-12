#include <iostream>
#include <Windows.h>
#include "DBConnector.h"

#define DB_IP		"127.0.0.1"
#define DB_PORT		3306
#define DB_ID		"root"
#define DB_PASSWORD "password"
#define DB_SCHEMA	"test"
#define DB_LOGTIME	200

int main() {
	DBConnector dbSession(
		DB_IP,
		DB_PORT,
		DB_ID,
		DB_PASSWORD,
		DB_SCHEMA,
		DB_LOGTIME
	);

	MYSQL_RES* sql_res = dbSession.Query("SELECT * FROM level");
	for (;;) {
		MYSQL_ROW sql_row = mysql_fetch_row(sql_res);
		if (NULL == sql_row) break;

		printf("%2s %2s %s\n", sql_row[0], sql_row[1], sql_row[2]);
	}
}