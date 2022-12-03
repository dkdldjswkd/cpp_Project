#include "stdafx.h"
#include "GameManager.h"
#include "SceneManager.h"
#include "ScreenBuffer.h"
#include "DataFileIO.h"
#include "Actor.h"
#include "ObjectManager.h"

GameManager* GameManager::inst = nullptr;
extern bool END;

void GameManager::Init() {
	cs_Init();

	SceneManager::Get_inst().Init();
}

void GameManager::Run() {

	LARGE_INTEGER prev;
	LARGE_INTEGER cur;
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq); // 1���� �����ֱ�

	QueryPerformanceCounter(&prev);
	while (1) {
		// ������Ʈ (��ǥ �̵�, �÷��̾� Ű �Է�)
		SceneManager::Get_inst().Update();
		if (END) break;

		// �浹�Ǵ�
		ObjectManager::Get_inst().List_Collision();

		// ��ũ�� ���� �ֽ�ȭ
		SceneManager::Get_inst().Render();

		// ��ü���� (�浹�Ǵ� �ٰŷ� ��ü����)
		ObjectManager::Get_inst().List_Check_flag();

		// �� ��ȯ
		SceneManager::Get_inst().Change_Scene();

		Buffer_Flip();
		Buffer_Clear();

		// �ð� üũ
		QueryPerformanceCounter(&cur);

		double t = (cur.QuadPart - prev.QuadPart) / (double)(freq.QuadPart / 1000); // ms
		auto sleep_time = 50 - t;
		if (sleep_time > 0) { Sleep(sleep_time); }

		prev = cur;
	}

	ObjectManager::Get_inst().List_Delete();
}