#include "stdafx.h"
#include "ObjectManager.h"
#include "Actor.h"
#include "ScreenBuffer.h"

ObjectManager* ObjectManager::inst = nullptr;

void ObjectManager::List_Update(){
	list<BaseObject*>::iterator iter = object_list.begin();
	for (; iter != object_list.end(); iter++) {
		(*iter)->Update();
	}
}

void ObjectManager::List_Render() {
	list<BaseObject*>::iterator iter = object_list.begin();
	for (; iter != object_list.end(); iter++) {
		if ((*iter)->flag) {
			(*iter)->Render();
		}
	}
}

void ObjectManager::List_Collision()
{
	// ����ִ� �Ѿ� ã��
	list<BaseObject*>::iterator find_bullet_iter = object_list.begin();
	for (; find_bullet_iter != object_list.end(); find_bullet_iter++) {
		if ((*find_bullet_iter)->Get_object_type() == EObjectType::BULLET && (*find_bullet_iter)->flag == true) {
			Bullet* p_bullet = (Bullet*)(*find_bullet_iter);


			if (p_bullet->is_mine) {
				// ����ִ� Enemy ã��
				list<BaseObject*>::iterator find_enemy_iter = object_list.begin();
				for (; find_enemy_iter != object_list.end(); find_enemy_iter++) {
					if ((*find_enemy_iter)->Get_object_type() == EObjectType::ENEMY && (*find_enemy_iter)->flag == true) {
						Enemy* p_enemy = (Enemy*)(*find_enemy_iter);

						// �浹üũ �� �浹����
						auto collision = Cmp_Pos(p_bullet->Get_x(), p_bullet->Get_y(), p_enemy->Get_x(), p_enemy->Get_y());
						if (collision) { p_enemy->flag = false; p_bullet->flag = false; }
					}
				}
			}
			else {
				// ����ִ� Player ã��
				list<BaseObject*>::iterator find_player_iter = object_list.begin();
				for (; find_player_iter != object_list.end(); find_player_iter++) {
					if ((*find_player_iter)->Get_object_type() == EObjectType::PLAYER && (*find_player_iter)->flag == true) {
						Player* p_player = (Player*)(*find_player_iter);

						// �浹üũ �� �浹����
						auto collision = Cmp_Pos(p_bullet->Get_x(), p_bullet->Get_y(), p_player->Get_x(), p_player->Get_y());
						if (collision) { p_player->flag = false; p_bullet->flag = false; }
					}
				}
			}
		}
	}
}

void ObjectManager::List_Check_flag()
{
	list<BaseObject*>::iterator iter = object_list.begin();
	for (; iter != object_list.end(); ) {
		if ((*iter)->flag == false) {
			delete (*iter); (*iter) = nullptr;
			object_list.erase(iter++);
		}
		else {
			iter++;
		}
	}
}

bool ObjectManager::List_Check_have(EObjectType type, int n = 1)
{
	// ����ִ� type�� ���� ��ü�� ã��
	list<BaseObject*>::iterator iter = object_list.begin();
	for (; iter != object_list.end(); iter++) {
		if ((*iter)->Get_object_type() == type && (*iter)->flag == true) {

			n--;
			// �ش� ��ü�� n�� �̻� �����Ѵٸ� return true
			if (n <= 0) {
				return true;
			}
		}
	}

	// �ش� ��ü�� n�� �̸� �����Ѵٸ� return false;
	return false;
}

void ObjectManager::List_Delete() {
	list<BaseObject*>::iterator iter = object_list.begin();
	for (; iter != object_list.end();) {
		delete (*iter);

		object_list.erase(iter++);
	}
}