#pragma once
#include "Actor.h"

class BaseObject;
class Player;
struct Enemy_Type;

enum class EScene {
	TITLE,
	OVER,
	GAME,
	CLEAR,
	NONE
};

class SceneBase {
public:
	SceneBase(EScene);
	virtual ~SceneBase();

protected:
	// Scene
	EScene next_scene;

public:
	// Scene
	const EScene my_scene;

public:
	virtual void Update() = 0;
	virtual void Render() = 0;
	EScene Change_Scene();
};

class SceneGame  : public SceneBase {
public:
	SceneGame();

private:
	// Stage
	char	stage_buf[dfSCREEN_HEIGHT][dfSCREEN_WIDTH] = { 0, };
	string	stage_name[MAX_STAGE];
	int		cur_stage = -1;
	bool	stage_init = false; // 스테이지 진입 판단
	int		cur_stage_enemies_no = 0;

public:
	void Update();
	void Render();
};

class SceneTitle : public SceneBase{
public:
	SceneTitle();

private:

public:
	ChoiceObject* p_choice;

public:
	void Update();
	void Render();
};

class SceneOver : public SceneBase {
public:
	SceneOver();

public:
	ChoiceObject* p_choice;

public:
	void Update();
	void Render();
};

class SceneClear : public SceneBase {
public:
	SceneClear();

public:
	ChoiceObject* p_choice;

public:
	void Update();
	void Render();
};

class SceneManager {
private:
	SceneManager();
	~SceneManager();
	static SceneManager* inst;

public:
	static SceneManager& Get_inst() {
		if (inst == nullptr) {
			inst = new SceneManager;
			atexit(destroy);
		}

		return *inst;
	}

	static void destroy() {
		delete inst;
	}

private:
	// Scene
	SceneBase* p_scene = NULL;

public:
	// Screen Buf
	const char	origin_title_buf[dfSCREEN_HEIGHT * dfSCREEN_WIDTH] = { 0, };
	const char	origin_over_buf[dfSCREEN_HEIGHT * dfSCREEN_WIDTH] = { 0, };
	const char	origin_clear_buf[dfSCREEN_HEIGHT * dfSCREEN_WIDTH] = { 0, };

	// Stage
	int max_stage;
	string stage_file_name[MAX_STAGE];

	// Enemy
	int			max_enemy_type;
	Enemy_Type	enemies_type[MAX_ENEMY_TYPE];
	string		enemy_file_name[MAX_ENEMY_TYPE];

private:
	void Screen_Set();
	void Game_Set();

public:
	void Init();
	void Update();
	void Render();
	void LoadScene(EScene e_scene);
	void Change_Scene();
};