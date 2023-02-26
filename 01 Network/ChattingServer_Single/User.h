#pragma once
#include "../NetworkLib/NetServer.h"

struct User {
public:
	User();
	virtual ~User();

public:
	// 연결 정보
	SESSION_ID session_id = INVALID_SESSION_ID;
	bool is_login = false;

public:
	virtual void Set(SESSION_ID session_id);
	virtual void Reset();
};