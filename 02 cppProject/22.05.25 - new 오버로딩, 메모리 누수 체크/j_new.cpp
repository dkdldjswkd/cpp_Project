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

		// ������ ���� ���� throw �� ???
		if (allocInfo.p_alloc == p_alloc) {
			// �Ҵ�, ������ ¦�� ���� ���� (new[] -> delete / new -> delete[])
			if (allocInfo.is_array != is_array) {
				auto err = fopen_s(&p_logFile, logFile_name, "a");
				if (err == 0) {
					fprintf(p_logFile, "ARRAY !! [0x%p] [%7d] \n", allocInfo.p_alloc, allocInfo.alloc_size);
					fclose(p_logFile);
				}
				return false;
			}
			// ������ ����
			else {
				allocInfo.p_alloc = nullptr;
				return true;
			}
		}
	}

	// �Ҵ����� ���� �ּҸ� ����
	auto err = fopen_s(&p_logFile, logFile_name, "a");
	if (err != 0) return false;

	fprintf(p_logFile, "NOALLOC [0x%x] \n", p_alloc);
	fclose(p_logFile);

	return false;
}


////////////////////////////
// new �����ε�
////////////////////////////

void* operator new (size_t size, const char* FILE, int Line) {
	void* p = malloc(size);
	g_memoryHistory.NewAlloc(p, size, FILE, Line, false);
	return p;
}

// * �Ҹ��ڰ� ������ (�����Ϸ��� �Ҹ��� ���翩�� �Ǵ��ؼ� �����ϰ� �޸� �Ҵ� �� ret��)
// size == sizeof(object) * [n] + ������ ũ��
// return p; �� ������ (p+8) �� ����
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

// * �Ҹ��ڰ� ������ (�Ҹ��� ���翩�� �Ǵ��ؼ� ���� �ּ� -8�� ��ġ�� free��)
// p - 8 �� p�� �ּҸ� �����ؼ� ����
void operator delete[](void* p) {
	if (g_memoryHistory.Delete(p, true)) {
		free(p);
	}
}

// ¦ ���߱� ����
void operator delete (void* p, char* File, int Line) {}
void operator delete[](void* p, char* File, int Line) {}