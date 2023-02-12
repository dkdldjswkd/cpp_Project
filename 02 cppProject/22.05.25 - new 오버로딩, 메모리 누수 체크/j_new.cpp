#include "j_new.h"
#undef new

#include <time.h>
#include <string.h>

MemoryHistory g_memoryHistory;

MemoryHistory::ALLOC_INFO::ALLOC_INFO(void* ptr, int size, const char* log_file_name, int line, bool is_array)
	: p_alloc(ptr), alloc_size(size), line(line), is_array(is_array) 
{
	strcpy_s(file_name, FILE_NAME_LEN, log_file_name);
}

MemoryHistory::ALLOC_INFO::ALLOC_INFO(const ALLOC_INFO& other) {
	p_alloc = other.p_alloc;
	alloc_size = other.alloc_size;
	strcpy_s(file_name, FILE_NAME_LEN, other.file_name);
	line = other.line;
	is_array = other.is_array;
}

MemoryHistory::MemoryHistory(const char* log_file_name) {
	time_t timer;
	time(&timer);

	tm time_data;
	localtime_s(&time_data, &timer);

	char time_buf[100] = { 0, };
	sprintf_s(time_buf, 100, "_%04d%02d%02d_%02d%02d_%02d",
		time_data.tm_year + 1900,
		time_data.tm_mon + 1,
		time_data.tm_mday,
		time_data.tm_hour,
		time_data.tm_min,
		time_data.tm_sec
	);

	strcat_s(logFile_name, LOGFILE_NAME_LEN, log_file_name);
	strcat_s(logFile_name, LOGFILE_NAME_LEN, time_buf);
	strcat_s(logFile_name, LOGFILE_NAME_LEN, ".txt");
}

MemoryHistory::~MemoryHistory() {
	Save_LogFile();
}

bool MemoryHistory::Save_LogFile()
{
	FILE* p_logFile;
	auto err = fopen_s(&p_logFile, logFile_name, "a");
	if (err != 0) return false;

	std::list<ALLOC_INFO>::iterator iter = allocInfos.begin();
	for (; iter != allocInfos.end(); iter++) {
		ALLOC_INFO& allocInfo = *iter;
		if (allocInfo.p_alloc != NULL) {
			fprintf(p_logFile, "LEAK    !! ");
			fprintf(p_logFile, "[0x%p] [%7d] %s : %d \n", allocInfo.p_alloc, allocInfo.alloc_size, allocInfo.file_name, allocInfo.line);
		}
	}
	fclose(p_logFile);
	return true;
}

void MemoryHistory::NewAlloc(void* p_alloc, int alloc_size, const char* file_name, int line, bool is_array) {
	allocInfos.push_back(ALLOC_INFO(p_alloc, alloc_size, file_name, line, is_array));
}

bool MemoryHistory::Delete(void* p_alloc, bool is_array) {
	FILE* p_logFile;

	std::list<ALLOC_INFO>::iterator iter = allocInfos.begin();
	for (; iter != allocInfos.end(); iter++) {
		ALLOC_INFO& allocInfo = *iter;

		// 엑세스 위반 예외 throw 왜 ???
		if (allocInfo.p_alloc == p_alloc) {
			// 할당, 해제의 짝이 맞지 않음 (new[] -> delete / new -> delete[])
			if (allocInfo.is_array != is_array) {
				auto err = fopen_s(&p_logFile, logFile_name, "a");
				if (err == 0) {
					fprintf(p_logFile, "ARRAY !! [0x%p] [%7d] \n", allocInfo.p_alloc, allocInfo.alloc_size);
					fclose(p_logFile);
				}
				return false;
			}
			// 정상적 해제
			else {
				allocInfo.p_alloc = nullptr;
				return true;
			}
		}
	}

	// 할당하지 않은 주소를 해제
	auto err = fopen_s(&p_logFile, logFile_name, "a");
	if (err != 0) return false;

	fprintf(p_logFile, "NOALLOC [0x%x] \n", p_alloc);
	fclose(p_logFile);

	return false;
}


////////////////////////////
// new 오버로딩
////////////////////////////

void* operator new (size_t size, const char* FILE, int Line) {
	void* p = malloc(size);
	g_memoryHistory.NewAlloc(p, size, FILE, Line, false);
	return p;
}

// * 소멸자가 있을때 (컴파일러가 소멸자 존재여부 판단해서 유연하게 메모리 할당 및 ret함)
// size == sizeof(object) * [n] + 포인터 크기
// return p; 로 던질때 (p+8) 을 던짐
void* operator new[](size_t size, const char* FILE, int Line) {
	void* p = malloc(size);
	g_memoryHistory.NewAlloc(p, size, FILE, Line, true);
	return p;
}

void operator delete (void* p) {
	if (g_memoryHistory.Delete(p, false)) {
		free(p);
	}
}

// * 소멸자가 있을때 (소멸자 존재여부 판단해서 받은 주소 -8의 위치를 free함)
// p - 8 로 p의 주소를 수정해서 받음
void operator delete[](void* p) {
	if (g_memoryHistory.Delete(p, true)) {
		free(p);
	}
}

// 짝 맞추기 위함
void operator delete (void* p, char* File, int Line) {}
void operator delete[](void* p, char* File, int Line) {}