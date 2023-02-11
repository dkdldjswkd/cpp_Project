#pragma once

struct MemoryLogger {
public:
	MemoryLogger();
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

extern MemoryLogger memLogger;

#define MEMORY_LOG(thread_id)			memLogger.Log(thread_id)
#define MEMORY_LOG_(thread_id, p_logMsg) memLogger.Log(thread_id, p_logMsg)