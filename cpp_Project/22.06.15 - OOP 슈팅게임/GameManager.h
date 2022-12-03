#pragma once

class GameManager {
private:
	GameManager() {}
	static GameManager* inst;

public:
	static GameManager& Get_inst() {
		if (inst == nullptr) {
			inst = new GameManager;
			atexit(destroy);
		}

		return *inst;
	}

	static void destroy() {
		delete inst;
	}

private:

public:

public:
	void Init();
	void Run();
};