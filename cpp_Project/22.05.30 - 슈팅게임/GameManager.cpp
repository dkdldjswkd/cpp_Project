#include "stdafx.h"
#include "GameManager.h"
#include "DataFileIO.h"

Enemy_Bullet Enemy::enemy_bullets[MAX_ENEMY_BULLET];

GameManager::GameManager() : player(40, 20) {};

GameManager& GameManager::GetInst() {
	static GameManager* inst = new GameManager();
	return *inst;
}

void GameManager::Loading(ChangeScene change_scene) {
	this->next_scene = SceneState::LOAD;
	this->change_scene = change_scene;
}

void GameManager::Init() {
	// 콘솔 초기화
	cs_Init();

	char buf[256];
	Fread_to_buf(buf, 256, STAGEINFO_FILE_NAME);
	char* context;

	// max_stage값 설정
	max_stage = atoi(strtok_s(buf, "\n", &context));
	enemies_of_stage.resize(max_stage);
	// 스테이지 데이터 파일이름 설정
	stage_file_names.resize(max_stage);
	for (int i = 0; i < max_stage; i++) {
		stage_file_names[i] = strtok_s(NULL, "\n", &context);
	}

	Fread_to_buf(buf, 256, ENEMYINFO_FILE_NAME);
	// resize enemy_types
	enemy_types.resize(atoi(strtok_s(buf, "\n", &context)));
	// 적 데이터 파일이름 설정
	enemy_file_names.resize(enemy_types.size());
	for (int i = 0; i < enemy_file_names.size(); i++) {
		enemy_file_names[i] = strtok_s(NULL, "\n", &context);
	}

	// enemy_types 초기화
	for (int i = 0; i < enemy_file_names.size(); i++) {
		Fread_to_buf(buf, 256, enemy_file_names[i].c_str());
		enemy_types[i].enemy_char = *strtok_s(buf, "\n", &context);
		strcpy_s(enemy_types[i].pattern, MAX_PATTERN, strtok_s(NULL, "\n", &context));
	}

	// enemies_of_stage Set // 스테이지 당 적의 수, 적의 타입, 적의 위치 등
	for (int i = 0; i < max_stage; i++) {
		Fread_to_ScreenBuf(stage_file_names[i].c_str());
		for (int y = 0; y < dfSCREEN_HEIGHT; y++) {
			for (int x = 0; x < dfSCREEN_WIDTH - 1; x++) {
				if (szScreenBuffer[y][x] != ' ' && szScreenBuffer[y][x] != '@') {
					for (int j = 0; j < enemy_types.size(); j++) {
						if (szScreenBuffer[y][x] == enemy_types[j].enemy_char) {
							enemies_of_stage[i].emplace_back(enemy_types[j], x, y); // 3, 6, 8
						}
					}
				}
			}
		}
	}

	Loading(ChangeScene::GoTitle);
}

void GameManager::StartGame() {
	double frame_time;
	unsigned int frame = 0;

	LARGE_INTEGER Start;
	LARGE_INTEGER End;
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq); // 1초의 진동주기

	while (1)
	{
		QueryPerformanceCounter(&Start);

		switch (game_manager_inst().next_scene)
		{
		case SceneState::TITLE:
			UpdateTitle();
			//printf("\nSceneState::TITLE\n");
			break;

		case SceneState::LOAD:
			UpdateLoad();
			//printf("\nSceneState::LOAD\n");
			break;

		case SceneState::STAGE:
			UpdateStage();
			//printf("\nSceneState::STAGE\n");
			break;

		case SceneState::CLEAR:
			UpdateClear();
			//printf("\nSceneState::CLEAR\n");
			break;

		case SceneState::OVER:
			UpdateOver();
			//printf("\nSceneState::OVER\n");
			break;

		case SceneState::EXIT:
			goto exit;
		}

		QueryPerformanceCounter(&End);
		double t = (End.QuadPart - Start.QuadPart) / (double)(Freq.QuadPart / 1000); // ms
		auto sleep_time = 50 - t;

		if (sleep_time > 0) {
			frame_time = 50;
			Sleep(sleep_time);
		}
		else {
			frame_time = t;
		}

		//cout << "frame : " << frame++ << " / frame_time : / " << frame_time << " / t : " << t << endl;
	}

exit:
	return;
}

bool Press_Check(int key = 0x0D) {
	if (_kbhit()) {
		if (key != 0) {
			if (GetAsyncKeyState(key) != 0) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			auto ret = _getch();
			return true;
		}
	}
	return false;
}

