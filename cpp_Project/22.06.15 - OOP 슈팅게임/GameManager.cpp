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
	QueryPerformanceFrequency(&freq); // 1초의 진동주기

	QueryPerformanceCounter(&prev);
	while (1) {
		// 업데이트 (좌표 이동, 플레이어 키 입력)
		SceneManager::Get_inst().Update();
		if (END) break;

		// 충돌판단
		ObjectManager::Get_inst().List_Collision();

		// 스크린 버퍼 최신화
		SceneManager::Get_inst().Render();

		// 객체삭제 (충돌판단 근거로 객체삭제)
		ObjectManager::Get_inst().List_Check_flag();

		// 씬 전환
		SceneManager::Get_inst().Change_Scene();

		Buffer_Flip();
		Buffer_Clear();

		// 시간 체크
		QueryPerformanceCounter(&cur);

		double t = (cur.QuadPart - prev.QuadPart) / (double)(freq.QuadPart / 1000); // ms
		auto sleep_time = 50 - t;
		if (sleep_time > 0) { Sleep(sleep_time); }

		prev = cur;
	}

	ObjectManager::Get_inst().List_Delete();
}