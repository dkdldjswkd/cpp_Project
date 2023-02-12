#include "stdafx.h"
#include "Actor.h"
#include "GameManager.h"

Movable_Object::Movable_Object() {}
Movable_Object::Movable_Object(int x, int y) : Pos(x, y) {}
void Movable_Object::move(Move_Dir dir) {
	switch (dir) {
	case  Move_Dir::N:
		if (y > 0)
			y--;
		break;

	case  Move_Dir::S:
		if (y < dfSCREEN_HEIGHT - 1)
			y++;
		break;

	case  Move_Dir::W:
		if (x > 0)
			x--;
		break;

	case  Move_Dir::E:
		if (x < dfSCREEN_WIDTH - 2)
			x++;
		break;

	case  Move_Dir::NW:
		if (x > 0)
			x--;
		if (y > 0)
			y--;
		break;

	case  Move_Dir::NE:
		if (x < dfSCREEN_WIDTH - 2)
			x++;
		if (y > 0)
			y--;
		break;

	case  Move_Dir::SW:
		if (x > 0)
			x--;
		if (y < dfSCREEN_HEIGHT - 1)
			y++;
		break;

	case  Move_Dir::SE:
		if (x < dfSCREEN_WIDTH - 2)
			x++;
		if (y < dfSCREEN_HEIGHT - 1)
			y++;
		break;

	case  Move_Dir::NONE:
		break;
	}
}

Enemy::Enemy(Enemy_Type type, int x, int y) : Movable_Object(x, y) {
	my_type.enemy_char = type.enemy_char;
	strcpy_s(my_type.pattern, MAX_PATTERN, type.pattern);
}

void Enemy::Shooting() {
	for (auto& bullet : enemy_bullets) {
		if (bullet.flag == false) {
			bullet.set(x, y);
			break;
		}
	}
}

void Enemy::Update_Bullet() {
	for (auto& bullet : enemy_bullets) {
		if (bullet.flag) {
			bullet.Update();
		}
	}
}

Player::Player(int x, int y) :Movable_Object(x, y) {}

void Player::Shooting() {
	for (auto& bullet : player_bullets) {
		if (bullet.flag == false) {
			bullet.set(x, y);
			break;
		}
	}
}

void Player::Update_Bullet() {
	for (auto& bullet : player_bullets) {
		if (bullet.flag) {
			bullet.Update();
		}
	}
}

bool Player::Player_Controll() {
	if (_kbhit()) {
		// 총알발사 체크
		if (GetAsyncKeyState(0x20) != 0)
			Shooting();

		// 움직임 체크
		if (GetAsyncKeyState(VK_UP) != 0) {
			if (GetAsyncKeyState(VK_LEFT) != 0) {
				move(Move_Dir::NW);
			}
			else if (GetAsyncKeyState(VK_RIGHT) != 0) {
				move(Move_Dir::NE);
			}
			else {
				move(Move_Dir::N);
			}
		}
		else if (GetAsyncKeyState(VK_DOWN) != 0) {
			if (GetAsyncKeyState(VK_LEFT) != 0) {
				move(Move_Dir::SW);
			}
			else if (GetAsyncKeyState(VK_RIGHT) != 0) {
				move(Move_Dir::SE);
			}
			else {
				move(Move_Dir::S);
			}
		}
		else if (GetAsyncKeyState(VK_LEFT) != 0) {
			move(Move_Dir::W);
		}
		else if (GetAsyncKeyState(VK_RIGHT) != 0) {
			move(Move_Dir::E);
		}
		else {
			move(Move_Dir::NONE);
		}

		// 디버크 코드 ESC 입력시 다음 스테이지로 전환
		if (GetAsyncKeyState(0x1B) != 0) {
			game_manager_inst().Loading(ChangeScene::NextStage);
		}

		return true;
	}
	return false;
}