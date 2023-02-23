#include "User.h"

void User::Set(SESSION_ID session_id){
	this->session_id = session_id;
}

void User::Reset(){
	is_login = false;
}

void User::LoginFromServer(int serverNo){
	is_login = true;
	this->serverNo = serverNo;
	isTool = false;
}

void User::LoginFromTool(){
	is_login = true;
	isTool = true;
}