#pragma once
#include "../NetworkLib/NetServer.h"

struct User {
public:
	User();
	virtual ~User();

public:
	// 연결 정보
	SessionId sessionId = INVALID_SESSION_ID;
	bool isLogin = false;

public:
	virtual void Set(SessionId sessionId);
	virtual void Reset();
};