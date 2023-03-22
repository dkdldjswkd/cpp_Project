#include "User.h"

User::User() {}

User::~User() {}

void User::Set(SESSION_ID session_id){
	this->sessionID = session_id;
}

void User::Reset(){
	isLogin = false;
}