#pragma once
#pragma once
#include "list"

#define FILE_NAME_LEN 128
#define LOGFILE_NAME_LEN 64

class MemoryHistory {
private:
	class ALLOC_INFO {
	public:
		ALLOC_INFO(void* ptr, int size, const char* log_file_name, int line, bool is_array);
		ALLOC_INFO(const ALLOC_INFO& other);

	public:
		void* p_alloc;
		int		alloc_size;
		char	file_name[FILE_NAME_LEN]; // __FILE__ (해당 코드를 실행한 파일 ex.D:\\directory\\main.cpp)
		int		line;
		bool	is_array;
	};

private:
	std::list<ALLOC_INFO>	allocInfos;
	char					logFile_name[LOGFILE_NAME_LEN] = { 0, };

public:
	MemoryHistory(const char* log_file_name = "AllocInfo");
	~MemoryHistory();

private:
	void NewAlloc(void* pPtr, int iSize, const char* szFile, int iLine, bool bArray);
	bool Delete(void* p_alloc, bool is_array);

public:
	bool Save_LogFile();

public:
	// new 연산자 오버로딩 함수
	friend void* operator new (size_t size, const char* FILE, int Line);
	friend void* operator new[](size_t size, const char* FILE, int Line);
	friend void operator delete (void* p);
	friend void operator delete[](void* p);
};

extern MemoryHistory g_memoryHistory;

void* operator new (size_t size, const char* FILE, int Line);
void* operator new[](size_t size, const char* FILE, int Line);
void operator delete (void* p);
void operator delete[](void* p);

// 짝 맞추기 위함
void operator delete (void* p, char* File, int Line);
void operator delete[](void* p, char* File, int Line);


#define new new(__FILE__, __LINE__)