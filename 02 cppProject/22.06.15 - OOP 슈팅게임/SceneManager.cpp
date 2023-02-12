#include "stdafx.h"
#include "SceneManager.h"
#include "DataFileIO.h"
#include "GameManager.h"
#include "ObjectManager.h"

SceneManager* SceneManager::inst = nullptr;
extern bool END;

////////////////////////////////////
// SceneBase
////////////////////////////////////
SceneBase::SceneBase(EScene my) : my_scene(my), next_scene(my) {}

SceneBase::~SceneBase() {
	ObjectManager::Get_inst().List_Delete();
}

EScene SceneBase::Change_Scene() {
	if (my_scene == next_scene) {
		return EScene::NONE;
	}

	return next_scene;
}

////////////////////////////////////
// SceneGame
////////////////////////////////////
SceneGame::SceneGame() : SceneBase(EScene::GAME), cur_stage(-1) {}

void SceneGame::Update() {
	// stage 진입 여부 체크
	if (stage_init == false)
	{
		cur_stage_enemies_no = 0;
		stage_init = true;
		cur_stage++;
		ObjectManager::Get_inst().List_Delete();


		// max_stage(Index Size)가 cur_stage() 보다 크지않다면 Clear Scene으로 넘어감 ()
		if (!(cur_stage < SceneManager::Get_inst().max_stage)) { 
			next_scene = EScene::CLEAR; return; 
		}

		Fread_to_buf(stage_buf[0], SCREEN_SIZE, SceneManager::Get_inst().stage_file_name[cur_stage].c_str());

		// 리스트 셋팅
		for (int y = 0; y < dfSCREEN_HEIGHT; y++) {
			for (int x = 0; x < dfSCREEN_WIDTH - 1; x++) {

				if (stage_buf[y][x] != ' ' && stage_buf[y][x] != '@') {
					if (stage_buf[y][x] == PLAYER_CHAR) {
						OBJECT_LIST.push_back(new Player(x, y));
					}
					else {
						for (int i = 0; i < SceneManager::Get_inst().max_enemy_type; i++) {
							if (stage_buf[y][x] == SceneManager::Get_inst().enemies_type[i].enemy_char) {
								OBJECT_LIST.push_back(new Enemy(x, y, SceneManager::Get_inst().enemies_type[i]));
								cur_stage_enemies_no++;
								break;
							}
						}
					}
				}
			}
		}
	}

	auto player_survive = ObjectManager::Get_inst().List_Check_have(EObjectType::PLAYER, 1);
	// 플레이어가 죽었다면
	if (player_survive == false) {
		next_scene = EScene::OVER;
	}

	auto enemies_survive = ObjectManager::Get_inst().List_Check_have(EObjectType::ENEMY, 1);
	// 적들이 전부 죽었다면
	if (enemies_survive == false) {
		stage_init = false;
	}

	// 디버크 코드 ESC 입력시 다음 스테이지로 전환
	if (GetAsyncKeyState(0x1B) != 0) {
		stage_init = false;
	}

	ObjectManager::Get_inst().List_Update();
}

void SceneGame::Render() {
	ObjectManager::Get_inst().List_Render();
}


////////////////////////////////////
// SceneTitle
////////////////////////////////////
SceneTitle::SceneTitle() : SceneBase(EScene::TITLE) {
	ObjectManager::Get_inst().List_Delete();
	p_choice = new ChoiceObject();
	OBJECT_LIST.push_back(p_choice);
}

void SceneTitle::Update() {
	// 엔터 입력
	if (GetAsyncKeyState(0x0D) != 0) {
		if (p_choice->choice == EChoice::UP) {
			next_scene = EScene::GAME;
		}
		else if (p_choice->choice == EChoice::DOWN) {
			END = true;
		}
	}

	ObjectManager::Get_inst().List_Update();
}

void SceneTitle::Render() {
	memcpy_s(szScreenBuffer, SCREEN_SIZE, SceneManager::Get_inst().origin_title_buf, SCREEN_SIZE);

	ObjectManager::Get_inst().List_Render();
}

////////////////////////////////////
// SceneOver
////////////////////////////////////
SceneOver::SceneOver() : SceneBase(EScene::OVER) {
	ObjectManager::Get_inst().List_Delete();
	p_choice = new ChoiceObject();
	OBJECT_LIST.push_back(p_choice);
}

