#include "stdafx.h"
#include "Render.h"
#include "Protocol.h"
#include "Actor.h"
#include "Server.h"
#include "Error_Check.h"
using namespace std;

int my_id = 9999;

int main() {
	cs_Init();

	StarServer::Get_Inst().Init();
	StarServer::Get_Inst().Run();
	StarServer::Get_Inst().Close();
}