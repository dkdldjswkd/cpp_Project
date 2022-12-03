#pragma once
#include "Actor.h"

#define game_manager_inst() GameManager::GetInst()

enum class SceneState {
	TITLE,
	LOAD,
	STAGE,
	CLEAR,
	OVER,
	EXIT,
};

enum class ChangeScene {
	GoTitle, // load to title
	NextStage, // load to stage or clear
	GameOver, // load to gameover
	Exit, // exit game
};


class GameManager {
private:
	GameManager();

public:
	static GameManager& GetInst();

public:
	// Player
	Player player;

	// Enemy
	vector<Enemy_Type> enemy_types; // ������ ���� �� ex.1,2,3 == 3
	vector<string> enemy_file_names;
	vector<vector<Enemy>> enemies_of_stage;

	// Stage
	unsigned short max_stage = 0;
	vector<string> stage_file_names;

	// Scene
	SceneState next_scene = SceneState::LOAD;
	ChangeScene change_scene = ChangeScene::GoTitle;

	// InGame
	vector<Enemy> enemies_of_curStage;
	short cur_stage = -1; // ù �������� ���� �� 0, ������ �������� ���� �� max_stage -1, ������ �������� Ŭ���� �� max_stage

private:

public:
	void Init();
	void Loading(ChangeScene change_scene);
	void StartGame();

private:
	void UpdateTitle();
	void UpdateStage();
	void UpdateLoad();
	void UpdateClear();
	void UpdateOver();
	void InitStage();
};