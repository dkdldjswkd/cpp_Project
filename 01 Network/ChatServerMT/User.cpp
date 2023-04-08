#include "User.h"

User::User() {}

User::~User() {}

void User::Set(SessionId sessionId){
	this->sessionId = sessionId;
}

void User::Reset(){
	isLogin = false;
}