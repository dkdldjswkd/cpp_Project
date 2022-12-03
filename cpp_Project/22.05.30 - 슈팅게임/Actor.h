#pragma once

#define MAX_PLAYER_BULLET 128
#define MAX_ENEMY_BULLET 128
#define MAX_PATTERN 256

enum class Move_Dir {
	NONE,
	N, S, W, E,
	NW, NE, SW, SE,
};

struct Pos {
	Pos() {}
	Pos(int x, int y) : x(x), y(y) {}
	int x;
	int y;
};

struct Movable_Object : public Pos {
	Movable_Object();
	Movable_Object(int x, int y);

	void move(Move_Dir dir);
};

struct Enemy_Type {
	char enemy_char = ' ';
	char pattern[MAX_PATTERN];
	int cur_pattern = 0;
};

struct Enemy_Bullet : public Pos {
	const char bullet_char = '*';
	bool flag = false;

	void set(int x, int y) {
		this->x = x;
		this->y = y;
		flag = true;
	}

	//좌표 업데이트, flag 최신화
	void Update() {
		if (!Pos_Check(x, ++y)) {
			flag = false;
		}
	}

	// 충돌체크
	bool Collision_Ceck(int x, int y) {
		if (this->x == x && (this->y == y || this->y - 1 == y)) {
			flag = false;
			return true;
		}
		return false;
	}
};

struct Enemy : public Movable_Object {
	Enemy(Enemy_Type type, int x, int y);

	Enemy_Type my_type;

	static Enemy_Bullet enemy_bullets[MAX_ENEMY_BULLET];

	static void Update_Bullet();
	void Shooting();
};

struct Player_Bullet {
	const char bullet_char = '^';
	bool flag = false;
	int x;
	int y;

	void set(int x, int y) {
		this->x = x;
		this->y = y;
		flag = true;
	}

	//좌표 업데이트, flag 최신화
	void Update() {
		if (!Pos_Check(x, --y)) {
			flag = false;
		}
	}

	// 충돌체크
	bool Collision_Ceck(int x, int y) {
		// 세로 판정 한칸 더
		if (this->x == x && (this->y == y || this->y + 1 == y)) {
			flag = false;
			return true;
		}
		return false;
	}
};

struct Player : public Movable_Object {
	Player(int x, int y);
	const char player_char = 'A';

	Player_Bullet player_bullets[MAX_PLAYER_BULLET];

	void Update_Bullet();
	void Shooting();
	bool Player_Controll();
};