void SceneOver::Update() {
	// 엔터 입력
	if (GetAsyncKeyState(0x0D) != 0) {
		if (p_choice->choice == EChoice::UP) {
			next_scene = EScene::GAME;
		}
		else if (p_choice->choice == EChoice::DOWN) {
			END = true;
		}
	}

	ObjectManager::Get_inst().List_Update();
}

void SceneOver::Render() {
	memcpy_s(szScreenBuffer, SCREEN_SIZE, SceneManager::Get_inst().origin_over_buf, SCREEN_SIZE);

	ObjectManager::Get_inst().List_Render();
}

////////////////////////////////////
// SceneClear
////////////////////////////////////
SceneClear::SceneClear() : SceneBase(EScene::OVER) {
	ObjectManager::Get_inst().List_Delete();
	p_choice = new ChoiceObject();
	OBJECT_LIST.push_back(p_choice);
}

void SceneClear::Update()
{
	// 엔터 입력
	if (GetAsyncKeyState(0x0D) != 0) {
		if (p_choice->choice == EChoice::UP) {
			next_scene = EScene::GAME;
		}
		else if (p_choice->choice == EChoice::DOWN) {
			END = true;
		}
	}

	ObjectManager::Get_inst().List_Update();
}

void SceneClear::Render() {
	memcpy_s(szScreenBuffer, SCREEN_SIZE, SceneManager::Get_inst().origin_clear_buf, SCREEN_SIZE);

	ObjectManager::Get_inst().List_Render();
}


////////////////////////////////////
// SceneManager
////////////////////////////////////
SceneManager::SceneManager() {
	// ...
}

SceneManager::~SceneManager() {
	delete p_scene;
}

void SceneManager::Screen_Set() {
	auto cast_origin_title_buf = const_cast<char*>(origin_title_buf);
	Fread_to_buf(cast_origin_title_buf, SCREEN_SIZE, TITLE_FILE);

	auto cast_origin_clear_buf = const_cast<char*>(origin_clear_buf);
	Fread_to_buf(cast_origin_clear_buf, SCREEN_SIZE, CLEAR_FILE);

	auto cast_origin_over_buf = const_cast<char*>(origin_over_buf);
	Fread_to_buf(cast_origin_over_buf, SCREEN_SIZE, OVER_FILE);
}

void SceneManager::Game_Set() {
	// Stage Info Set
	char stage_info_buf[MAX_INFO_SIZE] = {0, };
	Fread_to_buf(stage_info_buf, MAX_INFO_SIZE, STAGEINFO_FILE);

	char* context = nullptr;
	max_stage = atoi(strtok_s(stage_info_buf, "\n", &context));

	for (int i = 0; i < max_stage; i++) {
		stage_file_name[i] = strtok_s(NULL, "\n", &context);
	}

	// Enemy Info Set
	char enemy_info_buf[MAX_INFO_SIZE] = {0, };
	Fread_to_buf(enemy_info_buf, MAX_INFO_SIZE, ENEMYINFO_FILE);

	max_enemy_type = atoi(strtok_s(enemy_info_buf, "\n", &context));

	for (int i = 0; i < max_enemy_type; i++) {
		enemy_file_name[i] = strtok_s(NULL, "\n", &context);
	}

	char enemy_file_buf[MAX_ENEMY_FILE_SIZE] = { 0, };
	for (int i = 0; i < max_enemy_type; i++) {
		Fread_to_buf(enemy_file_buf, MAX_ENEMY_FILE_SIZE, enemy_file_name[i].c_str());

		enemies_type[i].enemy_char = *strtok_s(enemy_file_buf, "\n", &context);
		strcpy_s(enemies_type[i].pattern, MAX_ENEMY_PATTERN, strtok_s(NULL, "\n", &context));
	}
}

void SceneManager::Init() {
	Screen_Set();
	Game_Set();

	LoadScene(EScene::TITLE);
}

void SceneManager::Update() {
	p_scene->Update();
}

void SceneManager::Render() {
	p_scene->Render();
}

void SceneManager::LoadScene(EScene e_scene) {
	if (p_scene != NULL) {
		delete p_scene;
	}

	switch (e_scene) {
	case EScene::TITLE:
		p_scene = new SceneTitle;
		break;

	case EScene::GAME:
		p_scene = new SceneGame;
		break;

	case EScene::OVER:
		p_scene = new SceneOver;
		break;

	case EScene::CLEAR:
		p_scene = new SceneClear;
		break;

	case EScene::NONE:
		break;
	}
}

void SceneManager::Change_Scene() {
	auto ret = p_scene->Change_Scene();

	if (ret != EScene::NONE) {
		LoadScene(ret);
	}
}