#include "stdafx.h"
#include "Character.h"
#include "Sector.h"

using namespace std;

Character* Find_Character(SESSION_ID session_id){
	return nullptr;
}

Character::Character(){

}

Character::~Character(){
}

void Character::Clear(){
	p_session = nullptr;
	session_id = INVALID_SESSION_ID;
}

void Character::Set(Session* p_session, short x, short y, char hp){
	this->p_session = p_session;
	session_id = p_session->session_id;

	action	= ACTION::STOP;
	dir		= DIR::DIR_RR;

	this->x = x;
	this->y = y;

	cur_sector.x = x / SECTOR_SIZE;
	cur_sector.y = y / SECTOR_SIZE;
	old_sector = INVALID_SECTOR;

	this->hp = hp;
}
