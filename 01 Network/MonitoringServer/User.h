#pragma once
#include "../NetworkLib/NetServer.h"

struct User {
public:
	User() {}
	~User() {}

public:
	// ���� ����
	SESSION_ID session_id = INVALID_SESSION_ID;
	bool is_login = false;

public:
	// ������
	int serverNo;
	bool isTool = false;

public:
	void Set(SESSION_ID session_id);
	void Reset();
	void LoginFromServer(int serverNo);
	void LoginFromTool();
};