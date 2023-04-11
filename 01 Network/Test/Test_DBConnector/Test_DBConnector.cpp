#include <iostream>
#include <Windows.h>
#include "../../DBConnector/DBConnectorTLS.h"

#define DB_IP		"127.0.0.1"
#define DB_PORT		3306
#define DB_ID		"root"
#define DB_PASSWORD "password"
#define DB_SCHEMA	"accountdb"
#define DB_LOGTIME	200

using namespace std;

int main() {
	DBConnectorTLS dbSession(
		DB_IP,
		DB_PORT,
		DB_ID,
		DB_PASSWORD,
		DB_SCHEMA,
		DB_LOGTIME
	);

	//DBConnector dbSession(
	//	DB_IP,
	//	DB_PORT,
	//	DB_ID,
	//	DB_PASSWORD,
	//	DB_SCHEMA,
	//	DB_LOGTIME
	//);

	try {
		MYSQL_RES* sql_res = dbSession.Query("SELECT accountno, usernick FROM account where accountno = %d", 9999999999999);
		for (;;) {
			MYSQL_ROW sql_row = mysql_fetch_row(sql_res);
			if (NULL == sql_row) break;

			printf("%s %s \n", sql_row[0], sql_row[1]);
		}
		mysql_free_result(sql_res);
	}
	catch (DBException& e) {
		cout << "catch" << endl;
		cout << e.errorQuery << endl;
		cout << e.errorNo << endl;
		cout << e.errorStr << endl;
	}
}