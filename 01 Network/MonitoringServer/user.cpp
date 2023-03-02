#include "User.h"

User::User() {}
User::~User() {}

void User::Set(SESSION_ID session_id){
	this->session_id = session_id;
}

void User::Reset() {
	is_login = false;
}

void User::Login() {
	is_login = true;
}