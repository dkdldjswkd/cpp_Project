#pragma once
#include "../NetworkLib/NetServer.h"

struct User {
public:
	User();
	virtual ~User();

public:
	// ���� ����
	SessionId sessionID = INVALID_SESSION_ID;
	bool isLogin = false;

public:
	virtual void Set(SessionId session_id);
	virtual void Reset();
};