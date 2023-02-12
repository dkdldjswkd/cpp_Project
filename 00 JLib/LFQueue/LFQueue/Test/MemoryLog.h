#pragma once

struct MemoryLog {
public:
	MemoryLog();
public:
	char* begin;
	char* end;

	DWORD offset = 0;
	DWORD mask;

private:
	char* get_wp();

public:
	char* Log(BYTE thread_id);
	char* Log(BYTE thread_id, void* p_logMsg);
};

extern MemoryLog memLogger;