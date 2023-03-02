#include "ServerSession.h"

ServerSession::ServerSession(){
}

ServerSession::~ServerSession(){
}

void ServerSession::Login(int serverNo) {
 	is_login = true;
	this->serverNo = serverNo;
}