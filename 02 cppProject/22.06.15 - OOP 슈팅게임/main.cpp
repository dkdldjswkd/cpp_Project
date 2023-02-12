#include "stdafx.h"
#include "GameManager.h"

bool END = false;

int main(void)
{
	GameManager::Get_inst().Init();
	GameManager::Get_inst().Run();
}