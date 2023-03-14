#include "ServerSession.h"

ServerSession::ServerSession(){
}

ServerSession::~ServerSession(){
}

void ServerSession::Login(int serverNo) {
 	isLogin = true;
	this->serverNo = serverNo;
}