void GameManager::UpdateStage() {
	Buffer_Clear();

	// 적 이동, 총알 발사
	for (int i = 0; i < enemies_of_curStage.size(); i++) {
		int& cur_pattern = enemies_of_curStage[i].my_type.cur_pattern;
		char pattern = enemies_of_curStage[i].my_type.pattern[cur_pattern];

		switch (pattern) {
		case '*':
			enemies_of_curStage[i].Shooting();
			break;
		case '0':
			enemies_of_curStage[i].move(Move_Dir::N);
			break;
		case '1':
			enemies_of_curStage[i].move(Move_Dir::NE);
			break;
		case '2':
			enemies_of_curStage[i].move(Move_Dir::E);
			break;
		case '3':
			enemies_of_curStage[i].move(Move_Dir::SE);
			break;
		case '4':
			enemies_of_curStage[i].move(Move_Dir::S);
			break;
		case '5':
			enemies_of_curStage[i].move(Move_Dir::SW);
			break;
		case '6':
			enemies_of_curStage[i].move(Move_Dir::W);
			break;
		case '7':
			enemies_of_curStage[i].move(Move_Dir::NW);
			break;
		}

		// 마지막 패턴까지 왔다면 패턴 초기화
		if (enemies_of_curStage[i].my_type.pattern[cur_pattern + 1] == '\0') {
			cur_pattern = 0;
		}
		else {
			cur_pattern++;
		}
	}

	// 플레이어 이동, 총알 발사
	game_manager_inst().player.Player_Controll();

	// 플레이어 총알 업데이트
	game_manager_inst().player.Update_Bullet();

	// 적 총알 업데이트
	Enemy::Update_Bullet();

	// 적 총알 충돌 판단, 적 총알 그리기
	for (auto& enemy_bullet : Enemy::enemy_bullets) {
		if (enemy_bullet.flag) {
			if (enemy_bullet.Collision_Ceck(game_manager_inst().player.x, game_manager_inst().player.y)) {
				game_manager_inst().Loading(ChangeScene::GameOver);
			}
			else {
				Sprite_Draw(enemy_bullet.x, enemy_bullet.y, enemy_bullet.bullet_char);
			}
		}
	}

	// 플레이어 총알 충돌 판단, 총알 그리기
	for (auto& bullet : game_manager_inst().player.player_bullets) {
		if (bullet.flag) {
			for (int i = 0; i < enemies_of_curStage.size(); i++) {
				// 도는 중 삭제, 위험한 코드?
				if(bullet.Collision_Ceck(enemies_of_curStage[i].x, enemies_of_curStage[i].y)){
					enemies_of_curStage.erase(enemies_of_curStage.begin() + i);
					i--;
					if (enemies_of_curStage.size() == 0) {
						Loading(ChangeScene::NextStage);
					}
				}
				else {
					Sprite_Draw(bullet.x, bullet.y, bullet.bullet_char);
				}
			}
		}
	}

	// 플레이어 그리기
	Sprite_Draw(game_manager_inst().player.x, game_manager_inst().player.y, game_manager_inst().player.player_char);

	// 적 그리기
	for (int i = 0; i < enemies_of_curStage.size(); i++) {
		Sprite_Draw(enemies_of_curStage[i].x, enemies_of_curStage[i].y, enemies_of_curStage[i].my_type.enemy_char);
	}

	//화면 출력
	Buffer_Flip();
}

void GameManager::UpdateLoad() {
	switch (game_manager_inst().change_scene) {
	case ChangeScene::GoTitle:
		game_manager_inst().next_scene = SceneState::TITLE;
		break;

	case ChangeScene::NextStage:
		InitStage();

		if (++game_manager_inst().cur_stage < game_manager_inst().max_stage) {
			game_manager_inst().next_scene = SceneState::STAGE;

			Buffer_Clear();

			auto& index = game_manager_inst().cur_stage;
			enemies_of_curStage.clear();
			enemies_of_curStage = enemies_of_stage[index];

			for (int i = 0; i < enemies_of_curStage.size(); i++) {
				Sprite_Draw(enemies_of_curStage[i].x, enemies_of_curStage[i].y, enemies_of_curStage[i].my_type.enemy_char);
			}
			Sprite_Draw(game_manager_inst().player.x, game_manager_inst().player.y, game_manager_inst().player.player_char);
		}
		else {
			game_manager_inst().next_scene = SceneState::CLEAR;
			InitStage();
			cur_stage = -1;
		}
		break;

	case ChangeScene::GameOver:
		game_manager_inst().next_scene = SceneState::OVER;
		InitStage();
		cur_stage = -1;
		break;

	case ChangeScene::Exit:
		game_manager_inst().next_scene = SceneState::EXIT;

		break;
	}
}

void GameManager::UpdateTitle() {
	if (_kbhit()) {
		// ESC
		if (GetAsyncKeyState(0x1B) != 0) {
			{
				game_manager_inst().Loading(ChangeScene::Exit);
				return;
			}
		}
		// ENTER
		else if (GetAsyncKeyState(0x0D) != 0) {
			game_manager_inst().Loading(ChangeScene::NextStage);
		}
		else {
			auto ret = _getch();
		}
	}

	Fread_to_ScreenBuf(TITLE_FILE_NAME);
	Buffer_Flip();
}

void GameManager::UpdateClear() {
	if (_kbhit()) {
		// ESC
		if (GetAsyncKeyState(0x1B) != 0) {
			{
				game_manager_inst().Loading(ChangeScene::Exit);
				return;
			}
		}
		// ENTER
		else if (GetAsyncKeyState(0x0D) != 0) {
			game_manager_inst().Loading(ChangeScene::GoTitle);
		}
		else {
			auto ret = _getch();
		}
	}

	Fread_to_ScreenBuf(CLEAR_FILE_NAME);
	Buffer_Flip();
}

void GameManager::UpdateOver() {
	if (_kbhit()) {
		// ESC
		if (GetAsyncKeyState(0x1B) != 0) {
			{
				game_manager_inst().Loading(ChangeScene::Exit);
				return;
			}
		}
		// ENTER
		else if (GetAsyncKeyState(0x0D) != 0) {
			game_manager_inst().Loading(ChangeScene::GoTitle);
		}
		else {
			auto ret = _getch();
		}
	}

	Fread_to_ScreenBuf(OVER_FILE_NAME);
	Buffer_Flip();
}

void GameManager::InitStage() {
	player.x = 40;
	player.y = 20;
	for (auto& bullet : player.player_bullets) {
		bullet.flag = false;
	}

	for (auto& enemy_bullet : Enemy::enemy_bullets) {
		enemy_bullet.flag = false;
	}
}
