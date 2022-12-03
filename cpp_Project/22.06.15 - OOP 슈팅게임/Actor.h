#pragma once

enum class EObjectType {
	BASE,
	ENEMY,
	PLAYER,
	BULLET,
	CHOICE,
};

enum class EDir {
	NONE,
	N, S, W, E,
	NW, NE, SW, SE,
};

class BaseObject {
public:
	BaseObject(EObjectType type, int x, int y);
	virtual ~BaseObject();

protected:
	EObjectType object_type;
	int x;
	int y;

public:
	bool flag = true;

public:
	virtual void Update() = 0;
	virtual void Render() = 0;

	EObjectType Get_object_type();
	int Get_x();
	int Get_y();
};

enum class EChoice {
	NONE,
	UP,
	DOWN,
};

class ChoiceObject : public BaseObject {
public:
	ChoiceObject();

private:
	const char choice_char = '>';

public:
	EChoice choice = EChoice::NONE;
	void Update();
	void Render();
};

struct Enemy_Type {
	Enemy_Type() = default;
	Enemy_Type(const Enemy_Type& other);

	char enemy_char = ' ';
	char pattern[MAX_ENEMY_PATTERN] = { 0, };
};

class Enemy : public BaseObject {
public:
	Enemy(int x, int y, const Enemy_Type& enemy_type);

private:
	Enemy_Type enemy_type;
	int cur_pattern = 0;

private:
	void Shooting();

public:
	void Update();
	void Render();
};

class Player : public BaseObject {
public:
	Player(int x, int y);

public:
	EDir dir = EDir::NONE;

private:


private:
	EDir Control();
	void Shooting();

public:
	void Update();
	void Render();
};

class Bullet : public BaseObject {
public:
	Bullet(bool mine, int x, int y);

private:
	char bullet_char;

public:
	// ture -> player / false -> enemy
	bool is_mine;

public:
	void Update();
	void Render();
};