#include "stdafx.h"
#include "Actor.h"
#include "SceneManager.h"
#include "ScreenBuffer.h"
#include "ObjectManager.h"


////////////////////////////////////
// BaseObject
////////////////////////////////////
BaseObject::BaseObject(EObjectType type, int x, int y) : object_type(type), x(x), y(y) {}
BaseObject::~BaseObject() {}

EObjectType BaseObject::Get_object_type()
{
	return object_type;
}

int BaseObject::Get_x()
{
	return x;
}

int BaseObject::Get_y()
{
	return y;
}

////////////////////////////////////
// Player
////////////////////////////////////
Player::Player(int x, int y) : BaseObject(EObjectType::PLAYER, x, y) {}

void Player::Shooting() {
	OBJECT_LIST.push_back(new Bullet(true, x, y));
}

EDir Player::Control() {
	// Shoot Check
	if (GetAsyncKeyState(0x20) != 0)
		Shooting();

	// Move Check
	if (GetAsyncKeyState(VK_UP) != 0) {
		if (GetAsyncKeyState(VK_LEFT) != 0) {
			return EDir::NW;
		}
		else if (GetAsyncKeyState(VK_RIGHT) != 0) {
			return EDir::NE;
		}
		else {
			return EDir::N;
		}
	}
	else if (GetAsyncKeyState(VK_DOWN) != 0) {
		if (GetAsyncKeyState(VK_LEFT) != 0) {
			return EDir::SW;
		}
		else if (GetAsyncKeyState(VK_RIGHT) != 0) {
			return EDir::SE;
		}
		else {
			return EDir::S;
		}
	}
	else if (GetAsyncKeyState(VK_LEFT) != 0) {
		return EDir::W;
	}
	else if (GetAsyncKeyState(VK_RIGHT) != 0) {
		return EDir::E;
	}
	else {
		return EDir::NONE;
	}

	//if (_kbhit()) {
	//	// 디버크 코드 ESC 입력시 다음 스테이지로 전환
	//	if (GetAsyncKeyState(0x1B) != 0) {
	//		game_manager_inst().Loading(ChangeScene::NextStage);
	//	}
	//}
}

void Player::Update() {
	auto dir = Control();

	switch (dir) {
	case EDir::N:
		if (Pos_Check(x, y - 1)) { y--; }
		break;

	case EDir::S:
		if (Pos_Check(x, y + 1)) { y++; }
		break;

	case EDir::W:
		if (Pos_Check(x - 1, y)) { x--; }
		break;

	case EDir::E:
		if (Pos_Check(x + 1, y)) { x++; }
		break;

	case EDir::NW:
		if (Pos_Check(x, y - 1)) { y--; }
		if (Pos_Check(x - 1, y)) { x--; }
		break;

	case EDir::NE:
		if (Pos_Check(x, y - 1)) { y--; }
		if (Pos_Check(x + 1, y)) { x++; }
		break;

	case EDir::SW:
		if (Pos_Check(x, y + 1)) { y++; }
		if (Pos_Check(x - 1, y)) { x--; }
		break;

	case EDir::SE:
		if (Pos_Check(x, y + 1)) { y++; }
		if (Pos_Check(x + 1, y)) { x++; }
		break;
	}
}

void Player::Render() {
	Sprite_Draw(x, y, PLAYER_CHAR);
}

////////////////////////////////////
// Enemy
////////////////////////////////////

Enemy::Enemy(int x, int y, const Enemy_Type& enemy_type) : BaseObject(EObjectType::ENEMY, x, y), enemy_type(enemy_type) {
	this->x = x;
	this->y = y;
}

void Enemy::Update() {
	char pattern = enemy_type.pattern[cur_pattern];

	switch (pattern) {
	case '*':
		Shooting();
		break;
	case '0': // N
		if (Pos_Check(x, y - 1)) { y--; }
		break;
	case '1': // NE
		if (Pos_Check(x + 1, y - 1)) { x++; y--; }
		break;
	case '2': // E
		if (Pos_Check(x + 1, y)) { x++; }
		break;
	case '3': // SE
		if (Pos_Check(x - 1, y - 1)) { x--; y--; }
		break;
	case '4': // S
		if (Pos_Check(x, y + 1)) { y++; }
		break;
	case '5': // SW
		if (Pos_Check(x - 1, y + 1)) { x--; y++; }
		break;
	case '6': // W
		if (Pos_Check(x - 1, y)) { x--; }
		break;
	case '7': // NW
		if (Pos_Check(x - 1, y - 1)) { x--;	y--; }
		break;
	}

	// 마지막 패턴까지 왔다면 패턴 초기화
	if (enemy_type.pattern[cur_pattern + 1] == '\0') {
		cur_pattern = 0;
	}
	else {
		cur_pattern++;
	}
}

void Enemy::Render() {
	Sprite_Draw(x, y, enemy_type.enemy_char);
}

void Enemy::Shooting() {
	OBJECT_LIST.push_back(new Bullet(false, x, y));
}


//Enemy_Type::Enemy_Type(const Enemy_Type& other) {
//	enemy_char = other.enemy_char;
//	strcpy_s(this->pattern, MAX_ENEMY_PATTERN, other.pattern);
//}

Enemy_Type::Enemy_Type(const Enemy_Type& other) {
	enemy_char = other.enemy_char;
	strcpy_s(this->pattern, MAX_ENEMY_PATTERN, other.pattern);
}


////////////////////////////////////
// Bullet
////////////////////////////////////
Bullet::Bullet(bool mine, int x, int y) : BaseObject(EObjectType::BULLET, x, y), is_mine(mine) {
	if (is_mine) {
		bullet_char = '^';
	}
	else {
		bullet_char = '!';
	}
}

void Bullet::Update() {

	if (is_mine) {
		y--;
	}
	else {
		y++;
	}

	if (!Pos_Check(x, y)) {
		flag = false;
	}
}

void Bullet::Render() {
	Sprite_Draw(x, y, bullet_char);
}

ChoiceObject::ChoiceObject() : BaseObject(EObjectType::CHOICE, x = 30, y = 18) {}

void ChoiceObject::Update() {
	if (GetAsyncKeyState(VK_UP) != 0) {
		choice = EChoice::UP;
		y = 17;
	}
	else if (GetAsyncKeyState(VK_DOWN) != 0) {
		choice = EChoice::DOWN;
		y = 19;
	}
}

void ChoiceObject::Render()
{
	Sprite_Draw(x, y, choice_char);
}
