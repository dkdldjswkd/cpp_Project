#include <Windows.h>
#include "MemoryLogger.h"

#define LOG_SIZE 8
#define MEMORY_SIZE 0x07FFFFFF // 0xFFFFFFFF >> 5; (2^27, 134217727)

//MemoryLogger memLogger;

MemoryLogger::MemoryLogger(){
	mask = MEMORY_SIZE;

	begin = (char*)malloc(MEMORY_SIZE * sizeof(char));
	end = begin + MEMORY_SIZE;

	offset = -LOG_SIZE;
}

char* MemoryLogger::get_wp() {
	return begin + (InterlockedAdd((LONG*)&offset, (LONG64)LOG_SIZE) & mask);
}

// 1byte 기록
char* MemoryLogger::Log(BYTE thread_id){
	char* const wp = get_wp();
	*wp = thread_id;
	ZeroMemory(wp + 1, LOG_SIZE - 1);
	return wp;
}

// 8byte 기록
char* MemoryLogger::Log(BYTE thread_id, void* p_logMsg){
	char* const wp = get_wp();
	*wp = thread_id;
	memmove(wp + 1, p_logMsg, LOG_SIZE - 1);
	return wp;
}