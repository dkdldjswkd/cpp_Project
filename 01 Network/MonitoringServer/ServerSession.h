#pragma once
#include "User.h"

// MonitoringLanServer ����
class ServerSession : public User {
public:
	ServerSession();
	~ServerSession();

public:
	int serverNo;

public:
	void Login(int serverNo);
};