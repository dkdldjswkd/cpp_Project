#include <Windows.h>
#include "MemoryLog.h"

#define LOG_SIZE 8
#define MEMORY_SIZE 134217727 // 0xFFFFFFFF >> 5; (2^27)

MemoryLog memLogger;

MemoryLog::MemoryLog(){
	mask = MEMORY_SIZE;

	begin = (char*)malloc(MEMORY_SIZE * sizeof(char));
	end = begin + MEMORY_SIZE;

	offset = -LOG_SIZE;
}

char* MemoryLog::get_wp() {
	return begin + (InterlockedAdd((LONG*)&offset, (LONG64)LOG_SIZE) & mask);
}

char* MemoryLog::Log(BYTE thread_id){
	char* const wp = get_wp();
	*wp = thread_id;
	ZeroMemory(wp + 1, LOG_SIZE - 1);
	return wp;
}

char* MemoryLog::Log(BYTE thread_id, void* p_logMsg){
	char* const wp = get_wp();
	*wp = thread_id;
	memmove(wp + 1, p_logMsg, LOG_SIZE - 1);
	return wp;
}