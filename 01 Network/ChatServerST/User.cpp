#include "User.h"

User::User() {}

User::~User() {}

void User::Set(SessionId session_id){
	this->sessionID = session_id;
}

void User::Reset(){
	isLogin = false;
}